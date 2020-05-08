//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeView.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtAssociation2ColumnWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtVoidItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/view/Configuration.h"

#include <QBrush>
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPointer>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVariant>

#include <iostream>
#include <set>
namespace
{
const int status_column = 0;
const int name_column = 1;
const int type_column = 2;
const int color_column = 3;
};

using namespace smtk::attribute;
using namespace smtk::extension;

class qtAttributeViewInternals
{
public:
  ~qtAttributeViewInternals() { delete this->CurrentAtt; }

  // Return a list of definitions based on the current settings
  // of the UI Manager and whether the View is to ignore
  // categories
  const QList<smtk::attribute::DefinitionPtr> getCurrentDefs(
    smtk::extension::qtUIManager* uiManager, bool ignoreCategories) const
  {
    if (ignoreCategories)
    {
      return AllDefs;
    }

    if (uiManager->categoryEnabled())
    {
      auto currentCat = uiManager->currentCategory();
      if (this->AttDefMap.keys().contains(currentCat.c_str()))
      {
        return this->AttDefMap[currentCat.c_str()];
      }
      return this->AllDefs;
    }
    else if (!uiManager->topLevelCategoriesSet())
    {
      return this->AllDefs;
    }
    QList<smtk::attribute::DefinitionPtr> defs;
    foreach (DefinitionPtr attDef, this->AllDefs)
    {
      if (uiManager->passAttributeCategoryCheck(attDef))
      {
        defs.push_back(attDef);
      }
    }
    return defs;
  }

  qtTableWidget* ListTable;
  qtTableWidget* ValuesTable;

  QPushButton* AddButton;
  QComboBox* DefsCombo;
  QLabel* DefLabel;
  QPushButton* DeleteButton;
  QPushButton* CopyButton;

  QFrame* ButtonsFrame;
  QFrame* TopFrame; // top

  QPointer<qtAssociationWidget> AssociationsWidget;

  // <category, AttDefinitions>
  QMap<QString, QList<smtk::attribute::DefinitionPtr> > AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllDefs;

  // Attribute widget
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QFrame> AttFrame;

  //Last selected attribute
  WeakAttributePtr selectedAttribute;

  // Model for filtering the attribute by combobox.
  QPointer<QStandardItemModel> checkableAttComboModel;
  QMap<std::string, Qt::CheckState> AttSelections;
  qtCheckItemComboBox* SelectAttCombo;

  QMap<std::string, Qt::CheckState> AttProperties;
  std::vector<smtk::attribute::DefinitionPtr> m_attDefinitions;
  bool m_okToCreateModelEntities;
  smtk::model::BitFlags m_modelEntityMask;
  std::map<std::string, smtk::view::Configuration::Component> m_attCompMap;
  QIcon m_alertIcon;
  bool m_showTopButtons;

  smtk::operation::Observers::Key m_observerKey;
};

qtBaseView* qtAttributeView::createViewWidget(const smtk::view::Information& info)
{
  qtAttributeView* view = new qtAttributeView(info);
  view->buildUI();
  return view;
}

qtAttributeView::qtAttributeView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  m_internals = new qtAttributeViewInternals;
  m_internals->m_alertIcon = QIcon(this->uiManager()->alertPixmap());
  smtk::view::ConfigurationPtr view = this->getObject();
  m_hideAssociations = false;
  m_allAssociatedMode = false;
  m_disableNameField = false;

  if (view)
  {
    view->details().attributeAsBool("HideAssociations", m_hideAssociations);
    m_allAssociatedMode = view->details().attributeAsBool("RequireAllAssociated");
    view->details().attributeAsBool("DisableNameField", m_disableNameField);
  }
}

qtAttributeView::~qtAttributeView()
{
  if (m_internals->m_observerKey.assigned())
  {
    auto opManager = this->uiManager()->operationManager();
    if (opManager != nullptr)
    {
      opManager->observers().erase(m_internals->m_observerKey);
    }
  }
  delete m_internals;
}

const QMap<QString, QList<smtk::attribute::DefinitionPtr> >& qtAttributeView::attDefinitionMap()
  const
{
  return m_internals->AttDefMap;
}

smtk::extension::qtAssociationWidget* qtAttributeView::createAssociationWidget(
  QWidget* parent, qtBaseView* view)
{
  return new qtAssociation2ColumnWidget(parent, view);
}

void qtAttributeView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->getObject();
  if (view == nullptr)
  {
    return;
  }
  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  // m_internals->AttDefMap has to be initialized before getAllDefinitions()
  // since the getAllDefinitions() call needs the categories list in AttDefMap
  // Create a map for all catagories so we can cluster the definitions
  m_internals->AttDefMap.clear();
  const ResourcePtr attResource = this->uiManager()->attResource();
  std::set<std::string>::const_iterator it;
  const std::set<std::string>& cats = attResource->categories();

  for (it = cats.begin(); it != cats.end(); it++)
  {
    QList<smtk::attribute::DefinitionPtr> attdeflist;
    m_internals->AttDefMap[it->c_str()] = attdeflist;
  }

  // Initialize definition info
  this->getAllDefinitions();

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation(Qt::Vertical);

  QFrame* TopFrame = new QFrame(frame);
  m_internals->AttFrame = new QFrame(frame);

  m_internals->TopFrame = TopFrame;
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QVBoxLayout* TopLayout = new QVBoxLayout(TopFrame);
  TopLayout->setMargin(0);
  TopFrame->setSizePolicy(sizeFixedPolicy);
  QVBoxLayout* AttFrameLayout = new QVBoxLayout(m_internals->AttFrame);
  AttFrameLayout->setMargin(0);
  m_internals->AttFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  QSizePolicy tableSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // create a list box for all the entries
  m_internals->ListTable = new qtTableWidget(frame);
  m_internals->ListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_internals->ListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_internals->ListTable->setSizePolicy(tableSizePolicy);

  // Buttons frame
  m_internals->ButtonsFrame = new QFrame(frame);
  QHBoxLayout* buttonLayout = new QHBoxLayout(m_internals->ButtonsFrame);
  buttonLayout->setMargin(0);
  m_internals->ButtonsFrame->setSizePolicy(sizeFixedPolicy);
  if (view)
  {
    m_internals->m_showTopButtons = !view->details().attributeAsBool("DisableTopButtons");
    m_internals->ButtonsFrame->setVisible(m_internals->m_showTopButtons);
  }

  m_internals->AddButton = new QPushButton("New", m_internals->ButtonsFrame);
  m_internals->AddButton->setSizePolicy(sizeFixedPolicy);
  m_internals->DeleteButton = new QPushButton("Delete", m_internals->ButtonsFrame);
  m_internals->DeleteButton->setSizePolicy(sizeFixedPolicy);
  m_internals->CopyButton = new QPushButton("Copy", m_internals->ButtonsFrame);
  m_internals->CopyButton->setSizePolicy(sizeFixedPolicy);

  //If we have more than 1 def then create a combo box for selecting a def,
  // else just create a label
  if (m_internals->AllDefs.size() > 1)
  {
    m_internals->DefsCombo = new QComboBox(m_internals->ButtonsFrame);
    m_internals->DefsCombo->setVisible(false);
    buttonLayout->addWidget(m_internals->DefsCombo);
    m_internals->DefLabel = nullptr;
  }
  else
  {
    m_internals->DefLabel = new QLabel(m_internals->ButtonsFrame);
    m_internals->DefLabel->setText(m_internals->AllDefs[0]->displayedTypeName().c_str());
    m_internals->DefLabel->setVisible(false);
    buttonLayout->addWidget(m_internals->DefLabel);
    m_internals->DefsCombo = nullptr;
  }
  buttonLayout->addWidget(m_internals->AddButton);
  buttonLayout->addWidget(m_internals->CopyButton);
  buttonLayout->addWidget(m_internals->DeleteButton);

  // Attribute table
  m_internals->ValuesTable = new qtTableWidget(frame);
  m_internals->ValuesTable->setSizePolicy(tableSizePolicy);

  TopLayout->addWidget(m_internals->ButtonsFrame);
  TopLayout->addWidget(m_internals->ListTable);
  // REMOVED THIS TO SIMPLY LAYOUT FOR VIEW BY ATTRIBUTES
  // MODE - SHOULD BE REMOVED IF WE FACTOR OUT THE VIEW BY PROPERTY MODE
  //BottomLayout->addWidget(m_internals->ValuesTable);
  m_internals->ValuesTable->setVisible(false);

  // Attribte frame
  m_internals->AttFrame = new QFrame(frame);
  new QVBoxLayout(m_internals->AttFrame);

  frame->addWidget(TopFrame);
  frame->addWidget(m_internals->AttFrame);

  // the association widget
  m_internals->AssociationsWidget = this->createAssociationWidget(frame, this);
  this->updateAssociationEnableState(smtk::attribute::AttributePtr());
  frame->addWidget(m_internals->AssociationsWidget);

  m_internals->ValuesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_internals->ValuesTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_internals->ValuesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  if (view)
  {
    m_internals->m_showTopButtons = !view->details().attributeAsBool("DisableTopButtons");
    m_internals->ButtonsFrame->setVisible(m_internals->m_showTopButtons);
  }

  // signals/slots
  QObject::connect(m_internals->AssociationsWidget, SIGNAL(attAssociationChanged()), this,
    SLOT(associationsChanged()));

  QObject::connect(
    m_internals->AssociationsWidget, SIGNAL(availableChanged()), this, SIGNAL(modified()));

  // We want the signals that may change the attribute to be displayed Queued instead of
  // Direct so that QLineEdit::edittingFinished signals are processed prior to these.
  QObject::connect(m_internals->ListTable, SIGNAL(itemClicked(QTableWidgetItem*)), this,
    SLOT(onListBoxClicked(QTableWidgetItem*)), Qt::QueuedConnection);
  QObject::connect(m_internals->ListTable, SIGNAL(itemSelectionChanged()), this,
    SLOT(onListBoxSelectionChanged()), Qt::QueuedConnection);
  QObject::connect(m_internals->ListTable, SIGNAL(itemChanged(QTableWidgetItem*)), this,
    SLOT(onAttributeNameChanged(QTableWidgetItem*)));
  // we need this so that the attribute name will also be changed
  // when a recorded test is play back, which is using setText
  // on the underline QLineEdit of the cell.
  QObject::connect(m_internals->ListTable, SIGNAL(cellChanged(int, int)), this,
    SLOT(onAttributeCellChanged(int, int)), Qt::QueuedConnection);

  QObject::connect(m_internals->AddButton, SIGNAL(clicked()), this, SLOT(onCreateNew()));
  QObject::connect(m_internals->CopyButton, SIGNAL(clicked()), this, SLOT(onCopySelected()));
  QObject::connect(m_internals->DeleteButton, SIGNAL(clicked()), this, SLOT(onDeleteSelected()));

  QObject::connect(m_internals->ValuesTable, SIGNAL(itemChanged(QTableWidgetItem*)), this,
    SLOT(onAttributeValueChanged(QTableWidgetItem*)));
  //QObject::connect(m_internals->ValuesTable,
  //  SIGNAL(keyPressed (QKeyEvent *)),
  //  this, SLOT(onAttributeTableKeyPress(QKeyEvent * )));
  m_internals->ValuesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_internals->ValuesTable->setSelectionMode(QAbstractItemView::SingleSelection);

  m_internals->ValuesTable->setVisible(false);

  this->Widget = frame;

  this->updateModelAssociation();

  auto opManager = uiManager()->operationManager();
  QPointer<qtAttributeView> guardedObject(this);
  if (opManager != nullptr)
  {
    m_internals->m_observerKey = opManager->observers().insert(
      [guardedObject](const smtk::operation::Operation& oper, smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        if (guardedObject == nullptr)
        {
          return 0;
        }
        return guardedObject->handleOperationEvent(oper, event, result);
      },
      "qtInstancedView: Refresh qtAttributeView when components are modified.");
  }
}

void qtAttributeView::updateModelAssociation()
{
  this->updateUI();
}

smtk::attribute::Attribute* qtAttributeView::getRawAttributeFromItem(QTableWidgetItem* item)
{
  return (item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : nullptr);
}

smtk::attribute::AttributePtr qtAttributeView::getAttributeFromItem(QTableWidgetItem* item)
{
  smtk::attribute::Attribute* raw = this->getRawAttributeFromItem(item);
  if (raw == nullptr)
  {
    return smtk::attribute::AttributePtr();
  }

  return raw->shared_from_this();
}

QTableWidgetItem* qtAttributeView::getSelectedItem()
{
  auto selectedItems = m_internals->ListTable->selectedItems();
  int n = selectedItems.count();
  for (int i = 0; i < n; i++)
  {
    if (selectedItems.value(i)->column() == name_column)
    {
      return selectedItems.value(i);
    }
  }
  return nullptr;
}

smtk::attribute::AttributePtr qtAttributeView::getSelectedAttribute()
{
  return this->getAttributeFromItem(this->getSelectedItem());
}

void qtAttributeView::updateAssociationEnableState(smtk::attribute::AttributePtr theAtt)
{
  if (m_hideAssociations)
  {
    m_internals->AssociationsWidget->setVisible(false);
    return;
  }
  if (theAtt)
  {
    bool showAssocs = (theAtt->definition()->associationRule() != nullptr);
    if (showAssocs)
    {
      m_internals->AssociationsWidget->showEntityAssociation(theAtt);
    }
    m_internals->AssociationsWidget->setVisible(showAssocs);
    return;
  }

  if (m_allAssociatedMode && !m_internals->AllDefs.empty())
  {
    m_internals->AssociationsWidget->showEntityAssociation(m_internals->AllDefs.at(0));
    m_internals->AssociationsWidget->setVisible(true);
    return;
  }
  m_internals->AssociationsWidget->setVisible(false);
}

void qtAttributeView::onListBoxSelectionChanged()
{
  m_internals->ValuesTable->blockSignals(true);
  m_internals->ValuesTable->clear();
  m_internals->ValuesTable->setRowCount(0);
  m_internals->ValuesTable->setColumnCount(0);
  QTableWidgetItem* current = this->getSelectedItem();

  if (current)
  {
    smtk::attribute::AttributePtr dataItem = this->getAttributeFromItem(current);
    this->updateAssociationEnableState(dataItem);
    if (dataItem)
    {
      this->updateTableWithAttribute(dataItem);
      emit attributeSelected(dataItem);
    }
  }
  else
  {
    delete m_internals->CurrentAtt;
    m_internals->selectedAttribute = AttributePtr();
    m_internals->CurrentAtt = nullptr;
    this->updateAssociationEnableState(smtk::attribute::AttributePtr());
  }

  m_internals->ValuesTable->blockSignals(false);
  m_internals->ValuesTable->resizeRowsToContents();
  m_internals->ValuesTable->resizeColumnsToContents();
  m_internals->ValuesTable->update();
}

void qtAttributeView::onAttributeNameChanged(QTableWidgetItem* item)
{
  smtk::attribute::AttributePtr aAttribute = this->getAttributeFromItem(item);
  if (aAttribute && item->text().toStdString() != aAttribute->name())
  {
    ResourcePtr attResource = aAttribute->definition()->resource();
    // Lets see if the name is in use
    auto att = attResource->findAttribute(item->text().toStdString());
    if (att != nullptr)
    {
      std::string s;
      s = "Can't rename " + aAttribute->type() + ":" + aAttribute->name() +
        ".  There already exists an attribute of type: " + att->type() + " named " + att->name() +
        ".";

      QMessageBox::warning(this->Widget, "Attribute Can't be Renamed", s.c_str());
      item->setText(aAttribute->name().c_str());
      return;
    }
    attResource->rename(aAttribute, item->text().toStdString());
    this->attributeChanged(aAttribute);
    //aAttribute->definition()->setLabel(item->text().toAscii().constData());
  }
}

void qtAttributeView::onAttributeCellChanged(int row, int col)
{
  if (col == 0)
  {
    QTableWidgetItem* item = m_internals->ListTable->item(row, col);
    this->onAttributeNameChanged(item);
  }
}

void qtAttributeView::insertTableColumn(
  QTableWidget* vtWidget, int insertCol, const QString& title, int advancedlevel)
{
  vtWidget->insertColumn(insertCol);
  vtWidget->setHorizontalHeaderItem(insertCol, new QTableWidgetItem(title));

  if (advancedlevel)
  {
    vtWidget->horizontalHeaderItem(insertCol)->setFont(this->uiManager()->advancedFont());
  }
}

void qtAttributeView::onAttributeValueChanged(QTableWidgetItem* item)
{
  Item* linkedData = item ? static_cast<Item*>(item->data(Qt::UserRole).value<void*>()) : nullptr;

  if (linkedData && linkedData->isOptional())
  {
    linkedData->setIsEnabled(item->checkState() == Qt::Checked);
    this->updateChildWidgetsEnableState(linkedData->shared_from_this(), item);
  }
}

void qtAttributeView::updateChildWidgetsEnableState(
  smtk::attribute::ItemPtr attItem, QTableWidgetItem* item)
{
  if (!item || !attItem || !attItem->isOptional())
  {
    return;
  }
  bool bEnabled = attItem->isEnabled();
  int startRow = item->row();

  if (attItem->type() == smtk::attribute::Item::GroupType)
  {
    smtk::attribute::GroupItemPtr grpItem = dynamic_pointer_cast<GroupItem>(attItem);
    if (grpItem)
    {
      int numItems = static_cast<int>(grpItem->numberOfItemsPerGroup());
      for (int j = 0; j < numItems; j++) // expecting one item for each column
      {
        this->updateItemWidgetsEnableState(grpItem->item(j), startRow, bEnabled);
      }
    }
  }
  else
  {
    this->updateItemWidgetsEnableState(attItem, startRow, bEnabled);
  }
}

void qtAttributeView::updateItemWidgetsEnableState(
  smtk::attribute::ItemPtr inData, int& startRow, bool enabled)
{
  QTableWidget* tableWidget = m_internals->ValuesTable;
  if (inData->type() == smtk::attribute::Item::AttributeRefType)
  {
    QWidget* cellWidget = tableWidget->cellWidget(startRow, 1);
    if (cellWidget)
    {
      cellWidget->setEnabled(enabled);
    }
  }
  else if (inData->type() == smtk::attribute::Item::VoidType)
  {
    QWidget* cellWidget = tableWidget->cellWidget(startRow, 0);
    if (cellWidget)
    {
      cellWidget->setEnabled(enabled);
    }
  }
  else if (dynamic_pointer_cast<ValueItem>(inData))
  {
    smtk::attribute::ValueItemPtr linkedData = dynamic_pointer_cast<ValueItem>(inData);
    for (std::size_t row = 0; row < linkedData->numberOfValues(); row++, startRow++)
    {
      QWidget* cellWidget = tableWidget->cellWidget(startRow, 1);
      if (cellWidget)
      {
        cellWidget->setEnabled(enabled);
      }
    }
  }
}

smtk::attribute::DefinitionPtr qtAttributeView::getCurrentDef() const
{
  if (m_internals->m_attDefinitions.empty())
  {
    return nullptr;
  }
  if (m_internals->AllDefs.size() == 1)
  {
    return m_internals->AllDefs[0];
  }

  QString strDef = m_internals->DefsCombo->currentText();

  foreach (attribute::DefinitionPtr attDef,
    m_internals->getCurrentDefs(this->uiManager(), m_ignoreCategories))
  {
    std::string txtDef = attDef->displayedTypeName();
    if (strDef == QString::fromUtf8(txtDef.c_str()))
    {
      return attDef;
    }
  }
  return nullptr;
}

void qtAttributeView::onCreateNew()
{
  smtk::attribute::DefinitionPtr newAttDef = this->getCurrentDef();
  if (newAttDef != nullptr)
  {
    this->createNewAttribute(newAttDef);
  }
}

void qtAttributeView::createNewAttribute(smtk::attribute::DefinitionPtr attDef)
{
  if (!attDef)
  {
    return;
  }

  ResourcePtr attResource = attDef->resource();

  smtk::attribute::AttributePtr newAtt = attResource->createAttribute(attDef->type());
  this->attributeCreated(newAtt);
  QTableWidgetItem* item = this->addAttributeListItem(newAtt);
  if (item)
  {
    m_internals->ListTable->selectRow(item->row());
  }
  emit this->numOfAttributesChanged();
  emit qtBaseView::modified();
}

void qtAttributeView::onCopySelected()
{
  smtk::attribute::AttributePtr newObject, selObject = this->getSelectedAttribute();
  if (!selObject)
  {
    return;
  }

  ResourcePtr attResource = selObject->attributeResource();
  newObject = attResource->copyAttribute(selObject);
  if (newObject)
  {
    QTableWidgetItem* item = this->addAttributeListItem(newObject);
    if (item)
    {
      m_internals->ListTable->selectRow(item->row());
    }
    emit this->numOfAttributesChanged();
    emit qtBaseView::modified();
  }
}

void qtAttributeView::onDeleteSelected()
{
  smtk::attribute::AttributePtr selObject = this->getSelectedAttribute();
  if (selObject != nullptr)
  {
    if (this->deleteAttribute(selObject))
    {
      std::string keyName = selObject->name();
      m_internals->AttSelections.remove(keyName);
      this->attributeRemoved(selObject);

      QTableWidgetItem* selItem = this->getSelectedItem();
      if (selItem != nullptr)
      {
        m_internals->ListTable->removeRow(selItem->row());
        emit this->numOfAttributesChanged();
        emit qtBaseView::modified();
      }
    }
    else
    {
      std::string s("Can't remove attribute ");
      s.append(selObject->name()).append(" - Might be used as a prerequisite!");
      QMessageBox::warning(this->parentWidget(), tr("Failure to Remove Attribute"), s.c_str());
    }
  }
}

bool smtk::extension::qtAttributeView::deleteAttribute(smtk::attribute::AttributePtr att)
{
  bool status = false;

  if (att != nullptr)
  {
    attribute::DefinitionPtr attDef = att->definition();
    ResourcePtr attResource = attDef->resource();
    status = attResource->removeAttribute(att);
  }

  return status;
}

QTableWidgetItem* qtAttributeView::addAttributeListItem(smtk::attribute::AttributePtr childData)
{
  QTableWidgetItem* item =
    new QTableWidgetItem(QString::fromUtf8(childData->name().c_str()), smtk_USER_DATA_TYPE);
  Qt::ItemFlags nonEditableFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  Qt::ItemFlags itemFlags(nonEditableFlags | (m_disableNameField ? 0x0 : Qt::ItemIsEditable));
  QVariant vdata;
  vdata.setValue(static_cast<void*>(childData.get()));
  item->setFlags(itemFlags);

  int numRows = m_internals->ListTable->rowCount();
  m_internals->ListTable->setRowCount(++numRows);
  m_internals->ListTable->setItem(numRows - 1, name_column, item);

  // add the type column too.
  std::string txtDef = childData->definition()->displayedTypeName();

  QTableWidgetItem* defitem =
    new QTableWidgetItem(QString::fromUtf8(txtDef.c_str()), smtk_USER_DATA_TYPE);
  defitem->setFlags(nonEditableFlags);
  m_internals->ListTable->setItem(numRows - 1, type_column, defitem);

  // ToDo: Reactivate Color Option when we are ready to use it
  // Lets see if we need to show an alert icon cause the attribute is not
  // valid
  if (!this->uiManager()->checkAttributeValidity(childData.get()))
  {
    QTableWidgetItem* statusItem =
      new QTableWidgetItem(m_internals->m_alertIcon, "", smtk_USER_DATA_TYPE);
    m_internals->ListTable->setItem(numRows - 1, status_column, statusItem);
  }

  return item;
}

void qtAttributeView::onViewBy()
{
  if (m_internals->m_attDefinitions.empty())
  {
    return;
  }

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    m_internals->getCurrentDefs(this->uiManager(), m_ignoreCategories);
  m_internals->AddButton->setEnabled(currentDefs.count() > 0);

  m_internals->ButtonsFrame->setVisible(m_internals->m_showTopButtons);
  m_internals->ListTable->setVisible(true);
  m_internals->ListTable->blockSignals(true);
  m_internals->ListTable->clear();
  m_internals->ListTable->setRowCount(0);

  // ToDo: Reactivate Color Option when we are ready to use it
  //int numCols = viewAtt ? 4 : 3;
  int numCols = 3;
  m_internals->ListTable->setColumnCount(numCols);
  m_internals->ListTable->setHorizontalHeaderItem(status_column, new QTableWidgetItem(""));
  m_internals->ListTable->setHorizontalHeaderItem(name_column, new QTableWidgetItem("Name"));
  // Lets set up the column behavior
  // The Type and Status Columns should be size to fit their contents while
  // the Name field should stretch to take up the space
  m_internals->ListTable->setHorizontalHeaderItem(type_column, new QTableWidgetItem("Type"));
  m_internals->ListTable->horizontalHeader()->setSectionResizeMode(
    status_column, QHeaderView::ResizeToContents);
  m_internals->ListTable->horizontalHeader()->setSectionResizeMode(
    name_column, QHeaderView::Stretch);
  m_internals->ListTable->horizontalHeader()->setSectionResizeMode(
    type_column, QHeaderView::ResizeToContents);
  m_internals->ListTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  m_internals->ListTable->horizontalHeader()->setStretchLastSection(false);
  if (m_internals->AllDefs.size() == 1)
  {
    // If there is only one attribute type then there is no reason to
    // show the type column
    m_internals->ListTable->setColumnHidden(type_column, true);
  }
  if (numCols == 4)
  {
    m_internals->ListTable->setHorizontalHeaderItem(color_column, new QTableWidgetItem("Color"));
  }
  if (m_internals->AllDefs.size() == 1)
  {
    m_internals->DefLabel->setVisible(true);
  }
  else
  {
    m_internals->DefsCombo->clear();
    foreach (attribute::DefinitionPtr attDef, currentDefs)
    {
      if (!attDef->isAbstract())
      {
        std::string txtDef = attDef->displayedTypeName();
        m_internals->DefsCombo->addItem(QString::fromUtf8(txtDef.c_str()));
      }
    }

    m_internals->DefsCombo->setCurrentIndex(0);
    m_internals->DefsCombo->setVisible(true);
  }
  foreach (attribute::DefinitionPtr attDef, currentDefs)
  {
    this->onViewByWithDefinition(attDef);
  }

  m_internals->ListTable->blockSignals(false);

  QSplitter* frame = qobject_cast<QSplitter*>(this->Widget);
  if (m_internals->ListTable->rowCount() && !this->getSelectedItem())
  {
    // so switch tabs would not reset selection
    if (!m_internals->AssociationsWidget->hasSelectedItem())
    {
      m_internals->ListTable->selectRow(0);
    }
  }
  else
  {
    this->onListBoxSelectionChanged();
  }
  frame->handle(1)->setEnabled(true);
}

void qtAttributeView::onViewByWithDefinition(smtk::attribute::DefinitionPtr attDef)
{
  smtk::attribute::AttributePtr currentAtt = m_internals->selectedAttribute.lock();
  std::vector<smtk::attribute::AttributePtr> result;
  ResourcePtr attResource = attDef->resource();
  attResource->findAttributes(attDef, result);
  if (!result.empty())
  {
    std::vector<smtk::attribute::AttributePtr>::iterator it;
    for (it = result.begin(); it != result.end(); ++it)
    {
      if ((*it)->definition() != attDef)
      {
        continue;
      }
      QTableWidgetItem* item = this->addAttributeListItem(*it);
      if ((*it)->definition()->advanceLevel())
      {
        item->setFont(this->uiManager()->advancedFont());
      }
      if (currentAtt == (*it))
      {
        m_internals->ListTable->setCurrentItem(item);
      }
    }
  }
}

void qtAttributeView::onShowCategory()
{
  this->updateUI();
}

void qtAttributeView::updateUI()
{
  this->onViewBy();
}

void qtAttributeView::updateTableWithAttribute(smtk::attribute::AttributePtr att)
{
  m_internals->ValuesTable->setVisible(false);
  m_internals->AttFrame->setVisible(true);

  if (m_internals->CurrentAtt && m_internals->CurrentAtt->widget())
  {
    delete m_internals->CurrentAtt;
  }

  int currentLen = this->fixedLabelWidth();
  int tmpLen = this->uiManager()->getWidthOfAttributeMaxLabel(
    att->definition(), this->uiManager()->advancedFont());
  this->setFixedLabelWidth(tmpLen);
  auto it = m_internals->m_attCompMap.find(att->definition()->type());
  if (it != m_internals->m_attCompMap.end())
  {
    m_internals->CurrentAtt = new qtAttribute(att, it->second, m_internals->AttFrame, this);
  }
  else
  {
    smtk::view::Configuration::Component comp;
    m_internals->CurrentAtt = new qtAttribute(att, comp, m_internals->AttFrame, this);
  }
  m_internals->selectedAttribute = att;
  // By default use the basic layout with no model associations since this class
  // takes care of it
  m_internals->CurrentAtt->createBasicLayout(false);
  this->setFixedLabelWidth(currentLen);
  if (m_internals->CurrentAtt)
  {
    if (m_internals->CurrentAtt->widget())
    {
      m_internals->AttFrame->layout()->addWidget(m_internals->CurrentAtt->widget());
      QObject::connect(m_internals->CurrentAtt, SIGNAL(itemModified(qtItem*)), this,
        SLOT(onItemChanged(qtItem*)), Qt::QueuedConnection);
      if (this->advanceLevelVisible())
      {
        m_internals->CurrentAtt->showAdvanceLevelOverlay(true);
      }
    }
    m_internals->DeleteButton->setEnabled(
      this->uiManager()->passAdvancedCheck(att->advanceLevel(1)));
  }
}

void qtAttributeView::initSelectAttCombo(smtk::attribute::DefinitionPtr attDef)
{
  m_internals->SelectAttCombo->blockSignals(true);
  m_internals->SelectAttCombo->clear();
  m_internals->SelectAttCombo->init();
  m_internals->checkableAttComboModel->disconnect();

  if (!attDef)
  {
    m_internals->SelectAttCombo->blockSignals(false);
    return;
  }

  std::vector<smtk::attribute::AttributePtr> result;
  ResourcePtr attResource = attDef->resource();
  attResource->findAttributes(attDef, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  int row = 1;
  for (it = result.begin(); it != result.end(); ++it, ++row)
  {
    QStandardItem* item = new QStandardItem;
    item->setText((*it)->name().c_str());
    std::string keyName = (*it)->name();
    if (!m_internals->AttSelections.contains(keyName))
    {
      m_internals->AttSelections[keyName] = Qt::Unchecked;
    }

    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    //item->setData(m_internals->AttSelections[keyName], Qt::CheckStateRole);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    item->setCheckable(true);
    item->setCheckState(m_internals->AttSelections[keyName]);
    QVariant vdata;
    vdata.setValue(static_cast<void*>((*it).get()));
    item->setData(vdata, Qt::UserRole);
    m_internals->checkableAttComboModel->insertRow(row, item);
    if ((*it)->definition()->advanceLevel())
    {
      item->setFont(this->uiManager()->advancedFont());
    }
  }
}

int qtAttributeView::currentViewBy()
{
  return 0;
}

void qtAttributeView::getAllDefinitions()
{
  smtk::view::ConfigurationPtr view = this->getObject();
  if (!view)
  {
    return;
  }

  smtk::attribute::ResourcePtr resource = this->uiManager()->attResource();

  std::string attName, defName, val;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;
  bool flag;

  // The view should have a single internal component called InstancedAttributes
  if ((view->details().numberOfChildren() != 1) ||
    (view->details().child(0).name() != "AttributeTypes"))
  {
    // Should present error message
    return;
  }

  if (view->details().attributeAsBool("CreateEntities", flag))
  {
    m_internals->m_okToCreateModelEntities = flag;
  }
  else
  {
    m_internals->m_okToCreateModelEntities = false;
  }

  if (view->details().attribute("ModelEntityFilter", val))
  {
    smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(val);
    m_internals->m_modelEntityMask = flags;
  }
  else
  {
    m_internals->m_modelEntityMask = 0;
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

    m_internals->m_attCompMap[defName] = attsComp.child(i);
    this->qtBaseAttributeView::getDefinitions(attDef, m_internals->AllDefs);
    m_internals->m_attDefinitions.push_back(attDef);
  }

  // sort the list
  std::sort(std::begin(m_internals->AllDefs), std::end(m_internals->AllDefs),
    [](smtk::attribute::DefinitionPtr a, smtk::attribute::DefinitionPtr b) {
      return a->displayedTypeName() < b->displayedTypeName();
    });

  foreach (smtk::attribute::DefinitionPtr adef, m_internals->AllDefs)
  {
    foreach (QString category, m_internals->AttDefMap.keys())
    {
      if (adef->categories().passes(category.toStdString()) &&
        !m_internals->AttDefMap[category].contains(adef))
      {
        m_internals->AttDefMap[category].push_back(adef);
      }
    }
  }
}

void qtAttributeView::onListBoxClicked(QTableWidgetItem* item)
{
  bool isColor = item->column() == color_column;
  if (isColor)
  {
    QTableWidgetItem* selItem = m_internals->ListTable->item(item->row(), 0);
    smtk::attribute::AttributePtr selAtt = this->getAttributeFromItem(selItem);
    QBrush bgBrush = item->background();
    QColor color = QColorDialog::getColor(
      bgBrush.color(), this->Widget, "Choose Attribute Color", QColorDialog::DontUseNativeDialog);
    if (color.isValid() && color != bgBrush.color() && selAtt)
    {
      bgBrush.setColor(color);
      item->setBackground(bgBrush);
      selAtt->setColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
      emit this->attColorChanged();
    }
    if (!selItem->isSelected())
    {
      m_internals->ListTable->setCurrentItem(selItem);
      selItem->setSelected(true);
    }
  }
}

void qtAttributeView::childrenResized()
{
  if (m_internals->ValuesTable->isVisible())
  {
    m_internals->ValuesTable->resizeRowsToContents();
    m_internals->ValuesTable->resizeColumnsToContents();
    m_internals->ValuesTable->update();
    this->Widget->update();
  }
}

void qtAttributeView::showAdvanceLevelOverlay(bool show)
{
  if (m_internals->CurrentAtt)
  {
    m_internals->CurrentAtt->showAdvanceLevelOverlay(show);
  }
  this->qtBaseAttributeView::showAdvanceLevelOverlay(show);
}

bool qtAttributeView::isEmpty() const
{
  QList<smtk::attribute::DefinitionPtr> currentDefs =
    m_internals->getCurrentDefs(this->uiManager(), m_ignoreCategories);
  return currentDefs.isEmpty();
}

void qtAttributeView::associationsChanged()
{
  if (m_internals->CurrentAtt == nullptr)
  {
    return;
  }
  this->updateAttributeStatus(m_internals->CurrentAtt->attribute().get());
  this->valueChanged(m_internals->CurrentAtt->attribute()->associations());
  emit this->modified(m_internals->CurrentAtt->attribute()->associations());
  emit this->attAssociationChanged();
  emit qtBaseView::modified();
  std::vector<std::string> items;
  items.emplace_back("_associations");
  this->attributeChanged(m_internals->CurrentAtt->attribute(), items);
}

void qtAttributeView::onItemChanged(qtItem* qitem)
{
  if (qitem == nullptr)
  {
    return;
  }
  auto item = qitem->item();
  if (item == nullptr)
  {
    return;
  }
  auto attribute = item->attribute();
  if (attribute == nullptr)
  {
    return;
  }
  this->updateAttributeStatus(attribute.get());
  this->valueChanged(item);
  std::vector<std::string> items;
  items.push_back(item->name());
  this->attributeChanged(attribute, items);
}
void qtAttributeView::updateAttributeStatus(Attribute* att)
{
  if (att == nullptr)
  {
    return;
  }
  for (int i = 0; i < m_internals->ListTable->rowCount(); i++)
  {
    QTableWidgetItem* item = m_internals->ListTable->item(i, name_column);
    Attribute* listAtt =
      item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : nullptr;
    if (listAtt == nullptr)
    {
      continue;
    }
    if (listAtt == att)
    {
      if (this->uiManager()->checkAttributeValidity(att))
      {
        m_internals->ListTable->setItem(i, status_column, nullptr);
      }
      else
      {
        QTableWidgetItem* statusItem =
          new QTableWidgetItem(m_internals->m_alertIcon, "", smtk_USER_DATA_TYPE);
        m_internals->ListTable->setItem(i, status_column, statusItem);
      }
    }
  }
}

int qtAttributeView::handleOperationEvent(const smtk::operation::Operation& op,
  smtk::operation::EventType event, smtk::operation::Operation::Result result)
{
  if (event != smtk::operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  // Since the Signal Operation originates from a Qt Signal
  // being fired we can ignore this
  if (op.typeName() == smtk::common::typeName<smtk::attribute::Signal>())
  {
    return 0;
  }

  //In the case of modified components we only need to look at the
  // current attribute being displayed
  smtk::attribute::ComponentItemPtr compItem;
  std::size_t i, n;
  if (m_internals->CurrentAtt != nullptr)
  {
    compItem = result->findComponent("modified");
    n = compItem->numberOfValues();
    for (i = 0; i < n; i++)
    {
      if (compItem->isSet(i) && (compItem->value(i) == m_internals->CurrentAtt->attribute()))
      {
        // Update the attribute's items
        auto items = m_internals->CurrentAtt->items();
        for (auto item : items)
        {
          item->updateItemData();
        }
        break; // we don't have to keep looking for this ComponentPtr
      }
    }
  }

  // Check for expunged components - this case we need to look at all of the attributes
  // in the table and remove any that have been expunged
  compItem = result->findComponent("expunged");
  n = compItem->numberOfValues();
  if (n)
  {
    for (i = 0; i < n; i++)
    {
      if (compItem->isSet(i))
      {
        int row, numRows = m_internals->ListTable->rowCount();
        for (row = 0; row < numRows; ++row)
        {
          QTableWidgetItem* item = m_internals->ListTable->item(row, name_column);
          smtk::attribute::Attribute* att = this->getRawAttributeFromItem(item);
          if (compItem->value(i).get() == att)
          {
            m_internals->ListTable->removeRow(row);
            break;
          }
        }
      }
    }
  }

  // In the case of created components we need to see if anything would
  // cause us to reset the list view
  smtk::attribute::DefinitionPtr currentDef = this->getCurrentDef();
  if (currentDef == nullptr)
  {
    return 0; // nothing to do
  }

  compItem = result->findComponent("created");
  n = compItem->numberOfValues();
  for (i = 0; i < n; i++)
  {
    if (compItem->isSet(i))
    {
      auto att = dynamic_pointer_cast<smtk::attribute::Attribute>(compItem->value(i));
      if (att == nullptr)
      {
        continue;
      }
      smtk::attribute::DefinitionPtr attDef = att->definition();
      if (attDef->isA(currentDef))
      {
        this->addAttributeListItem(att);
      }
    }
  }
  return 0;
}

bool qtAttributeView::isValid() const
{
  for (int i = 0; i < m_internals->ListTable->rowCount(); i++)
  {
    if (m_internals->ListTable->item(i, status_column) != nullptr)
    {
      return false;
    }
  }
  if (m_internals->AssociationsWidget != nullptr)
  {
    return m_internals->AssociationsWidget->isValid();
  }
  return true;
}
