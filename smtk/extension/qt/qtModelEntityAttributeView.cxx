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
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Resource.h"
#include "smtk/resource/Manager.h"

#include "smtk/view/Selection.h"
#include "smtk/view/View.h"

#include <QApplication>
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
  connect(cbox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(choiceMade()));
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
      model->setData(index, cb->currentText(), Qt::EditRole);
      model->setData(index, QColor(Qt::white), Qt::BackgroundRole);
    }
    //QApplication::postEvent(model, new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier));
    emit const_cast<qModelEntityAttributeViewComboBoxItemDelegate*>(this)->choiceMade();
    emit const_cast<qModelEntityAttributeViewComboBoxItemDelegate*>(this)->destroyEditor(
      editor, index);
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

  smtk::attribute::AttributePtr getAttribute(const smtk::resource::PersistentObjectPtr obj) const
  {
    // Check against all our definitions; should only find 1 attribute at the most
    auto iter = this->m_attDefinitions.cbegin();
    for (; iter != this->m_attDefinitions.cend(); ++iter)
    {
      auto atts = (*iter)->attributes(obj);
      assert(atts.size() <= 1); // debug
      if (atts.size())
      {
        return *(atts.begin());
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
  std::string m_modelEntityMask;
  std::string m_selectionSourceName;
  std::string m_unSetVal;
  int m_selectionObserverId;
  std::map<std::string, smtk::view::View::Component> m_attCompMap;
};

qtBaseView* qtModelEntityAttributeView::createViewWidget(const ViewInfo& info)
{
  // TO DO Need to deal with Selections
  qtModelEntityAttributeView* view = new qtModelEntityAttributeView(info);
  view->buildUI();
  return view;
}

qtModelEntityAttributeView::qtModelEntityAttributeView(const ViewInfo& info)
  : qtBaseView(info)
{
  this->Internals = new qtModelEntityAttributeViewInternals;
}

qtModelEntityAttributeView::~qtModelEntityAttributeView()
{
  auto sel = this->uiManager()->selection();
  if (sel)
  {
    sel->unregisterSelectionSource(this->Internals->m_selectionSourceName);
    sel->unobserve(this->Internals->m_selectionObserverId);
  }
  delete this->Internals;
}

void qtModelEntityAttributeView::buildUI()
{
  this->qtBaseView::buildUI();
  std::ostringstream receiverSource;
  receiverSource << "qtModelEntityAttributeView" << this;
  this->Internals->m_selectionSourceName = receiverSource.str();

  auto sel = this->uiManager()->selection();
  if (sel)
  {
    if (!sel->registerSelectionSource(this->Internals->m_selectionSourceName))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "register selection source "
          << this->Internals->m_selectionSourceName << "failed. Already existed!");
    }
    this->Internals->m_selectionObserverId = sel->observe(
      [this](const std::string& selectionSource, smtk::view::SelectionPtr sp) {
        this->updateSelectedModelEntity(selectionSource, sp);
      },
      true);
  }
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
  const ResourcePtr attResource = this->uiManager()->attResource();
  std::set<std::string>::const_iterator it;
  const std::set<std::string>& cats = attResource->categories();

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
    headers << "Entity";
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

std::set<smtk::resource::PersistentObjectPtr> qtModelEntityAttributeView::associatableObjects()
  const
{
  std::set<smtk::resource::PersistentObjectPtr> result;
  // First we need to determine if the attribute resource has resources associated with it
  // if not we need to go to resource manager to get the information
  auto attResource = this->uiManager()->attResource();
  auto resources = attResource->associations();
  if (!resources.empty())
  {
    // Lets see if any of the resources are model resources
    for (auto resource : resources)
    {
      if (resource->isOfType(smtk::model::Resource::type_name))
      {
        // Find all components of the proper type
        auto comps = resource->find(this->Internals->m_modelEntityMask);
        result.insert(comps.begin(), comps.end());
      }
    }
  }
  else // we need to use the resource manager
  {
    // Iterate over the acceptable entries
    auto resManager = this->uiManager()->resourceManager();
    // Ask the resource manager to get all appropriate resources
    resources = resManager->find(smtk::model::Resource::type_name);
    // Need to process all of these resources
    for (auto resource : resources)
    {
      // Find all components of the proper type
      auto comps = resource->find(this->Internals->m_modelEntityMask);
      result.insert(comps.begin(), comps.end());
    }
  }
  return result;
}

smtk::resource::PersistentObjectPtr qtModelEntityAttributeView::object(QTableWidgetItem* item)
{
  auto resManager = this->uiManager()->resourceManager();
  smtk::resource::PersistentObjectPtr object;
  if (item == nullptr)
  {
    smtk::resource::PersistentObjectPtr obj;
    return obj;
  }

  QVariant var = item->data(Qt::UserRole);
  smtk::common::UUID uid = qtSMTKUtilities::QVariantToUUID(var);
  // Get the resource
  smtk::resource::ResourcePtr res = resManager->get(uid);
  if (res == nullptr)
  {
    std::cerr << "Could not find Item's Resource!\n";
    return res;
  }
  // Now get the uuid of the component
  var = item->data(Qt::UserRole + 1);
  uid = qtSMTKUtilities::QVariantToUUID(var);
  auto comp = res->find(uid);
  if (comp == nullptr)
  {
    std::cerr << "Could not find Item's Resource Component!\n";
  }
  return comp;
}

void qtModelEntityAttributeView::updateModelEntities()
{
  // First lets clear out the attribute editor
  if (this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
  {
    delete this->Internals->CurrentAtt;
  }

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->uiManager()->currentCategory().c_str());

  // Create an initial string list for the combo boxes
  QStringList slist;
  for (int i = 0; i < currentDefs.size(); ++i)
  {
    slist.append(currentDefs.at(i)->displayedTypeName().c_str());
  }

  // Turn off sorting:
  this->Internals->ListTable->setSortingEnabled(false);

  // Add Special entry for the case of no attribite assigned
  slist.append(this->Internals->m_unSetVal.c_str());

  auto col2Delegate =
    new qModelEntityAttributeViewComboBoxItemDelegate(slist, this->Internals->ListTable);
  connect(col2Delegate, SIGNAL(choiceMade()), this, SLOT(selectionMade()));
  this->Internals->ListTable->blockSignals(true);
  this->Internals->ListTable->setRowCount(0);
  this->Internals->ListTable->setItemDelegateForColumn(1, col2Delegate);

  auto entities = this->associatableObjects();

  int rcount = 0;
  for (auto entity : entities)
  {
    std::string name = entity->name();
    auto item = new QTableWidgetItem(QString::fromStdString(name));
    //save the resource/entity as a uuid strings
    auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(entity);
    QVariant vdata = qtSMTKUtilities::UUIDToQVariant(comp->resource()->id());
    item->setData(Qt::UserRole, vdata);
    vdata = qtSMTKUtilities::UUIDToQVariant(entity->id());
    item->setData(Qt::UserRole + 1, vdata);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    this->Internals->ListTable->insertRow(rcount);
    this->Internals->ListTable->setItem(rcount, 0, item);
    auto att = this->Internals->getAttribute(entity);
    if (att == nullptr)
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
  this->Internals->ListTable->setSortingEnabled(true);
  this->Internals->ListTable->sortByColumn(Qt::AscendingOrder);
  this->Internals->ListTable->blockSignals(false);
  this->Internals->ListTable->sortItems(0);
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

  auto attRes = this->uiManager()->attResource();
  auto resManager = this->uiManager()->resourceManager();
  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->uiManager()->currentCategory().c_str());
  // Get the component of the item
  auto entity = this->object(this->Internals->ListTable->item(row, 0));
  if (entity == nullptr)
  {
    std::cerr << "Could not find selected Item!\n";
    return;
  }

  // Get the current attribute associated with the model entity (if any)
  auto att = this->Internals->getAttribute(entity);
  if (att && att->definition()->displayedTypeName() == tname)
  {
    // The attribute itself didn't change, so we can stop here
    return;
  }
  else if (att)
  {
    attRes->removeAttribute(att);
    this->attributeRemoved(att);
  }

  // Now create a new attribute for the model entity of the correct type
  // Find the def we need to use
  for (int j = 0; j < currentDefs.size(); ++j)
  {
    if (currentDefs.at(j)->displayedTypeName() == tname)
    {
      att = attRes->createAttribute(currentDefs.at(j));
      att->associate(entity);
      // Notify the application of the new attribute via an "operation"
      this->attributeCreated(att);
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
  // Lets get the model entity that is selected in the table
  int index = this->Internals->ListTable->currentRow();
  auto entity = this->object(this->Internals->ListTable->item(index, 0));
  // Get the current attribute associated with the model entity (if any)
  auto att = this->Internals->getAttribute(entity);
  this->displayAttribute(att);

  if (broadcastSelected)
  {
    auto sel = this->uiManager()->selection();
    if (sel)
    {
      smtk::model::EntityArray selents;
      auto modelEnt = std::dynamic_pointer_cast<smtk::model::Entity>(entity);
      selents.push_back(modelEnt);
      auto selBit = this->uiManager()->selectionBit();

      sel->modifySelection(selents, this->Internals->m_selectionSourceName, selBit,
        smtk::view::SelectionAction::UNFILTERED_REPLACE);
    }
  }
}

void qtModelEntityAttributeView::updateSelectedModelEntity(
  const std::string&, smtk::view::SelectionPtr p)
{
  this->Internals->ListTable->blockSignals(true);
  auto selBit = this->uiManager()->selectionBit();
  const auto& selEnts = p->currentSelectionByValueAs<smtk::model::EntityArray>(selBit, false);
  if (selEnts.size() != 1)
  {
    this->Internals->ListTable->clearSelection();
    this->displayAttribute(nullptr);
  }
  else
  {
    for (int i = 0; i < this->Internals->ListTable->rowCount(); ++i)
    {
      auto data = this->Internals->ListTable->item(i, 0)->data(Qt::UserRole + 1);
      smtk::common::UUID mid = qtSMTKUtilities::QVariantToUUID(data);
      if (selEnts.at(0)->id() == mid)
      {
        int alreadyShowing = this->Internals->ListTable->currentRow();
        if (i != alreadyShowing)
        {
          this->Internals->ListTable->selectRow(i);
          this->showCurrentRow(false);
        }
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
  auto it = this->Internals->m_attCompMap.find(att->definition()->type());
  if (it != this->Internals->m_attCompMap.end())
  {
    this->Internals->CurrentAtt = new qtAttribute(att, it->second, this->Internals->AttFrame, this);
  }
  else
  {
    smtk::view::View::Component comp;
    this->Internals->CurrentAtt = new qtAttribute(att, comp, this->Internals->AttFrame, this);
  }
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

  smtk::attribute::ResourcePtr resource = this->uiManager()->attResource();

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
    this->Internals->m_modelEntityMask = val;
  }
  else
  {
    this->Internals->m_modelEntityMask = "*";
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

    attDef = resource->findDefinition(defName);
    if (attDef == nullptr)
    {
      continue;
    }
    this->Internals->m_attCompMap[defName] = attsComp.child(i);
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

void qtModelEntityAttributeView::selectionMade()
{
  if (this->Internals->ListTable)
  {
    QApplication::postEvent(this->Internals->ListTable,
      new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier));
  }
}

bool qtModelEntityAttributeView::isEmpty() const
{
  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->uiManager()->currentCategory().c_str());
  return currentDefs.isEmpty();
}
