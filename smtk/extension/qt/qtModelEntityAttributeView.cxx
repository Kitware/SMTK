//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtModelEntityAttributeView.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtSelectionManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"

#include "smtk/view/View.h"

#include <QBrush>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPointer>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVariant>

#include <cassert>
#include <iostream>
#include <set>

using namespace smtk::attribute;
using namespace smtk::extension;

qModelEntityAttributeViewComboBoxItemDelegate::qModelEntityAttributeViewComboBoxItemDelegate(
  const QStringList& vals, QObject* parent)
  : QStyledItemDelegate(parent)
  , m_values(vals)
{
}
qModelEntityAttributeViewComboBoxItemDelegate::~qModelEntityAttributeViewComboBoxItemDelegate()
{
}

QWidget* qModelEntityAttributeViewComboBoxItemDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
  auto cbox = new QComboBox(parent);
  cbox->addItems(m_values);
  return cbox;
}

void qModelEntityAttributeViewComboBoxItemDelegate::setEditorData(
  QWidget* editor, const QModelIndex& index) const
{
  auto cb = qobject_cast<QComboBox*>(editor);
  if (cb != nullptr)
  {
    // Lets find the proper index of the current value w/r the combobox
    auto currentText = index.data(Qt::EditRole).toString();
    int pos = cb->findText(currentText);
    if (pos >= 0)
    {
      cb->setCurrentIndex(pos);
    }
  }
  else
  {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

void qModelEntityAttributeViewComboBoxItemDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  auto cb = qobject_cast<QComboBox*>(editor);
  if (cb != nullptr)
  {
    if (cb->currentIndex() > -1)
    {
      // Block signals on first setData, so that only 1 cellChanged signal is emitted
      model->blockSignals(true);
      model->setData(index, cb->currentText(), Qt::EditRole);
      model->blockSignals(false);
      model->setData(index, QColor(Qt::white), Qt::BackgroundRole);
    }
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

class qtModelEntityAttributeViewInternals
{
public:
  const QList<smtk::attribute::DefinitionPtr> getCurrentDefs(const QString strCategory) const
  {

    if (this->AttDefMap.keys().contains(strCategory))
    {
      return this->AttDefMap[strCategory];
    }
    return this->AllDefs;
  }

  smtk::attribute::AttributePtr getAttribute(const smtk::model::EntityRef& modelEnt) const
  {
    // Check against all our definitions; should only find 1 attribute at the most
    auto iter = this->m_attDefinitions.cbegin();
    for (; iter != this->m_attDefinitions.cend(); ++iter)
    {
      auto atts = modelEnt.attributes(*iter);
      assert(atts.size() <= 1); // debug
      if (atts.size())
      {
        return atts.at(0);
      }
    } // for
    return nullptr;
  }

  qtTableWidget* ListTable;

  QFrame* ButtonsFrame;
  QFrame* TopFrame;    // top
  QFrame* BottomFrame; // bottom

  // <category, AttDefinitions>
  QMap<QString, QList<smtk::attribute::DefinitionPtr> > AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllDefs;

  // Attribute widget
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QFrame> AttFrame;

  // Model for filtering the attribute properties by combobox.
  QPointer<QStandardItemModel> checkablePropComboModel;
  QMap<std::string, Qt::CheckState> AttProperties;
  std::vector<smtk::attribute::DefinitionPtr> m_attDefinitions;
  smtk::model::BitFlags m_modelEntityMask;
  std::string m_selectionSourceName;
  std::string m_unSetVal;
};

qtBaseView* qtModelEntityAttributeView::createViewWidget(const ViewInfo& info)
{
  // TO DO Need to deal with Selections
  qtModelEntityAttributeView* view = new qtModelEntityAttributeView(info);

  // connect with selection manager
  if (auto selMgr = qtActiveObjects::instance().smtkSelectionManager())
  {
    // need a relay since association widget might be empty at this time
    QObject::connect(selMgr.get(),
      SIGNAL(broadcastToReceivers(const smtk::model::EntityRefs&, const smtk::mesh::MeshSets&,
        const smtk::model::DescriptivePhrases&, const std::string&)),
      view, SLOT(updateSelectedModelEntity(const smtk::model::EntityRefs&)));
    view->buildUI();
    smtk::model::EntityRefs selEntities;
    selMgr->getSelectedEntitiesAsEntityRefs(selEntities);
    view->updateSelectedModelEntity(selEntities);
  }
  else
  {
    view->buildUI();
  }
  return view;
}

qtModelEntityAttributeView::qtModelEntityAttributeView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new qtModelEntityAttributeViewInternals;
  std::ostringstream receiverSource;
  receiverSource << "qtModelEntityAttributeView" << this;
  this->Internals->m_selectionSourceName = receiverSource.str();
  if (qtActiveObjects::instance().smtkSelectionManager() &&
    !qtActiveObjects::instance().smtkSelectionManager()->registerSelectionSource(
      this->Internals->m_selectionSourceName))
  {
    std::cerr << "register selection source " << this->Internals->m_selectionSourceName
              << "failed. Already existed!" << std::endl;
  }
}

qtModelEntityAttributeView::~qtModelEntityAttributeView()
{
  if (qtActiveObjects::instance().smtkSelectionManager())
  {
    qtActiveObjects::instance().smtkSelectionManager()->unregisterSelectionSource(
      this->Internals->m_selectionSourceName);
  }
  delete this->Internals;
}

const QMap<QString, QList<smtk::attribute::DefinitionPtr> >&
qtModelEntityAttributeView::attDefinitionMap() const
{
  return this->Internals->AttDefMap;
}

void qtModelEntityAttributeView::createWidget()
{
  auto view = this->getObject();
  if (view == nullptr)
  {
    return;
  }

  this->Internals->AttDefMap.clear();
  const CollectionPtr attSys = this->uiManager()->attCollection();
  std::set<std::string>::const_iterator it;
  const std::set<std::string>& cats = attSys->categories();

  for (it = cats.begin(); it != cats.end(); it++)
  {
    QList<smtk::attribute::DefinitionPtr> attdeflist;
    this->Internals->AttDefMap[it->c_str()] = attdeflist;
  }

  // Initialize definition info
  this->getAllDefinitions();

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation(Qt::Vertical);

  QFrame* TopFrame = new QFrame(frame);
  QFrame* BottomFrame = new QFrame(frame);

  this->Internals->TopFrame = TopFrame;
  this->Internals->BottomFrame = BottomFrame;
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QVBoxLayout* TopLayout = new QVBoxLayout(TopFrame);
  TopLayout->setMargin(0);
  TopFrame->setSizePolicy(sizeFixedPolicy);
  QVBoxLayout* BottomLayout = new QVBoxLayout(BottomFrame);
  BottomLayout->setMargin(0);
  BottomFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  QSizePolicy tableSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // create a list box for all the entries
  this->Internals->ListTable = new qtTableWidget(frame);
  this->Internals->ListTable->setColumnCount(2);
  QStringList headers;
  // Set Headers
  std::string s;
  if (view->details().attribute("ColHeader1", s))
  {
    headers << s.c_str();
  }
  else
  {
    headers << "Model Entity";
  }
  if (view->details().attribute("ColHeader2", s))
  {
    headers << s.c_str();
  }
  else
  {
    headers << "Type";
  }
  this->Internals->ListTable->setHorizontalHeaderLabels(headers);
  this->Internals->ListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  this->Internals->ListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->ListTable->setSizePolicy(tableSizePolicy);
  this->Internals->ListTable->setEditTriggers(
    QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);

  // Lets see if a unset name has been set (which is used if the model entity does not currently have an attribute
  // assigned to it)
  if (view->details().attribute("NoValueLabel", s))
  {
    this->Internals->m_unSetVal = s;
  }
  else
  {
    this->Internals->m_unSetVal = "Please Select";
  }
  this->updateModelEntities();
  TopLayout->addWidget(this->Internals->ListTable);

  // Attribte frame
  this->Internals->AttFrame = new QFrame(frame);
  new QVBoxLayout(this->Internals->AttFrame);
  BottomLayout->addWidget(this->Internals->AttFrame);

  frame->addWidget(TopFrame);
  frame->addWidget(BottomFrame);

  this->Internals->ListTable->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  this->Internals->ListTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  connect(
    this->Internals->ListTable, SIGNAL(itemSelectionChanged()), this, SLOT(selectedRowChanged()));
  connect(
    this->Internals->ListTable, SIGNAL(cellChanged(int, int)), this, SLOT(cellChanged(int, int)));
  this->Widget = frame;
}

void qtModelEntityAttributeView::updateModelEntities()
{
  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->uiManager()->currentCategory().c_str());

  // Create an initial string list for the combo boxes
  QStringList slist;
  for (int i = 0; i < currentDefs.size(); ++i)
  {
    slist.append(currentDefs.at(i)->displayedTypeName().c_str());
  }

  // Add Special entry for the case of no attribite assigned
  slist.append(this->Internals->m_unSetVal.c_str());

  auto col2Delegate =
    new qModelEntityAttributeViewComboBoxItemDelegate(slist, this->Internals->ListTable);
  this->Internals->ListTable->blockSignals(true);
  this->Internals->ListTable->setRowCount(0);
  this->Internals->ListTable->setItemDelegateForColumn(1, col2Delegate);
  auto attCol = this->uiManager()->attCollection();
  auto modelCol = attCol->refModelManager();
  if (!modelCol)
  {
    this->Internals->ListTable->blockSignals(false);
    return;
  }

  // Disable sorting when modifying ListTable
  this->Internals->ListTable->setSortingEnabled(false);

  auto entityRefs = modelCol->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(
    this->Internals->m_modelEntityMask, false);
  smtk::model::Model activeModel = qtActiveObjects::instance().activeModel();
  smtk::model::Manager::Ptr tmpMgr = smtk::model::Manager::create();
  smtk::model::Group tmpGrp = tmpMgr->addGroup();
  tmpGrp.setMembershipMask(this->Internals->m_modelEntityMask);

  int rcount = 0;
  for (auto iter = entityRefs.begin(); iter != entityRefs.end(); ++iter)
  {
    if (tmpGrp.meetsMembershipConstraints(*iter) &&
      iter->owningModel().entity() == activeModel.entity())
    {
      std::string name = iter->name();
      auto item = new QTableWidgetItem(QString::fromStdString(name));
      //save the entity as a uuid string
      QVariant vdata(QString::fromStdString(iter->entity().toString()));
      item->setData(Qt::UserRole, vdata);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      this->Internals->ListTable->insertRow(rcount);
      this->Internals->ListTable->setItem(rcount, 0, item);

      auto att = this->Internals->getAttribute(*iter);
      if (!att)
      {
        item = new QTableWidgetItem(slist.at(slist.size() - 1));
      }
      else
      {
        std::string typeName = att->definition()->displayedTypeName();
        QColor icolor = Qt::white;
        if (!slist.contains(typeName.c_str()))
        {
          typeName += "-Invalid";
          icolor = Qt::red;
        }
        item = new QTableWidgetItem(typeName.c_str());
        item->setBackground(icolor);
      }
      this->Internals->ListTable->setItem(rcount, 1, item);
      ++rcount;
    }
  }
  this->Internals->ListTable->blockSignals(false);
  this->Internals->ListTable->setSortingEnabled(true);
  this->Internals->ListTable->sortByColumn(Qt::AscendingOrder);
  this->Internals->ListTable->sortItems(0);
  if (this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
  {
    delete this->Internals->CurrentAtt;
  }
}

QTableWidgetItem* qtModelEntityAttributeView::getSelectedItem()
{
  return this->Internals->ListTable->selectedItems().count() > 0
    ? this->Internals->ListTable->selectedItems().value(0)
    : nullptr;
}

void qtModelEntityAttributeView::updateModelAssociation()
{
  this->updateModelEntities();
}

void qtModelEntityAttributeView::cellChanged(int row, int column)
{
  if (column != 1)
  {
    std::cerr << "ERROR: cell changed at (" << row << "," << column << ")\n";
  }

  // Get selected type
  std::string tname = this->Internals->ListTable->item(row, 1)->text().toStdString();

  auto attCol = this->uiManager()->attCollection();
  auto modelCol = attCol->refModelManager();
  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->uiManager()->currentCategory().c_str());
  // Get the model entity UUID
  auto data = this->Internals->ListTable->item(row, 0)->data(Qt::UserRole);
  std::string val = data.toString().toStdString();
  smtk::common::UUID mid(val);
  smtk::model::EntityRef eref(modelCol, mid);
  // Get the current attribute associated with the model entity (if any)
  auto att = this->Internals->getAttribute(eref);
  if (att && att->definition()->displayedTypeName() == tname)
  {
    // The attribute itself didn't change, so we can stop here
    return;
  }
  else if (att)
  {
    attCol->removeAttribute(att);
  }

  // Now create a new attribute for the model entity of the correct type
  // Find the def we need to use
  for (int j = 0; j < currentDefs.size(); ++j)
  {
    if (currentDefs.at(j)->displayedTypeName() == tname)
    {
      auto att = attCol->createAttribute(currentDefs.at(j));
      att->associateEntity(eref);
      break;
    }
  }
  this->Internals->ListTable->selectRow(row);
  this->selectedRowChanged();
}

void qtModelEntityAttributeView::selectedRowChanged()
{
  this->showCurrentRow(true);
}

void qtModelEntityAttributeView::showCurrentRow(bool broadcastSelected)
{
  int index = this->Internals->ListTable->currentRow();
  auto attCol = this->uiManager()->attCollection();
  auto modelCol = attCol->refModelManager();
  auto data = this->Internals->ListTable->item(index, 0)->data(Qt::UserRole);
  std::string val = data.toString().toStdString();
  smtk::common::UUID mid(val);
  smtk::model::EntityRef eref(modelCol, mid);
  // Get the current attribute associated with the model entity (if any)
  auto att = this->Internals->getAttribute(eref);
  this->displayAttribute(att);

  if (broadcastSelected)
  {
    smtk::model::EntityRefs selents;
    selents.insert(eref);
    this->invokeEntitiesSelected(selents, this->Internals->m_selectionSourceName);
  }
}

void qtModelEntityAttributeView::updateSelectedModelEntity(const smtk::model::EntityRefs& ents)
{
  this->Internals->ListTable->blockSignals(true);
  if (ents.size() != 1)
  {
    this->Internals->ListTable->clearSelection();
    this->displayAttribute(nullptr);
  }
  else
  {
    for (int i = 0; i < this->Internals->ListTable->rowCount(); ++i)
    {
      auto data = this->Internals->ListTable->item(i, 0)->data(Qt::UserRole);
      std::string val = data.toString().toStdString();
      smtk::common::UUID mid(val);
      if (ents.begin()->entity() == mid)
      {
        this->Internals->ListTable->selectRow(i);
        this->showCurrentRow(false);
        break;
      }
    }
  }
  this->Internals->ListTable->blockSignals(false);
}

void qtModelEntityAttributeView::onShowCategory()
{
  this->updateModelEntities();
}

void qtModelEntityAttributeView::displayAttribute(smtk::attribute::AttributePtr att)
{
  if (this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
  {
    delete this->Internals->CurrentAtt;
  }

  if (att == nullptr)
  {
    this->Internals->AttFrame->setVisible(0);
    return;
  }

  this->Internals->AttFrame->setVisible(1);

  int currentLen = this->fixedLabelWidth();
  int tmpLen = this->uiManager()->getWidthOfAttributeMaxLabel(
    att->definition(), this->uiManager()->advancedFont());
  this->setFixedLabelWidth(tmpLen);

  this->Internals->CurrentAtt = new qtAttribute(att, this->Internals->AttFrame, this);
  // By default use the basic layout with no model associations since this class
  // takes care of it
  this->Internals->CurrentAtt->createBasicLayout(false);
  this->setFixedLabelWidth(currentLen);
  if (this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
  {
    this->Internals->AttFrame->layout()->addWidget(this->Internals->CurrentAtt->widget());
    if (this->advanceLevelVisible())
    {
      this->Internals->CurrentAtt->showAdvanceLevelOverlay(true);
    }
  }
}

void qtModelEntityAttributeView::getAllDefinitions()
{
  smtk::view::ViewPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  smtk::attribute::CollectionPtr sys = this->uiManager()->attCollection();

  std::string attName, defName, val;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;

  // The view should have a single internal component called InstancedAttributes
  if ((view->details().numberOfChildren() != 1) ||
    (view->details().child(0).name() != "AttributeTypes"))
  {
    // Should present error message
    return;
  }

  if (view->details().attribute("ModelEntityFilter", val))
  {
    smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(val);
    this->Internals->m_modelEntityMask = flags;
  }
  else
  {
    this->Internals->m_modelEntityMask = 0;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  smtk::view::View::Component& attsComp = view->details().child(0);
  std::size_t i, n = attsComp.numberOfChildren();
  for (i = 0; i < n; i++)
  {
    if (attsComp.child(i).name() != "Att")
    {
      continue;
    }
    if (!attsComp.child(i).attribute("Type", defName))
    {
      continue;
    }

    attDef = sys->findDefinition(defName);
    this->qtBaseView::getDefinitions(attDef, this->Internals->AllDefs);
    this->Internals->m_attDefinitions.push_back(attDef);
  }

  // sort the list
  std::sort(std::begin(this->Internals->AllDefs), std::end(this->Internals->AllDefs),
    [](smtk::attribute::DefinitionPtr a, smtk::attribute::DefinitionPtr b) {
      return a->displayedTypeName() < b->displayedTypeName();
    });
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
  foreach (smtk::attribute::DefinitionPtr adef, this->Internals->AllDefs)
  {
    foreach (QString category, this->Internals->AttDefMap.keys())
    {
      if (adef->isMemberOf(category.toStdString()) &&
        !this->Internals->AttDefMap[category].contains(adef))
      {
        this->Internals->AttDefMap[category].push_back(adef);
      }
    }
  }
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
}

void qtModelEntityAttributeView::showAdvanceLevelOverlay(bool show)
{
  if (this->Internals->CurrentAtt)
  {
    this->Internals->CurrentAtt->showAdvanceLevelOverlay(show);
  }
}
