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

#include "smtk/extension/qt/qtAssociation2ColumnWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtNotEditableDelegate.h"
#include "smtk/extension/qt/qtRegexDelegate.h"
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
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

#include <QAbstractItemDelegate>
#include <QBrush>
#include <QCheckBox>
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPointer>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGlobal>

#include <iostream>
#include <set>
namespace
{
const int status_column = 0;
const int name_column = 1;
const int type_column = 2;
const int color_column = 3;
}; // namespace

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
    smtk::extension::qtUIManager* uiManager,
    bool ignoreCategories) const
  {
    QList<smtk::attribute::DefinitionPtr> defs;
    bool includeAdvanceLevel = false;
    int advLevel = 0;
    if (uiManager)
    {
      includeAdvanceLevel = true;
      advLevel = uiManager->advanceLevel();
    }
    Q_FOREACH (DefinitionPtr attDef, this->AllDefs)
    {
      if (attDef->isRelevant(!ignoreCategories, includeAdvanceLevel, advLevel))
      {
        defs.push_back(attDef);
      }
    }
    return defs;
  }

  QTableView* ListTable;
  QStandardItemModel* ListTableModel;
  QSortFilterProxyModel* ListTableProxyModel;
  qtTableWidget* ValuesTable;

  QComboBox* DefsCombo;
  QLabel* DefLabel;

  QPointer<QToolBar> TopToolBar;

  QAction* AddAction;
  QAction* DeleteAction;
  QAction* CopyAction;

  QFrame* ButtonsFrame;
  QFrame* TopFrame;           // top
  QPointer<QFrame> SearchBox; //Frame holding searching capability
  QPointer<qtAssociationWidget> AssociationsWidget;

  // <category, AttDefinitions>
  QMap<QString, QList<smtk::attribute::DefinitionPtr>> AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllDefs;

  // Attribute widget
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QFrame> AttFrame;

  //Last selected attribute
  WeakAttributePtr selectedAttribute;
  const std::string m_activeAttributeViewAttName = "ActiveAttribute";

  // Model for filtering the attribute by combobox.
  QPointer<QStandardItemModel> checkableAttComboModel;
  QMap<std::string, Qt::CheckState> AttSelections;
  qtCheckItemComboBox* SelectAttCombo;

  QMap<std::string, Qt::CheckState> AttProperties;
  std::vector<smtk::attribute::DefinitionPtr> m_attDefinitions;
  bool m_okToCreateModelEntities;
  smtk::model::BitFlags m_modelEntityMask;

  // Used to store attribute styles defined within the View as well as
  // style names explicitly assigned to an attribute type
  std::map<std::string, smtk::view::Configuration::Component> m_attCompMap;
  std::map<std::string, std::string> m_attStyleMap;

  QIcon m_alertIcon;
  QSize m_alertSize;
  bool m_showTopButtons;

  smtk::operation::Observers::Key m_observerKey;
};

qtBaseView* qtAttributeView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtAttributeView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtAttributeView::qtAttributeView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  m_internals = new qtAttributeViewInternals;
  m_internals->m_alertIcon = QIcon(this->uiManager()->alertPixmap());
  m_internals->m_alertSize = this->uiManager()->alertPixmap().size();
  smtk::view::ConfigurationPtr view = this->configuration();
  m_hideAssociations = false;
  m_allAssociatedMode = false;
  m_associationWidgetIsUsed = false;
  m_disableNameField = false;
  m_searchBoxVisibility = true;
  m_searchBoxText = "Search attributes...";

  if (view)
  {
    view->details().attributeAsBool("HideAssociations", m_hideAssociations);
    view->details().attributeAsBool("RequireAllAssociated", m_allAssociatedMode);
    view->details().attributeAsBool("DisableNameField", m_disableNameField);
    view->details().attributeAsBool("DisplaySearchBox", m_searchBoxVisibility);
    view->details().attribute("SearchBoxText", m_searchBoxText);
    view->details().attribute("AttributeNameRegex", m_attributeNameRegex);
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

const QMap<QString, QList<smtk::attribute::DefinitionPtr>>& qtAttributeView::attDefinitionMap()
  const
{
  return m_internals->AttDefMap;
}

smtk::extension::qtAssociationWidget* qtAttributeView::createAssociationWidget(
  QWidget* parent,
  qtBaseView* view)
{
  return new qtAssociation2ColumnWidget(parent, view);
}

void qtAttributeView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
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
  // Create a map for all categories so we can cluster the definitions
  m_internals->AttDefMap.clear();
  const ResourcePtr attResource = this->attributeResource();
  std::set<std::string>::const_iterator it;
  const std::set<std::string>& cats = attResource->categories();

  for (it = cats.begin(); it != cats.end(); it++)
  {
    QList<smtk::attribute::DefinitionPtr> attdeflist;
    m_internals->AttDefMap[it->c_str()] = attdeflist;
  }

  // Initialize definition info
  this->getAllDefinitions();
  if (m_internals->AllDefs.empty())
  {
    qWarning("WARNING: View \"%s\" has no AttributeTypes defined.", view->name().c_str());
    return;
  }

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation(Qt::Vertical);

  QFrame* TopFrame = new QFrame(frame);
  TopFrame->setObjectName(view->name().c_str());
  m_internals->AttFrame = new QFrame(frame);
  m_internals->AttFrame->setObjectName("attribute");

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
  m_internals->ListTable = new QTableView(frame);
  m_internals->ListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_internals->ListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_internals->ListTable->setSizePolicy(tableSizePolicy);
  m_internals->ListTable->setItemDelegateForColumn(
    status_column, new qtNotEditableDelegate(m_internals->ListTable));

  if (!m_attributeNameRegex.empty())
  {
    auto* nameDelegate = new qtRegexDelegate(m_internals->ListTable);
    nameDelegate->setExpression(m_attributeNameRegex);
    m_internals->ListTable->setItemDelegateForColumn(name_column, nameDelegate);
  }

  m_internals->ListTableProxyModel = new QSortFilterProxyModel(m_internals->ListTable);
  m_internals->ListTableModel = new QStandardItemModel(m_internals->ListTable);
  m_internals->ListTableProxyModel->setSourceModel(m_internals->ListTableModel);
  m_internals->ListTableProxyModel->setFilterKeyColumn(name_column);
  m_internals->ListTable->setModel(m_internals->ListTableProxyModel);

  // Buttons frame
  m_internals->ButtonsFrame = new QFrame(frame);
  m_internals->ButtonsFrame->setObjectName("buttons");
  QHBoxLayout* buttonLayout = new QHBoxLayout(m_internals->ButtonsFrame);
  buttonLayout->setMargin(0);
  m_internals->ButtonsFrame->setSizePolicy(sizeFixedPolicy);
  if (view)
  {
    m_internals->m_showTopButtons = !view->details().attributeAsBool("DisableTopButtons");
    m_internals->ButtonsFrame->setVisible(m_internals->m_showTopButtons);
  }

  m_internals->TopToolBar = new QToolBar(m_internals->ButtonsFrame);
  m_internals->TopToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
  m_internals->TopToolBar->setStyleSheet(R"(
    QToolButton {
      border: 1px solid palette(mid);
      border-radius: 3px;
      background-color: palette(button);
    }

    QToolButton:hover {
      border: 1px solid palette(mid);
      border-radius: 3px;
      background-color: palette(light);
    }

    QToolButton:pressed {
      border: 1px solid palette(mid);
      border-radius: 3px;
      background-color: palette(midlight);
    }
  )");

  m_internals->AddAction = new QAction("New");
  m_internals->DeleteAction = new QAction("Delete");
  m_internals->CopyAction = new QAction("Copy");

  m_internals->TopToolBar->addAction(m_internals->AddAction);

  m_internals->TopToolBar->addAction(m_internals->CopyAction);

  m_internals->TopToolBar->addAction(m_internals->DeleteAction);

  //If we have more than 1 def then create a combo box for selecting a def,
  // else just create a label
  if (m_internals->AllDefs.size() > 1)
  {
    m_internals->DefsCombo = new QComboBox(m_internals->ButtonsFrame);
    m_internals->DefsCombo->setVisible(false);
    buttonLayout->addWidget(m_internals->DefsCombo);
    m_internals->DefsCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
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

  //Add toolbar
  buttonLayout->addWidget(m_internals->TopToolBar);

  // Attribute table
  m_internals->ValuesTable = new qtTableWidget(frame);
  m_internals->ValuesTable->setSizePolicy(tableSizePolicy);

  m_internals->SearchBox = new QFrame(TopFrame);
  m_internals->SearchBox->setObjectName("searchBox");
  QHBoxLayout* searchLayout = new QHBoxLayout(m_internals->SearchBox);
  searchLayout->setMargin(0);

  QLineEdit* searchBar = new QLineEdit(m_internals->SearchBox);
  searchBar->setPlaceholderText(m_searchBoxText.c_str());
  searchBar->setClearButtonEnabled(true);

  QCheckBox* caseSensitivity = new QCheckBox("Ignore Case", m_internals->SearchBox);
  connect(caseSensitivity, &QCheckBox::stateChanged, [this](int val) {
    if (val)
    {
      this->m_internals->ListTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    }
    else
    {
      this->m_internals->ListTableProxyModel->setFilterCaseSensitivity(Qt::CaseSensitive);
    }
  });
  caseSensitivity->setChecked(true);

  searchLayout->addWidget(searchBar);
  searchLayout->addWidget(caseSensitivity);
  m_internals->SearchBox->setContentsMargins(0, 0, 0, 0);
  m_internals->SearchBox->setVisible(m_searchBoxVisibility);

  TopLayout->addWidget(m_internals->ButtonsFrame);
  TopLayout->addWidget(m_internals->SearchBox);
  TopLayout->addWidget(m_internals->ListTable);

  connect(
    searchBar,
    &QLineEdit::textEdited,
    m_internals->ListTableProxyModel,
    &QSortFilterProxyModel::setFilterFixedString);

  m_internals->ValuesTable->setVisible(false);

  // Attribte frame
  m_internals->AttFrame = new QFrame(frame);
  m_internals->AttFrame->setObjectName("attribute");
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
  connect(
    m_internals->AssociationsWidget,
    &qtAssociationWidget::attAssociationChanged,
    this,
    &qtAttributeView::associationsChanged);

  connect(m_internals->AssociationsWidget, SIGNAL(availableChanged()), this, SIGNAL(modified()));

  //TODO: Reconnect these if view's item model is ever replaced
  connect(
    m_internals->ListTable,
    &QAbstractItemView::clicked,
    this,
    &qtAttributeView::onListBoxClicked,
    Qt::QueuedConnection);
  connect(
    m_internals->ListTable->selectionModel(),
    &QItemSelectionModel::selectionChanged,
    this,
    [this](const QItemSelection&, const QItemSelection&) { this->onListBoxSelectionChanged(); },
    Qt::QueuedConnection);
  connect(
    m_internals->ListTableModel,
    &QStandardItemModel::itemChanged,
    this,
    &qtAttributeView::onAttributeNameChanged);

  // we need this so that the attribute name will also be changed
  // when a recorded test is play back, which is using setText
  // on the underline QLineEdit of the cell.
  connect(
    m_internals->ListTableModel,
    &QStandardItemModel::dataChanged,
    this,
    [this](const QModelIndex& topLeft, const QModelIndex&, const QVector<int>&) {
      auto* item = m_internals->ListTableModel->itemFromIndex(topLeft);
      this->onAttributeItemChanged(item);
    },
    Qt::QueuedConnection);

  connect(m_internals->AddAction, &QAction::triggered, this, &qtAttributeView::onCreateNew);
  connect(m_internals->CopyAction, &QAction::triggered, this, &qtAttributeView::onCopySelected);
  connect(m_internals->DeleteAction, &QAction::triggered, this, &qtAttributeView::onDeleteSelected);

  connect(
    m_internals->ValuesTable,
    &QTableWidget::itemChanged,
    this,
    &qtAttributeView::onAttributeValueChanged);
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
      [guardedObject](
        const smtk::operation::Operation& oper,
        smtk::operation::EventType event,
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

smtk::attribute::Attribute* qtAttributeView::getRawAttributeFromItem(const QStandardItem* item)
{
  return (item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : nullptr);
}

smtk::attribute::Attribute* qtAttributeView::getRawAttributeFromIndex(const QModelIndex& index)
{
  if (!index.isValid())
  {
    return nullptr;
  }

  QModelIndex attIndex = index.sibling(index.row(), name_column);
  smtk::attribute::Attribute* raw =
    static_cast<Attribute*>(attIndex.data(Qt::UserRole).value<void*>());

  return raw;
}

smtk::attribute::AttributePtr qtAttributeView::getAttributeFromItem(const QStandardItem* item)
{
  smtk::attribute::Attribute* raw = this->getRawAttributeFromItem(item);
  if (raw == nullptr)
  {
    return smtk::attribute::AttributePtr();
  }

  return raw->shared_from_this();
}

smtk::attribute::AttributePtr qtAttributeView::getAttributeFromIndex(const QModelIndex& index)
{
  smtk::attribute::Attribute* raw = this->getRawAttributeFromIndex(index);
  return (raw ? raw->shared_from_this() : smtk::attribute::AttributePtr());
}

QStandardItem* qtAttributeView::getItemFromAttribute(smtk::attribute::Attribute* attribute)
{
  int n = m_internals->ListTableModel->rowCount();
  for (int i = 0; i < n; i++)
  {
    QStandardItem* item = m_internals->ListTableModel->item(i, name_column);
    if (this->getRawAttributeFromItem(item) == attribute)
    {
      return item;
    }
  }
  return nullptr;
}

// The selected item will always refer to the item that has the attribute data assigned to
// it
QStandardItem* qtAttributeView::getSelectedItem()
{
  QStandardItem* item = nullptr;

  //Get the index that corresponds to the item that corresponds to the selected attribute
  // Note that this is related to the proxy model
  QModelIndex currentIndex = m_internals->ListTable->currentIndex();
  QModelIndex selectedIndex = currentIndex.sibling(currentIndex.row(), name_column);

  //Trust nobody
  if (selectedIndex.isValid())
  {
    //Map back to source model index
    QModelIndex srcIndex = m_internals->ListTableProxyModel->mapToSource(selectedIndex);

    if (srcIndex.isValid())
    {
      item = m_internals->ListTableModel->itemFromIndex(srcIndex);
    }
  }

  //Don't forget nullptr checking on the other end
  return item;
}

smtk::attribute::AttributePtr qtAttributeView::getSelectedAttribute()
{
  // Using the index does not require mapping between the ProxyModel and
  // the underlying model (in contrast with getting the actual item)
  QModelIndex selectedIndex = m_internals->ListTable->currentIndex();
  return this->getAttributeFromIndex(selectedIndex);
}

void qtAttributeView::updateAssociationEnableState(smtk::attribute::AttributePtr theAtt)
{
  m_associationWidgetIsUsed = false;

  if (!this->hideAssociations())
  {
    if (theAtt)
    {
      m_associationWidgetIsUsed = (theAtt->definition()->associationRule() != nullptr);
      if (m_associationWidgetIsUsed)
      {
        m_internals->AssociationsWidget->showEntityAssociation(theAtt);
      }
    }
    else if (this->requireAllAssociated() && !m_internals->AllDefs.empty())
    {
      m_associationWidgetIsUsed = true;
      m_internals->AssociationsWidget->showEntityAssociation(m_internals->AllDefs.at(0));
    }
  }
  m_internals->AssociationsWidget->setVisible(m_associationWidgetIsUsed);
}

void qtAttributeView::onListBoxSelectionChanged()
{
  // Has the selected attribute been changed?
  smtk::attribute::AttributePtr dataItem = this->getSelectedAttribute();
  if (dataItem == m_internals->selectedAttribute.lock())
  {
    return; // The selection has not changed
  }

  m_internals->ValuesTable->blockSignals(true);
  m_internals->ValuesTable->clear();
  m_internals->ValuesTable->setRowCount(0);
  m_internals->ValuesTable->setColumnCount(0);

  if (dataItem)
  {
    this->configuration()->details().setAttribute(
      m_internals->m_activeAttributeViewAttName, dataItem->id().toString());
    this->updateAssociationEnableState(dataItem);
    this->updateTableWithAttribute(dataItem);
    Q_EMIT attributeSelected(dataItem);
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

void qtAttributeView::onAttributeNameChanged(QStandardItem* item)
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

void qtAttributeView::onAttributeItemChanged(QStandardItem* item)
{
  if (item == nullptr)
  {
    return;
  }

  if (item->index().column() == name_column)
  {
    this->onAttributeNameChanged(item);
  }
}

void qtAttributeView::insertTableColumn(
  QTableWidget* vtWidget,
  int insertCol,
  const QString& title,
  int advancedlevel)
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
  smtk::attribute::ItemPtr attItem,
  QTableWidgetItem* item)
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
  smtk::attribute::ItemPtr inData,
  int& startRow,
  bool enabled)
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

  Q_FOREACH (
    attribute::DefinitionPtr attDef,
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
  QStandardItem* item = this->addAttributeListItem(newAtt);
  if (item)
  {
    // Select the newly created attribute
    m_internals->ListTable->selectRow(item->row());
    QModelIndex index = item->index().sibling(item->row(), name_column);
    QModelIndex mappedIndex = m_internals->ListTableProxyModel->mapFromSource(index);
    m_internals->ListTable->setCurrentIndex(mappedIndex);
    //Automatically trigger name edit if allowed
    if (!this->attributeNamesConstant())
    {
      m_internals->ListTable->edit(mappedIndex);
    }
  }
  this->attributeCreated(newAtt);
  Q_EMIT this->numOfAttributesChanged();
  Q_EMIT qtBaseView::modified();
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
    QStandardItem* item = this->addAttributeListItem(newObject);
    if (item)
    {
      m_internals->ListTable->selectRow(item->row());
    }
    Q_EMIT this->numOfAttributesChanged();
    Q_EMIT qtBaseView::modified();
  }
}

void qtAttributeView::onDeleteSelected()
{
  smtk::attribute::AttributePtr selObject = this->getSelectedAttribute();
  if (selObject == nullptr)
  {
    return; // nothing to be deleted
  }

  if (selObject != nullptr)
  {
    if (this->deleteAttribute(selObject))
    {
      std::string keyName = selObject->name();
      m_internals->AttSelections.remove(keyName);

      QStandardItem* selItem = this->getSelectedItem();
      if (selItem != nullptr)
      {
        m_internals->ListTableModel->removeRow(selItem->row());
        this->attributeRemoved(selObject);
        Q_EMIT this->numOfAttributesChanged();
        Q_EMIT qtBaseView::modified();
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

QStandardItem* qtAttributeView::addAttributeListItem(smtk::attribute::AttributePtr childData)
{
  QStandardItem* item = new QStandardItem(QString::fromUtf8(childData->name().c_str()));
  Qt::ItemFlags nonEditableFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  //Can the attribute name be changed?
  bool nameIsConstant = this->attributeNamesConstant() ||
    (childData->properties().contains<bool>("smtk.extensions.attribute_view.name_read_only") &&
     childData->properties().at<bool>("smtk.extensions.attribute_view.name_read_only"));

  Qt::ItemFlags itemFlags(nonEditableFlags | (nameIsConstant ? 0x0 : Qt::ItemIsEditable));
  QVariant vdata;
  vdata.setValue(static_cast<void*>(childData.get()));
  item->setData(vdata, Qt::UserRole);
  item->setFlags(itemFlags);

  int numRows = m_internals->ListTableModel->rowCount();
  m_internals->ListTableModel->setRowCount(++numRows);
  m_internals->ListTableModel->setItem(numRows - 1, name_column, item);

  // add the type column too.
  std::string txtDef = childData->definition()->displayedTypeName();

  QStandardItem* defitem = new QStandardItem(QString::fromUtf8(txtDef.c_str()));
  defitem->setFlags(nonEditableFlags);
  m_internals->ListTableModel->setItem(numRows - 1, type_column, defitem);

  // ToDo: Reactivate Color Option when we are ready to use it
  // Lets see if we need to show an alert icon cause the attribute is not
  // valid
  if (!childData->isValid())
  {
    QStandardItem* statusItem = new QStandardItem(m_internals->m_alertIcon, "");
    statusItem->setSizeHint(m_internals->m_alertSize);
    m_internals->ListTableModel->setItem(numRows - 1, status_column, statusItem);
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

  m_internals->AddAction->setEnabled(currentDefs.count() > 0);

  m_internals->ButtonsFrame->setVisible(m_internals->m_showTopButtons);
  m_internals->ListTable->setVisible(true);
  m_internals->ListTable->blockSignals(true);
  m_internals->ListTableModel->removeRows(0, m_internals->ListTableModel->rowCount());

  // ToDo: Reactivate Color Option when we are ready to use it
  //int numCols = viewAtt ? 4 : 3;
  int numCols = 3;
  m_internals->ListTableModel->setColumnCount(numCols);
  m_internals->ListTableModel->setHorizontalHeaderItem(status_column, new QStandardItem(""));
  m_internals->ListTableModel->setHorizontalHeaderItem(name_column, new QStandardItem("Name"));
  // Lets set up the column behavior
  // The Type and Status Columns should be size to fit their contents while
  // the Name field should stretch to take up the space
  m_internals->ListTableModel->setHorizontalHeaderItem(type_column, new QStandardItem("Type"));
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
    m_internals->ListTableModel->setHorizontalHeaderItem(color_column, new QStandardItem("Color"));
  }
  if (m_internals->AllDefs.size() == 1)
  {
    m_internals->DefLabel->setVisible(true);
  }
  else
  {
    m_internals->DefsCombo->clear();
    Q_FOREACH (attribute::DefinitionPtr attDef, currentDefs)
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
  Q_FOREACH (attribute::DefinitionPtr attDef, currentDefs)
  {
    this->onViewByWithDefinition(attDef);
  }

  m_internals->ListTable->blockSignals(false);

  QSplitter* frame = qobject_cast<QSplitter*>(this->Widget);
  if (m_internals->ListTableModel->rowCount() && !this->getSelectedItem())
  {
    // so switch tabs would not reset selection
    // get the active tab from the view config if it exists
    std::string activeAttUuid;
    this->configuration()->details().attribute(
      m_internals->m_activeAttributeViewAttName, activeAttUuid);
    smtk::attribute::ConstAttributePtr activeAtt =
      this->attributeResource()->findAttribute(smtk::common::UUID(activeAttUuid));

    if (activeAtt)
    {
      auto items = this->m_internals->ListTableModel->findItems(
        activeAtt->name().c_str(), Qt::MatchExactly, name_column);
      if (!items.empty())
      {
        auto* activeItem = items.at(0);
        // Select the newly created attribute
        m_internals->ListTable->selectRow(activeItem->row());
        QModelIndex index = activeItem->index();
        QModelIndex mappedIndex = m_internals->ListTableProxyModel->mapFromSource(index);
        m_internals->ListTable->setCurrentIndex(mappedIndex);
      }
    }
    else if (!m_internals->AssociationsWidget->hasSelectedItem())
    {
      // In this case there was no previous selection so lets select the
      // first attribute
      QModelIndex mi = m_internals->ListTableProxyModel->index(0, name_column);
      if (mi.isValid())
      {
        m_internals->ListTable->setCurrentIndex(mi);
      }
    }
  }
  else
  {
    m_internals->selectedAttribute.reset();
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
      QStandardItem* item = this->addAttributeListItem(*it);
      if ((*it)->definition()->advanceLevel())
      {
        item->setFont(this->uiManager()->advancedFont());
      }
      if (currentAtt == (*it))
      {
        auto index = m_internals->ListTableProxyModel->mapFromSource(item->index());
        m_internals->ListTable->setCurrentIndex(index);
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
  smtk::attribute::DefinitionPtr def = att->definition();
  // Lets get a style for this attribute
  const auto& style = this->findStyle(def);
  m_internals->CurrentAtt = new qtAttribute(att, style, m_internals->AttFrame, this);
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
      connect(
        m_internals->CurrentAtt,
        &qtAttribute::itemModified,
        this,
        &qtAttributeView::onItemChanged,
        Qt::QueuedConnection);
      if (this->advanceLevelVisible())
      {
        m_internals->CurrentAtt->showAdvanceLevelOverlay(true);
      }
    }
    m_internals->DeleteAction->setEnabled(
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
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }

  smtk::attribute::ResourcePtr resource = this->attributeResource();

  std::string attName, defName, val, styleName;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;
  bool flag;

  // The view should have a single internal component called InstancedAttributes
  if (
    (view->details().numberOfChildren() != 1) ||
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

    //Lets see if there is an explicit style name
    if (attsComp.child(i).attribute("Style", styleName))
    {
      m_internals->m_attStyleMap[defName] = styleName;
    }

    m_internals->m_attCompMap[defName] = attsComp.child(i);
    this->qtBaseAttributeView::getDefinitions(attDef, m_internals->AllDefs);
    m_internals->m_attDefinitions.push_back(attDef);
  }

  // sort the list
  std::sort(
    std::begin(m_internals->AllDefs),
    std::end(m_internals->AllDefs),
    [](smtk::attribute::DefinitionPtr a, smtk::attribute::DefinitionPtr b) {
      return a->displayedTypeName() < b->displayedTypeName();
    });

  Q_FOREACH (smtk::attribute::DefinitionPtr adef, m_internals->AllDefs)
  {
    Q_FOREACH (QString category, m_internals->AttDefMap.keys())
    {
      if (
        adef->categories().passes(category.toStdString()) &&
        !m_internals->AttDefMap[category].contains(adef))
      {
        m_internals->AttDefMap[category].push_back(adef);
      }
    }
  }
}

void qtAttributeView::onListBoxClicked(const QModelIndex& index)
{
  auto mappedIndex = m_internals->ListTableProxyModel->mapToSource(index);

  QStandardItem* item = m_internals->ListTableModel->itemFromIndex(mappedIndex);

  if (item == nullptr)
  {
    return;
  }

  bool isColor = item->column() == color_column;
  if (isColor)
  {
    QStandardItem* selItem = m_internals->ListTableModel->item(item->row(), 0);
    smtk::attribute::AttributePtr selAtt = this->getAttributeFromItem(selItem);
    QBrush bgBrush = item->background();
    QColor color = QColorDialog::getColor(
      bgBrush.color(), this->Widget, "Choose Attribute Color", QColorDialog::DontUseNativeDialog);
    if (color.isValid() && color != bgBrush.color() && selAtt)
    {
      bgBrush.setColor(color);
      item->setBackground(bgBrush);
      selAtt->setColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
      Q_EMIT this->attColorChanged();
    }
    auto mappedCurrent =
      m_internals->ListTableProxyModel->mapToSource(m_internals->ListTable->currentIndex());
    if (mappedCurrent != selItem->index())
    {
      mappedCurrent = m_internals->ListTableProxyModel->mapFromSource(selItem->index());
      m_internals->ListTable->setCurrentIndex(mappedCurrent);
      m_internals->ListTable->selectRow(selItem->row());
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
  Q_EMIT this->modified(m_internals->CurrentAtt->attribute()->associations());
  Q_EMIT this->attAssociationChanged();
  Q_EMIT qtBaseView::modified();
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
  for (int i = 0; i < m_internals->ListTableModel->rowCount(); i++)
  {
    QStandardItem* item = m_internals->ListTableModel->item(i, name_column);
    Attribute* listAtt =
      item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : nullptr;
    if (listAtt == nullptr)
    {
      continue;
    }
    if (listAtt == att)
    {
      if (att->isValid())
      {
        m_internals->ListTableModel->setItem(i, status_column, nullptr);
      }
      else
      {
        QStandardItem* statusItem = new QStandardItem(m_internals->m_alertIcon, "");
        statusItem->setSizeHint(m_internals->m_alertSize);
        m_internals->ListTableModel->setItem(i, status_column, statusItem);
      }
      break;
    }
  }
}

bool qtAttributeView::matchesDefinitions(const smtk::attribute::DefinitionPtr& def) const
{
  return std::any_of(
    m_internals->m_attDefinitions.begin(),
    m_internals->m_attDefinitions.end(),
    [=](const smtk::attribute::DefinitionPtr& viewDef) { return def->isA(viewDef); });
}

int qtAttributeView::handleOperationEvent(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  if (event != smtk::operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  // Since the Signal Operation originates from a Qt Signal
  // being fired we need to see if this view is one that triggered it
  if (
    (op.typeName() == smtk::common::typeName<smtk::attribute::Signal>()) &&
    (op.parameters()->findString("source")->value() == m_addressString))
  {
    // We can ignore this operation since we initiated it
    return 0;
  }

  //In the case of modified components we only need to look at the
  // current attribute being displayed
  smtk::attribute::ComponentItemPtr compItem;
  std::size_t i, n;
  smtk::attribute::DefinitionPtr currentDef = this->getCurrentDef();

  // If there is no definition or it's attribute resource is mark for removal
  // then we don't need to update anything
  if ((currentDef == nullptr) || currentDef->resource()->isMarkedForRemoval())
  {
    return 0;
  }

  compItem = result->findComponent("modified");
  n = compItem->numberOfValues();
  for (i = 0; i < n; i++)
  {
    // We need to make sure the attribute's name and name edit-ability are properly set
    if (!compItem->isSet(i))
    {
      continue;
    }

    auto att = dynamic_pointer_cast<smtk::attribute::Attribute>(compItem->value(i));
    // If there is no attribute or it's definition is not being displayed in the View - skip it
    if (!(att && this->matchesDefinitions(att->definition())))
    {
      continue;
    }
    // Is this the current attribute being displayed?
    if (m_internals->CurrentAtt && (att == m_internals->CurrentAtt->attribute()))
    {
      // Update the attribute's items
      auto items = m_internals->CurrentAtt->items();
      for (auto* item : items)
      {
        item->updateItemData();
      }
    }
    // Need to update the item's name and edit ability
    auto* item = this->getItemFromAttribute(att.get());
    if (item)
    {
      item->setText(QString::fromUtf8(att->name().c_str()));
      if (!this->attributeNamesConstant())
      {
        // Need to see if the name is editable
        if (
          att->properties().contains<bool>("smtk.extensions.attribute_view.name_read_only") &&
          att->properties().at<bool>("smtk.extensions.attribute_view.name_read_only"))
        {
          Qt::ItemFlags itemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
          item->setFlags(itemFlags);
        }
        else
        {
          Qt::ItemFlags itemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
          item->setFlags(itemFlags);
        }
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
      if (!compItem->isSet(i))
      {
        continue;
      }

      auto att = dynamic_pointer_cast<smtk::attribute::Attribute>(compItem->value(i));
      // If there is no attribute or it's definition is not being displayed in the View - skip it
      if (!(att && this->matchesDefinitions(att->definition())))
      {
        continue;
      }
      int row, numRows = m_internals->ListTableModel->rowCount();
      for (row = 0; row < numRows; ++row)
      {
        QStandardItem* item = m_internals->ListTableModel->item(row, name_column);
        smtk::attribute::Attribute* itemAtt = this->getRawAttributeFromItem(item);
        if (att.get() == itemAtt)
        {
          m_internals->ListTableModel->removeRow(row);
          break;
        }
      }
    }
  }

  compItem = result->findComponent("created");
  n = compItem->numberOfValues();
  for (i = 0; i < n; i++)
  {
    if (compItem->isSet(i))
    {
      auto att = dynamic_pointer_cast<smtk::attribute::Attribute>(compItem->value(i));
      // If there is no attribute or it's definition is not being displayed in the View - skip it
      if (!(att && this->matchesDefinitions(att->definition())))
      {
        continue;
      }
      this->addAttributeListItem(att);
    }
  }
  return 0;
}

bool qtAttributeView::isValid() const
{
  for (int i = 0; i < m_internals->ListTableModel->rowCount(); i++)
  {
    QStandardItem* item = m_internals->ListTableModel->item(i, status_column);
    // If there is an alert icon in the status column then we know the attribute is invalid
    if (!((item == nullptr) || item->icon().isNull()))
    {
      return false;
    }
  }
  if ((m_internals->AssociationsWidget != nullptr) && m_associationWidgetIsUsed)
  {
    return m_internals->AssociationsWidget->isValid();
  }
  return true;
}

QToolBar* smtk::extension::qtAttributeView::toolBar()
{
  return m_internals->TopToolBar;
}

QAction* smtk::extension::qtAttributeView::addAction()
{
  return m_internals->AddAction;
}

QAction* smtk::extension::qtAttributeView::copyAction()
{
  return m_internals->CopyAction;
}

QAction* smtk::extension::qtAttributeView::deleteAction()
{
  return m_internals->DeleteAction;
}

void smtk::extension::qtAttributeView::setTableItemDelegate(QAbstractItemDelegate* delegate)
{
  m_internals->ListTable->setItemDelegate(delegate);
}

void smtk::extension::qtAttributeView::setTableColumnItemDelegate(
  int column,
  QAbstractItemDelegate* delegate)
{
  m_internals->ListTable->setItemDelegateForColumn(column, delegate);
}

void smtk::extension::qtAttributeView::setTableRowItemDelegate(
  int row,
  QAbstractItemDelegate* delegate)
{
  m_internals->ListTable->setItemDelegateForRow(row, delegate);
}

void smtk::extension::qtAttributeView::triggerEdit(const QModelIndex& index)
{
  if (index.column() == name_column)
  {
    m_internals->ListTable->edit(index);
  }
}

int smtk::extension::qtAttributeView::numOfAttributes()
{
  return m_internals->ListTableModel->rowCount();
}

const smtk::view::Configuration::Component& smtk::extension::qtAttributeView::findStyle(
  const smtk::attribute::DefinitionPtr& def,
  bool isOriginalDef)
{
  // This is the order of preference in trying to find a style:
  // 1. Is there an explicit style store in the View for this Definition?
  // 2. Else, is there  a style name associated with the definition?
  // 3. Lets Check the above conditions for the definition's base
  // 4. Finally lets as the UI Manager for a default global style (if this is the original definition)
  static smtk::view::Configuration::Component emptyStyle;

  auto it = m_internals->m_attCompMap.find(def->type());
  // Did we find a component that contained style information
  if ((it != m_internals->m_attCompMap.end()) && it->second.numberOfChildren())
  {
    return it->second;
  }
  // Was there a global style associated with it?
  auto it1 = m_internals->m_attStyleMap.find(def->type());
  if (it1 != m_internals->m_attStyleMap.end())
  {
    return this->uiManager()->findStyle(def, it1->second);
  }
  // Are there more definitions for us to check?
  if (def->baseDefinition())
  {
    const auto& style = this->findStyle(def->baseDefinition(), false);
    // Did we get an empty style?
    if (style.numberOfChildren())
    {
      return style;
    }
  }
  // Is this the original definition?
  if (isOriginalDef)
  {
    return this->uiManager()->findStyle(def);
  }
  return emptyStyle;
}
