//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtComponentAttributeView.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/utility/Queries.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Resource.h"
#include "smtk/resource/Manager.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Selection.h"

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

qComponentAttributeViewComboBoxItemDelegate::qComponentAttributeViewComboBoxItemDelegate(
  const QStringList& vals,
  QObject* parent)
  : QStyledItemDelegate(parent)
  , m_values(vals)
{
}
qComponentAttributeViewComboBoxItemDelegate::~qComponentAttributeViewComboBoxItemDelegate() =
  default;

QWidget* qComponentAttributeViewComboBoxItemDelegate::createEditor(
  QWidget* parent,
  const QStyleOptionViewItem& /*option*/,
  const QModelIndex& /*index*/) const
{
  auto* cbox = new QComboBox(parent);
  cbox->addItems(m_values);
  // We want the combo box to immediately display when chosen and
  // to immediately closed when a value has been selected
  connect(cbox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [=]() {
    // This event will be captured by the delegate eventFilter()
    QApplication::sendEvent(cbox, new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier));
  });
  return cbox;
}

void qComponentAttributeViewComboBoxItemDelegate::setEditorData(
  QWidget* editor,
  const QModelIndex& index) const
{
  auto* cb = qobject_cast<QComboBox*>(editor);
  if (cb != nullptr)
  {
    // Lets find the proper index of the current value w/r the combobox
    auto currentText = index.data(Qt::DisplayRole).toString();
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

void qComponentAttributeViewComboBoxItemDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex& index) const
{
  auto* cb = qobject_cast<QComboBox*>(editor);
  if (cb != nullptr)
  {
    if (cb->currentIndex() > -1)
    {
      model->setData(index, cb->currentText(), Qt::DisplayRole);
      model->setData(index, qtUIManager::contrastWithText(Qt::white), Qt::BackgroundRole);
    }
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

bool qComponentAttributeViewComboBoxItemDelegate::eventFilter(QObject* object, QEvent* event)
{
  // Show combo box popup when the box gains focus
  if (event->type() == QEvent::FocusIn)
  {
    auto* combo_box = qobject_cast<QComboBox*>(object);
    auto* focus_event = dynamic_cast<QFocusEvent*>(event);
    if (
      combo_box && focus_event &&
      // Do not consider focus gained when the popup closes
      focus_event->reason() != Qt::PopupFocusReason)
    {
      combo_box->showPopup();
    }
  }

  return QStyledItemDelegate::eventFilter(object, event);
}

class qtComponentAttributeViewInternals
{
public:
  ~qtComponentAttributeViewInternals() { delete this->CurrentAtt; }

  const QList<smtk::attribute::DefinitionPtr> getCurrentDefs(const ResourcePtr& attResource) const
  {
    if (!(attResource && attResource->activeCategoriesEnabled()))
    {
      // There are no active categories - return everything
      return this->AllDefs;
    }

    QList<smtk::attribute::DefinitionPtr> defs;

    if (attResource->activeCategories().size() == 1)
    {
      std::string theCategory = *(attResource->activeCategories().begin());
      if (this->AttDefMap.keys().contains(theCategory.c_str()))
      {
        return this->AttDefMap[theCategory.c_str()];
      }
      return defs; // return an empty list
    }

    foreach (DefinitionPtr attDef, this->AllDefs)
    {
      if (attResource->passActiveCategoryCheck(attDef->categories()))
      {
        defs.push_back(attDef);
      }
    }
    return defs;
  }

  smtk::attribute::AttributePtr getAttribute(const smtk::resource::PersistentObjectPtr obj) const
  {
    // Check against all our definitions; should only find 1 attribute at the most
    auto iter = this->m_attDefinitions.cbegin();
    for (; iter != this->m_attDefinitions.cend(); ++iter)
    {
      auto atts = (*iter)->attributes(obj);
      assert(atts.size() <= 1); // debug
      if (!atts.empty())
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
  QMap<QString, QList<smtk::attribute::DefinitionPtr>> AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllDefs;

  // Attribute widget
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QFrame> AttFrame;

  // Model for filtering the attribute properties by combobox.
  QPointer<QStandardItemModel> checkablePropComboModel;
  QMap<std::string, Qt::CheckState> AttProperties;
  std::vector<smtk::attribute::DefinitionPtr> m_attDefinitions;
  std::string m_selectionSourceName;
  std::string m_unSetVal;
  smtk::view::SelectionObservers::Key m_selectionObserverId;
  std::map<std::string, smtk::view::Configuration::Component> m_attCompMap;
};

qtBaseView* qtComponentAttributeView::createViewWidget(const smtk::view::Information& info)
{
  // TO DO Need to deal with Selections
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtComponentAttributeView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtComponentAttributeView::qtComponentAttributeView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtComponentAttributeViewInternals;
}

qtComponentAttributeView::~qtComponentAttributeView()
{
  auto sel = this->uiManager()->selection();
  if (sel)
  {
    sel->unregisterSelectionSource(this->Internals->m_selectionSourceName);
    sel->observers().erase(this->Internals->m_selectionObserverId);
  }
  delete this->Internals;
}

void qtComponentAttributeView::buildUI()
{
  this->qtBaseAttributeView::buildUI();
  std::ostringstream receiverSource;
  receiverSource << "qtComponentAttributeView" << this;
  this->Internals->m_selectionSourceName = receiverSource.str();

  auto sel = this->uiManager()->selection();
  if (sel)
  {
    if (!sel->registerSelectionSource(this->Internals->m_selectionSourceName))
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "register selection source " << this->Internals->m_selectionSourceName
                                     << "failed. Already existed!");
    }
    QPointer<qtComponentAttributeView> guardedObject(this);
    this->Internals->m_selectionObserverId = sel->observers().insert(
      [guardedObject](const std::string& selectionSource, smtk::view::SelectionPtr sp) {
        if (guardedObject != nullptr)
        {
          guardedObject->updateSelectedComponent(selectionSource, sp);
        }
      },
      0,
      true,
      "qtComponentAttributeView: Change focus on selection.");
  }
}

const QMap<QString, QList<smtk::attribute::DefinitionPtr>>&
qtComponentAttributeView::attDefinitionMap() const
{
  return this->Internals->AttDefMap;
}

void qtComponentAttributeView::createWidget()
{
  auto view = this->configuration();
  if (view == nullptr)
  {
    return;
  }

  this->Internals->AttDefMap.clear();
  const ResourcePtr attResource = this->attributeResource();
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
  frame->setObjectName(view->name().c_str());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation(Qt::Vertical);

  QFrame* TopFrame = new QFrame(frame);
  TopFrame->setObjectName("top");
  QFrame* BottomFrame = new QFrame(frame);
  BottomFrame->setObjectName("bottom");

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
    headers << "Component";
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

  // Lets see if a unset name has been set (which is used if the resource component does not currently have an attribute
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
  this->Internals->AttFrame->setObjectName("attribute");
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

smtk::resource::PersistentObjectPtr qtComponentAttributeView::object(QTableWidgetItem* item)
{
  auto resManager = this->uiManager()->resourceManager();
  smtk::resource::PersistentObjectPtr object;
  if ((resManager == nullptr) || (item == nullptr))
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

void qtComponentAttributeView::updateModelEntities()
{
  // First lets clear out the attribute editor
  if (this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
  {
    delete this->Internals->CurrentAtt;
    this->Internals->CurrentAtt = nullptr;
  }

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->attributeResource());

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

  auto* col2Delegate =
    new qComponentAttributeViewComboBoxItemDelegate(slist, this->Internals->ListTable);
  this->Internals->ListTable->blockSignals(true);
  this->Internals->ListTable->setRowCount(0);
  this->Internals->ListTable->setItemDelegateForColumn(1, col2Delegate);

  std::set<smtk::resource::PersistentObjectPtr> entities;
  if (!this->Internals->m_attDefinitions.empty())
  {
    ResourcePtr attResource = this->attributeResource();
    entities = attribute::utility::associatableObjects(
      this->Internals->m_attDefinitions.at(0)->associationRule(),
      attResource,
      this->uiManager()->resourceManager());
  }

  int rcount = 0;
  for (const auto& entity : entities)
  {
    std::string name = entity->name();
    auto* item = new QTableWidgetItem(QString::fromStdString(name));
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
      item->setBackground(qtUIManager::contrastWithText(icolor));
    }
    this->Internals->ListTable->setItem(rcount, 1, item);
    ++rcount;
  }
  this->Internals->ListTable->setSortingEnabled(true);
  this->Internals->ListTable->sortByColumn(0, Qt::AscendingOrder);
  this->Internals->ListTable->blockSignals(false);
  this->Internals->ListTable->sortItems(0);
}

QTableWidgetItem* qtComponentAttributeView::getSelectedItem()
{
  return this->Internals->ListTable->selectedItems().count() > 0
    ? this->Internals->ListTable->selectedItems().value(0)
    : nullptr;
}

void qtComponentAttributeView::updateModelAssociation()
{
  this->updateModelEntities();
}

void qtComponentAttributeView::cellChanged(int row, int column)
{
  if (column != 1)
  {
    std::cerr << "ERROR: cell changed at (" << row << "," << column << ")\n";
  }

  // Get selected type
  std::string tname = this->Internals->ListTable->item(row, 1)->text().toStdString();

  auto attRes = this->attributeResource();
  auto resManager = this->uiManager()->resourceManager();
  if (resManager == nullptr)
  {
    return;
  }

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->attributeResource());
  // Get the component of the item
  auto entity = this->object(this->Internals->ListTable->item(row, 0));
  if (entity == nullptr)
  {
    std::cerr << "Could not find selected Item!\n";
    return;
  }

  // Get the current attribute associated with the resource component (if any)
  smtk::attribute::AttributePtr exisitingAtt = this->Internals->getAttribute(entity);
  smtk::attribute::AttributePtr newAtt;
  if (exisitingAtt && exisitingAtt->definition()->displayedTypeName() == tname)
  {
    // The attribute itself didn't change, so we can stop here
    return;
  }
  else if (exisitingAtt)
  {
    // Note though we are removing the attribute from the resource here, we can't call
    // attributeRemoved until after we have created it's replacement since it will
    // cause the view to update.
    attRes->removeAttribute(exisitingAtt);
  }

  // Now create a new attribute for the resource component of the correct type
  // Find the def we need to use
  for (int j = 0; j < currentDefs.size(); ++j)
  {
    if (currentDefs.at(j)->displayedTypeName() == tname)
    {
      newAtt = attRes->createAttribute(currentDefs.at(j));
      newAtt->associate(entity);
      // Notify the application of the new attribute via an "operation"
      this->attributeCreated(newAtt);
      break;
    }
  }
  if (exisitingAtt)
  {
    this->attributeRemoved(exisitingAtt);
  }
  this->Internals->ListTable->selectRow(row);
  this->selectedRowChanged();
}

void qtComponentAttributeView::selectedRowChanged()
{
  this->showCurrentRow(true);
}

void qtComponentAttributeView::showCurrentRow(bool broadcastSelected)
{
  // Lets get the resource component that is selected in the table
  int index = this->Internals->ListTable->currentRow();
  auto entity = this->object(this->Internals->ListTable->item(index, 0));
  // Get the current attribute associated with the resource component (if any)
  auto att = this->Internals->getAttribute(entity);
  this->displayAttribute(att);

  if (broadcastSelected)
  {
    auto sel = this->uiManager()->selection();
    if (sel)
    {
      smtk::resource::PersistentObjectArray selents;
      selents.push_back(entity);
      auto selBit = this->uiManager()->selectionBit();

      sel->modifySelection(
        selents,
        this->Internals->m_selectionSourceName,
        selBit,
        smtk::view::SelectionAction::UNFILTERED_REPLACE);
    }
  }
}

void qtComponentAttributeView::updateSelectedComponent(
  const std::string& /*unused*/,
  smtk::view::SelectionPtr p)
{
  this->Internals->ListTable->blockSignals(true);
  auto selBit = this->uiManager()->selectionBit();
  const auto& selEnts =
    p->currentSelectionByValueAs<smtk::resource::PersistentObjectArray>(selBit, false);
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

void qtComponentAttributeView::onShowCategory()
{
  this->updateUI();
}

void qtComponentAttributeView::updateUI()
{
  this->updateModelEntities();
}

void qtComponentAttributeView::displayAttribute(smtk::attribute::AttributePtr att)
{
  if (this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
  {
    delete this->Internals->CurrentAtt;
    this->Internals->CurrentAtt = nullptr;
  }

  if (att == nullptr)
  {
    this->Internals->AttFrame->setVisible(false);
    return;
  }

  this->Internals->AttFrame->setVisible(true);

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
    smtk::view::Configuration::Component comp;
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

void qtComponentAttributeView::getAllDefinitions()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }

  smtk::attribute::ResourcePtr resource = this->attributeResource();

  std::string attName, defName, val;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;

  // The view should have a single internal component called InstancedAttributes
  if (
    (view->details().numberOfChildren() != 1) ||
    (view->details().child(0).name() != "AttributeTypes"))
  {
    // Should present error message
    return;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  smtk::view::Configuration::Component& attsComp = view->details().child(0);
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
    this->qtBaseAttributeView::getDefinitions(attDef, this->Internals->AllDefs);
    this->Internals->m_attDefinitions.push_back(attDef);
  }

  // sort the list
  std::sort(
    std::begin(this->Internals->AllDefs),
    std::end(this->Internals->AllDefs),
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
      if (
        adef->categories().passes(category.toStdString()) &&
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

void qtComponentAttributeView::showAdvanceLevelOverlay(bool show)
{
  if (this->Internals->CurrentAtt)
  {
    this->Internals->CurrentAtt->showAdvanceLevelOverlay(show);
  }
}

bool qtComponentAttributeView::isEmpty() const
{
  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(this->attributeResource());
  return currentDefs.isEmpty();
}
