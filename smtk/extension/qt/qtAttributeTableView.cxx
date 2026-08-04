//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeTableView.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeTableDelegate.h"
#include "smtk/extension/qt/qtAttributeTableModel.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/task/Manager.h"

#include "smtk/view/Selection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Information.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace extension
{

namespace
{

bool containsGroupItem(const smtk::attribute::ItemDefinitionPtr& definition)
{
  if (!definition)
  {
    return false;
  }

  if (std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(definition))
  {
    return true;
  }

  const auto valueDefinition =
    std::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(definition);
  if (!valueDefinition)
  {
    return false;
  }

  for (const auto& child : valueDefinition->childrenItemDefinitions())
  {
    if (containsGroupItem(child.second))
    {
      return true;
    }
  }
  return false;
}

bool containsGroupItem(const smtk::attribute::DefinitionPtr& definition)
{
  if (!definition)
  {
    return false;
  }

  for (std::size_t i = 0; i < definition->numberOfItemDefinitions(); ++i)
  {
    if (containsGroupItem(definition->itemDefinition(static_cast<int>(i))))
    {
      return true;
    }
  }
  return false;
}

} // anonymous namespace

/**
 * Private Qt implementation owned by qtAttributeTableView.
 *
 * This widget will allow the user to tab from the last cell
 * in a row to the first cell in the next row
 */
class qtAttributeTableWidget : public QTableView
{

public:
  using QTableView::QTableView;

protected:
  QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override
  {
    if (cursorAction != QAbstractItemView::MoveNext)
    {
      return QTableView::moveCursor(cursorAction, modifiers);
    }

    QModelIndex current = this->currentIndex();

    if (!current.isValid())
    {
      return QTableView::moveCursor(cursorAction, modifiers);
    }

    const int row = current.row();
    const int column = current.column();

    /*
     * Find the next editable cell in the current row.
     */
    for (int c = column + 1; c < this->model()->columnCount(); ++c)
    {
      if (this->isColumnHidden(c))
      {
        continue;
      }

      QModelIndex next = this->model()->index(row, c);

      if (next.flags() & Qt::ItemIsEditable)
      {
        return next;
      }
    }

    /*
     * No more editable cells exist in this row. Advance to the first
     * editable item cell of the next row.
     */
    for (int r = row + 1; r < this->model()->rowCount(); ++r)
    {
      for (int c = 1; c < this->model()->columnCount(); ++c)
      {
        if (this->isColumnHidden(c))
        {
          continue;
        }

        QModelIndex next = this->model()->index(r, c);

        if (next.flags() & Qt::ItemIsEditable)
        {
          return next;
        }
      }
    }

    /*
     * Fall back to Qt's default behavior if there are no remaining
     * editable cells.
     */
    return QTableView::moveCursor(cursorAction, modifiers);
  }
};

/**
 * Private Qt implementation owned by qtAttributeTableView.
 *
 * QPointer is used so these members are cleared automatically if Qt deletes
 * the corresponding QObject through parent-child ownership.
 */
class qtAttributeTableView::Internal
{
public:
  ~Internal() { delete AttributeEditor; }

  /// Top-level widget inserted into the SMTK view hierarchy.
  QPointer<QWidget> Widget;

  /// Table that displays the attributes.
  QPointer<QTableView> Table;

  /// Vertically divides the table and selected-attribute editor.
  QPointer<QSplitter> TableEditorSplitter;

  /// Source model that maps SMTK attributes to table rows.
  QPointer<qtAttributeTableModel> Model;

  /// Proxy model that provides sorting and filtering.
  QPointer<QSortFilterProxyModel> Proxy;

  /// Creates a new attribute based on the configured definition.
  QPointer<QPushButton> AddButton;

  /// Delegate responsible for constructing item-specific table editors.
  QPointer<qtAttributeTableDelegate> Delegate;

  /// Deletes the attributes represented by the selected rows.
  QPointer<QPushButton> DeleteButton;

  /// Hide conditional-child columns that are inactive for the current row.
  bool HideInactiveChildren{ false };

  /// Container for editing group-bearing attributes below the table.
  QPointer<QGroupBox> AttributeEditorFrame;

  /// Scrollable viewport for the selected-attribute editor.
  QPointer<QScrollArea> AttributeEditorScrollArea;

  /// Standard SMTK editor for the current group-bearing attribute.
  qtAttribute* AttributeEditor{ nullptr };

  /// Prevent a detail edit's model refresh from rebuilding its signal sender.
  bool UpdatingFromAttributeEditor{ false };

  /// Per-definition inline attribute-editor configuration from AttributeTypes.
  std::map<std::string, smtk::view::Configuration::Component> AttributeComponents;

  /// Per-definition named styles from AttributeTypes.
  std::map<std::string, std::string> AttributeStyles;

  /// Policy controlling which item columns are included in the model.
  qtAttributeTableModel::ColumnDisplay ColumnDisplay{ qtAttributeTableModel::ColumnDisplay::All };

  /// Exact item paths included by the UserSpecified column-display policy.
  std::set<std::string> ColumnItemPaths;

  /// Logical columns backed by mutually-exclusive conditional children.
  std::vector<qtAttributeTableModel::SharedColumn> SharedColumns;
};

qtBaseView* qtAttributeTableView::createViewWidget(const smtk::view::Information& info)
{
  auto* view = new qtAttributeTableView(info);
  view->buildUI();
  return view;
}

qtAttributeTableView::qtAttributeTableView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
  , m_internals(new Internal)
{
  this->enableOperationObserver();
}

qtAttributeTableView::~qtAttributeTableView()
{
  // Qt owns the widgets through parent-child ownership. Only the internal
  // non-QObject structure must be deleted explicitly.
  delete m_internals;
}

qtAttributeTableModel* qtAttributeTableView::tableModel() const
{
  return m_internals->Model;
}

QTableView* qtAttributeTableView::tableView() const
{
  return m_internals->Table;
}

void qtAttributeTableView::createWidget()
{
  QWidget* parentWidget = this->parentWidget();

  m_internals->Widget = new QWidget(parentWidget);

  auto* mainLayout = new QVBoxLayout(m_internals->Widget);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  /*
   * Place attribute-management buttons above the table. Keeping the controls
   * outside the table avoids introducing special rows or columns into the
   * attribute model.
   */
  auto* buttonLayout = new QHBoxLayout;

  m_internals->AddButton = new QPushButton(tr("➕"), m_internals->Widget);

  m_internals->AddButton->setToolTip(tr("Create an attribute using the configured definition."));

  m_internals->DeleteButton = new QPushButton(tr("✘"), m_internals->Widget);

  m_internals->DeleteButton->setToolTip(
    tr("Delete the attributes represented by the selected rows."));

  buttonLayout->addWidget(m_internals->AddButton);
  buttonLayout->addWidget(m_internals->DeleteButton);
  buttonLayout->addStretch();

  mainLayout->addLayout(buttonLayout);

  /* Properly set the add/delete buttons visibilities */
  bool disableAdd = false;
  bool disableDelete = false;

  const auto& details = this->configuration()->details();

  details.attributeAsBool("DisableAddAttribute", disableAdd);

  details.attributeAsBool("DisableDeleteAttribute", disableDelete);
  details.attributeAsBool("HideInactiveChildren", m_internals->HideInactiveChildren);

  std::string columnDisplay;
  if (details.attribute("ColumnDisplay", columnDisplay))
  {
    std::transform(
      columnDisplay.begin(),
      columnDisplay.end(),
      columnDisplay.begin(),
      [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    if (columnDisplay == "toplevelnongroup" || columnDisplay == "toplevelnongroupitems")
    {
      m_internals->ColumnDisplay = qtAttributeTableModel::ColumnDisplay::TopLevelNonGroup;
    }
    else if (columnDisplay == "topleveldiscrete" || columnDisplay == "topleveldiscreteitems")
    {
      m_internals->ColumnDisplay = qtAttributeTableModel::ColumnDisplay::TopLevelDiscrete;
    }
    else if (columnDisplay == "userspecified")
    {
      m_internals->ColumnDisplay = qtAttributeTableModel::ColumnDisplay::UserSpecified;
    }
  }

  const int tableItemsIndex = details.findChild("TableItems");
  if (tableItemsIndex != -1)
  {
    const auto& tableItems = details.child(tableItemsIndex);
    for (std::size_t i = 0; i < tableItems.numberOfChildren(); ++i)
    {
      const auto& itemComponent = tableItems.child(i);
      if (itemComponent.name() == "Item")
      {
        std::string itemPath;
        if (
          (itemComponent.attribute("Path", itemPath) ||
           itemComponent.attribute("Name", itemPath)) &&
          !itemPath.empty())
        {
          m_internals->ColumnItemPaths.insert(itemPath);
        }
        continue;
      }

      if (itemComponent.name() != "Column")
      {
        continue;
      }

      qtAttributeTableModel::SharedColumn sharedColumn;
      itemComponent.attribute("Name", sharedColumn.Name);
      if (!itemComponent.attribute("Label", sharedColumn.Label))
      {
        sharedColumn.Label = sharedColumn.Name;
      }

      for (std::size_t j = 0; j < itemComponent.numberOfChildren(); ++j)
      {
        const auto& candidate = itemComponent.child(j);
        if (candidate.name() != "Item")
        {
          continue;
        }

        std::string itemPath;
        if (
          (candidate.attribute("Path", itemPath) || candidate.attribute("Name", itemPath)) &&
          !itemPath.empty())
        {
          sharedColumn.ItemPaths.push_back(itemPath);
        }
      }

      // The attribute utility reports a detailed warning later if the list is
      // incomplete or the candidate activation rules are invalid.
      if (!sharedColumn.ItemPaths.empty())
      {
        m_internals->SharedColumns.push_back(std::move(sharedColumn));
      }
    }
  }

  /*
   * Preserve each Att component, rather than only its Type, so the selected
   * attribute editor can honor ItemViews and named styles just as
   * qtAttributeView does.
   */
  const int attributeTypesIndex = details.findChild("AttributeTypes");
  if (attributeTypesIndex != -1)
  {
    const auto& attributeTypes = details.child(attributeTypesIndex);
    for (std::size_t i = 0; i < attributeTypes.numberOfChildren(); ++i)
    {
      const auto& attributeComponent = attributeTypes.child(i);
      if (attributeComponent.name() != "Att")
      {
        continue;
      }

      std::string definitionName;
      if (!attributeComponent.attribute("Type", definitionName))
      {
        continue;
      }

      m_internals->AttributeComponents[definitionName] = attributeComponent;

      std::string styleName;
      if (attributeComponent.attribute("Style", styleName))
      {
        m_internals->AttributeStyles[definitionName] = styleName;
      }
    }
  }

  m_internals->AddButton->setVisible(!disableAdd);
  m_internals->DeleteButton->setVisible(!disableDelete);

  /*
   * Create the table and its source model.
   *
   * The model accesses SMTK attribute values directly; it does not maintain a
   * separate copy of the attribute data.
   */
  m_internals->Table = new qtAttributeTableWidget(m_internals->Widget);
  m_internals->Table->setTabKeyNavigation(true);
  m_internals->Model = new qtAttributeTableModel(m_internals->Table);
  m_internals->Model->setUIManager(this->uiManager());
  m_internals->Model->setColumnDisplay(
    m_internals->ColumnDisplay, m_internals->ColumnItemPaths, m_internals->SharedColumns);

  /*
   * Insert a proxy model between the table and source model so sorting and
   * filtering can be handled without changing the source-model row ordering.
   */
  m_internals->Proxy = new QSortFilterProxyModel(m_internals->Table);

  m_internals->Proxy->setSourceModel(m_internals->Model);
  m_internals->Proxy->setDynamicSortFilter(true);
  m_internals->Proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

  m_internals->Table->setModel(m_internals->Proxy);
  m_internals->Table->setSortingEnabled(true);
  m_internals->Table->setAlternatingRowColors(true);

  /*
 * The delegate selects an editor based on the SMTK item represented by each
 * table cell:
 *
 * - Discrete ValueItem   -> QComboBox
 * - IntItemDefinition    -> QLineEdit + QIntValidator
 * - DoubleItemDefinition -> QLineEdit + QDoubleValidator
 * - StringItemDefinition -> default Qt editor
 */

  m_internals->Delegate = new qtAttributeTableDelegate(m_internals->Model, m_internals->Table);

  m_internals->Table->setItemDelegate(m_internals->Delegate);

  /*
   * A row represents a complete SMTK attribute. Selecting rows therefore
   * provides the most natural semantics for attribute deletion.
   */
  m_internals->Table->setSelectionBehavior(QAbstractItemView::SelectRows);

  m_internals->Table->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_internals->Table->setEditTriggers(
    QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed |
    QAbstractItemView::SelectedClicked);

  m_internals->Table->horizontalHeader()->setStretchLastSection(true);

  m_internals->Table->verticalHeader()->setVisible(false);

  m_internals->TableEditorSplitter = new QSplitter(Qt::Vertical, m_internals->Widget);
  m_internals->TableEditorSplitter->setObjectName(QStringLiteral("TableEditorSplitter"));
  m_internals->TableEditorSplitter->setChildrenCollapsible(false);
  m_internals->TableEditorSplitter->addWidget(m_internals->Table);
  mainLayout->addWidget(m_internals->TableEditorSplitter);

  /*
   * Group items cannot be represented faithfully in a single table cell.
   * Provide the standard SMTK attribute editor below the table when the
   * current attribute contains a top-level or conditional group item.
   */
  m_internals->AttributeEditorScrollArea = new QScrollArea(m_internals->TableEditorSplitter);
  m_internals->AttributeEditorScrollArea->setObjectName(
    QStringLiteral("AttributeEditorScrollArea"));
  m_internals->AttributeEditorScrollArea->setWidgetResizable(true);
  m_internals->AttributeEditorScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_internals->AttributeEditorScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  m_internals->AttributeEditorFrame =
    new QGroupBox(tr("Selected Attribute"), m_internals->AttributeEditorScrollArea);
  auto* attributeEditorLayout = new QVBoxLayout(m_internals->AttributeEditorFrame);
  attributeEditorLayout->setContentsMargins(0, 0, 0, 0);
  /*
   * Let this spacer consume excess splitter height. Without it, the layout
   * gives the extra height to the qtAttribute widget and its item rows spread
   * apart.
   */
  attributeEditorLayout->addStretch(1);
  m_internals->AttributeEditorScrollArea->setWidget(m_internals->AttributeEditorFrame);
  m_internals->AttributeEditorScrollArea->setVisible(false);
  m_internals->TableEditorSplitter->addWidget(m_internals->AttributeEditorScrollArea);
  m_internals->TableEditorSplitter->setStretchFactor(0, 1);
  m_internals->TableEditorSplitter->setStretchFactor(1, 1);

  /*
   * Register the top-level widget with qtBaseView.
   *
   * Some SMTK versions expose Widget directly, while others provide a
   * setWidget() method.
   */
  this->Widget = m_internals->Widget;

  /*
   * Notify the view after the table model commits an item edit.
   */
  m_internals->Model->setAttributeModifiedCallback(
    [this](const smtk::attribute::AttributePtr& attribute) { this->attributeModified(attribute); });

  /*
   * Connect the management controls to the corresponding resource actions.
   */
  QObject::connect(
    m_internals->AddButton, &QPushButton::clicked, this, &qtAttributeTableView::createAttribute);

  QObject::connect(
    m_internals->DeleteButton,
    &QPushButton::clicked,
    this,
    &qtAttributeTableView::deleteSelectedAttributes);

  /*
   * Update the delete button whenever the table selection changes.
   */
  QObject::connect(
    m_internals->Table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
      this->updateButtonState();
      this->updateColumnVisibility();
      this->updateAttributeEditor();
    });

  QObject::connect(
    m_internals->Table->selectionModel(),
    &QItemSelectionModel::currentChanged,
    this,
    [this](const QModelIndex&, const QModelIndex&) {
      this->updateColumnVisibility();
      this->updateAttributeEditor();
    });

  /*
   * Editing a discrete item can activate a different set of child items.
   * Recompute visibility after the model has committed such a change.
   */
  QObject::connect(
    m_internals->Model,
    &QAbstractItemModel::dataChanged,
    this,
    [this](const QModelIndex&, const QModelIndex&) {
      this->updateColumnVisibility();
      if (!m_internals->UpdatingFromAttributeEditor)
      {
        this->updateAttributeEditor(true);
      }
    });

  this->updateUI();
  this->updateButtonState();
  this->updateColumnVisibility();
  this->updateAttributeEditor();
}

void qtAttributeTableView::updateUI()
{
  if (!m_internals->Model)
  {
    return;
  }

  const auto resource = this->attributeResource();

  m_internals->Model->setAttributeResource(resource);

  /*
   * Attribute creation requires a resource and a valid definition. Disable the
   * add button until those conditions have been established below.
   */
  if (m_internals->AddButton)
  {
    m_internals->AddButton->setEnabled(false);
  }

  if (!resource)
  {
    m_internals->Model->setDefinition(nullptr);
    m_internals->Model->setAttributes({});
    this->updateButtonState();
    this->updateAttributeEditor(true);
    return;
  }

  const std::string typeName = this->definitionType();

  if (typeName.empty())
  {
    m_internals->Model->setDefinition(nullptr);
    m_internals->Model->setAttributes({});
    this->updateButtonState();
    this->updateAttributeEditor(true);
    return;
  }

  const auto definition = resource->findDefinition(typeName);

  if (!definition)
  {
    m_internals->Model->setDefinition(nullptr);
    m_internals->Model->setAttributes({});
    this->updateButtonState();
    this->updateAttributeEditor(true);
    return;
  }

  /*
   * Setting the definition rebuilds the model's column schema.
   */
  m_internals->Model->setDefinition(definition);

  this->rebuildAttributeList();

  if (m_internals->AddButton)
  {
    /*
     * Do not allow the view to violate the definition's uniqueness
     * constraint.
     */
    const bool canCreate = !definition->isUnique() || m_internals->Model->rowCount() == 0;

    m_internals->AddButton->setEnabled(canCreate);
  }

  m_internals->Table->resizeColumnsToContents();
  this->updateButtonState();
  this->updateColumnVisibility();
  this->updateAttributeEditor(true);
}

void qtAttributeTableView::createAttribute()
{
  const auto resource = this->attributeResource();
  const auto definition = m_internals->Model ? m_internals->Model->definition() : nullptr;

  if (!resource || !definition)
  {
    return;
  }

  /*
   * A unique definition may have at most one attribute instance.
   */
  if (definition->isUnique() && m_internals->Model->rowCount() != 0)
  {
    QMessageBox::information(
      m_internals->Widget,
      tr("Cannot Create Attribute"),
      tr("The definition \"%1\" permits only one attribute.")
        .arg(QString::fromStdString(definition->type())));

    return;
  }

  /*
   * Create the attribute without supplying a name. The attribute resource
   * generates a name that is unique within the resource.
   */
  const auto attribute = resource->createAttribute(definition);

  if (!attribute)
  {
    QMessageBox::warning(
      m_internals->Widget,
      tr("Attribute Creation Failed"),
      tr("An attribute based on \"%1\" could not be created.")
        .arg(QString::fromStdString(definition->type())));

    return;
  }

  /*
   * updateUI() rebuilds the table and updates the Add button's enabled state.
   * Select the new attribute only after the model reset is complete.
   */
  this->updateUI();
  this->selectAttribute(attribute); /*
   * Notify listeners that this view changed the resource.
   *
   * A production implementation may replace direct resource modification with
   * an SMTK operation so created components are reported in an operation
   * result.
   */
  Q_EMIT this->qtBaseView::modified();
}

void qtAttributeTableView::deleteSelectedAttributes()
{
  if (!m_internals->Table || !m_internals->Proxy || !m_internals->Model)
  {
    return;
  }

  const auto resource = this->attributeResource();

  if (!resource)
  {
    return;
  }

  const QModelIndexList selectedRows = m_internals->Table->selectionModel()->selectedRows();

  if (selectedRows.empty())
  {
    return;
  }

  /*
   * Multiple selected cells or proxy rows could resolve to the same attribute.
   * Store the attributes in a set to ensure each one is removed only once.
   */
  std::set<smtk::attribute::AttributePtr> attributesToDelete;

  for (const QModelIndex& proxyIndex : selectedRows)
  {
    const QModelIndex sourceIndex = m_internals->Proxy->mapToSource(proxyIndex);

    const auto attribute = m_internals->Model->attributeForRow(sourceIndex.row());

    if (attribute)
    {
      attributesToDelete.insert(attribute);
    }
  }

  if (attributesToDelete.empty())
  {
    return;
  }

  const auto& details = this->configuration()->details();
  if (details.attributeAsBool("AskToDelete"))
  {
    const QString confirmationText = attributesToDelete.size() == 1
      ? tr("Delete the selected attribute?")
      : tr("Delete the %1 selected attributes?").arg(attributesToDelete.size());

    const auto answer = QMessageBox::question(
      m_internals->Widget,
      tr("Delete Attributes"),
      confirmationText,
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No);

    if (answer != QMessageBox::Yes)
    {
      return;
    }
  }

  auto opManager = this->uiManager()->operationManager();
  std::shared_ptr<smtk::operation::Operation> deleterOp;
  std::string deleteOpName;
  if (details.attribute("DeleteOp", deleteOpName))
  {
    deleterOp = opManager->create(deleteOpName);
    if (deleterOp)
    {
      // If the deletion operation can accept the active task (and
      // there is one), the populate it here.
      if (auto taskItem = deleterOp->parameters()->findComponent("task"))
      {
        if (auto* activeTask = smtk::task::getActiveTask(this->uiManager()->managers()))
        {
          taskItem->appendValue(activeTask->shared_from_this());
        }
      }
    }
  }
  if (!deleterOp)
  {
    if (!deleteOpName.empty())
    {
      qCritical() << "Could not create \"" << QString::fromStdString(deleteOpName)
                  << "\" operation.";
    }
    deleterOp = opManager->create("smtk::attribute::DeleteAttribute");
  }

  for (const auto& attribute : attributesToDelete)
  {
    /*
     * removeAttribute() may fail when the attribute cannot legally be removed,
     * such as when resource constraints or references prevent deletion.
     */
    deleterOp->parameters()->associate(attribute);
  }

  QPointer<qtAttributeTableView> myself(this);
  deleterOp->addHandler(
    [myself, this, attributesToDelete](
      smtk::operation::Operation&, smtk::operation::Operation::Result res) {
      /*
   * Rebuild once after all removals rather than resetting the model after each
   * individual attribute.
   */
      if (!myself)
      {
        return;
      }
      this->updateUI();

      auto numExpunged = res->findComponent("expunged")->numberOfValues();
      if (attributesToDelete.size() > numExpunged)
      {
        QMessageBox::warning(
          m_internals->Widget,
          tr("Attribute Deletion Incomplete"),
          tr("%1 of %2 selected attributes were deleted.")
            .arg(attributesToDelete.size())
            .arg(numExpunged));
      }

      if (numExpunged != 0)
      {
        /*
     * Notify listeners that the attribute resource changed.
     *
     * An operation-based implementation should report these attributes as
     * expunged components in the operation result.
     */
        Q_EMIT this->qtBaseView::modified();
      }
    },
    0);

  opManager->launchers()(deleterOp);
}

void qtAttributeTableView::updateButtonState()
{
  if (!m_internals->DeleteButton || !m_internals->Table || !m_internals->Table->selectionModel())
  {
    return;
  }

  const bool hasSelection = m_internals->Table->selectionModel()->hasSelection();

  m_internals->DeleteButton->setEnabled(hasSelection);
}

void qtAttributeTableView::updateColumnVisibility()
{
  if (!m_internals->Table || !m_internals->Proxy || !m_internals->Model)
  {
    return;
  }

  QModelIndex sourceCurrent;
  if (
    m_internals->HideInactiveChildren && m_internals->Table->selectionModel() &&
    m_internals->Table->selectionModel()->hasSelection())
  {
    sourceCurrent =
      m_internals->Proxy->mapToSource(m_internals->Table->selectionModel()->currentIndex());
  }

  for (int column = 0; column < m_internals->Model->columnCount(); ++column)
  {
    bool hide = false;
    if (sourceCurrent.isValid())
    {
      const QModelIndex sourceIndex = m_internals->Model->index(sourceCurrent.row(), column);
      /*
       * The attribute-name column has no associated item. Missing items are
       * likewise not evidence that a column is an inactive conditional child.
       */
      hide = m_internals->Model->itemForIndex(sourceIndex) &&
        !m_internals->Model->isItemActive(sourceIndex);
    }
    m_internals->Table->setColumnHidden(column, hide);
  }
}

void qtAttributeTableView::updateAttributeEditor(bool rebuild)
{
  if (
    !m_internals->AttributeEditorFrame || !m_internals->AttributeEditorScrollArea ||
    !m_internals->Table || !m_internals->Proxy || !m_internals->Model)
  {
    return;
  }

  smtk::attribute::AttributePtr attribute;
  if (m_internals->Table->selectionModel() && m_internals->Table->selectionModel()->hasSelection())
  {
    const QModelIndex sourceCurrent =
      m_internals->Proxy->mapToSource(m_internals->Table->selectionModel()->currentIndex());
    if (sourceCurrent.isValid())
    {
      attribute = m_internals->Model->attributeForRow(sourceCurrent.row());
    }
  }

  if (!attribute)
  {
    if (auto smtkSelection = this->uiManager()->selection())
    {
      std::vector<smtk::resource::Component::Ptr> blank;
      smtkSelection->modifySelection(
        blank,
        "qtAttributeTableView",
        (1 << this->uiManager()->selectionBit()),
        smtk::view::SelectionAction::UNFILTERED_REPLACE,
        /*bitwise*/ true);
      // smtkSelection->resetSelectionBits("qtAttributeTableView", this->uiManager()->selectionBit());
    }
  }

  if (!attribute || !containsGroupItem(attribute->definition()))
  {
    delete m_internals->AttributeEditor;
    m_internals->AttributeEditor = nullptr;
    m_internals->AttributeEditorScrollArea->setVisible(false);
    return;
  }

  if (
    !rebuild && m_internals->AttributeEditor &&
    m_internals->AttributeEditor->attribute() == attribute)
  {
    return;
  }

  m_internals->AttributeEditorFrame->setTitle(
    tr("Selected Attribute: %1").arg(QString::fromStdString(attribute->name())));

  delete m_internals->AttributeEditor;
  m_internals->AttributeEditor = nullptr;

  m_internals->AttributeEditor = new qtAttribute(
    attribute, this->findStyle(attribute->definition()), m_internals->AttributeEditorFrame, this);
  m_internals->AttributeEditor->createBasicLayout(false);

  if (QWidget* editorWidget = m_internals->AttributeEditor->widget())
  {
    auto* editorLayout = qobject_cast<QVBoxLayout*>(m_internals->AttributeEditorFrame->layout());
    editorLayout->insertWidget(0, editorWidget, 0, Qt::AlignTop);
    m_internals->AttributeEditorScrollArea->setVisible(true);

    QObject::connect(
      m_internals->AttributeEditor, &qtAttribute::modified, this, [this, attribute]() {
        m_internals->UpdatingFromAttributeEditor = true;
        m_internals->Model->refreshAttribute(attribute);
        m_internals->UpdatingFromAttributeEditor = false;
        this->updateColumnVisibility();
        this->attributeModified(attribute);
      });

    if (this->advanceLevelVisible())
    {
      m_internals->AttributeEditor->showAdvanceLevelOverlay(true);
    }
  }
  else
  {
    m_internals->AttributeEditorScrollArea->setVisible(false);
  }

  // Replace SMTK's primary selection with the newly-selected attribute.
  if (auto smtkSelection = this->uiManager()->selection())
  {
    std::vector<smtk::attribute::Attribute::Ptr> seln;
    seln.push_back(attribute);
    smtkSelection->modifySelection(
      seln,
      "qtAttributeTableView",
      (1 << this->uiManager()->selectionBit()),
      smtk::view::SelectionAction::UNFILTERED_REPLACE,
      /*bitwise*/ true);
  }
}

void qtAttributeTableView::selectAttribute(const smtk::attribute::AttributePtr& attribute)
{
  if (!attribute || !m_internals->Model || !m_internals->Proxy || !m_internals->Table)
  {
    return;
  }

  const auto& attributes = m_internals->Model->attributes();

  const auto iterator = std::find(attributes.begin(), attributes.end(), attribute);

  if (iterator == attributes.end())
  {
    return;
  }

  const int sourceRow = static_cast<int>(std::distance(attributes.begin(), iterator));

  const QModelIndex sourceIndex = m_internals->Model->index(sourceRow, 0);

  const QModelIndex proxyIndex = m_internals->Proxy->mapFromSource(sourceIndex);

  if (!proxyIndex.isValid())
  {
    return;
  }

  /*
   * Clear the previous selection and select the complete row corresponding to
   * the newly created attribute.
   */
  m_internals->Table->selectionModel()->setCurrentIndex(
    proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

  m_internals->Table->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);

  /*
   * Begin editing the attribute-name cell so the generated name can be
   * replaced immediately.
   */
  m_internals->Table->edit(proxyIndex);
}

std::string qtAttributeTableView::definitionType() const
{
  const auto& configuration = this->configuration();

  if (!configuration)
  {
    return {};
  }

  const auto& details = configuration->details();

  /*
   * Expected configuration:
   *
   * <View Type="AttributeTable">
   *   <AttributeTypes>
   *     <Att Type="BoundaryCondition"/>
   *   </AttributeTypes>
   * </View>
   */
  int index = details.findChild("AttributeTypes");
  if (index == -1)
  {
    // The child is missing
    return {};
  }
  const auto& attributeTypes = details.child(index);

  index = attributeTypes.findChild("Att");
  if (index == -1)
  {
    // The child is missing
    return {};
  }
  const auto& attributeEntry = attributeTypes.child(index);

  std::string typeName;
  attributeEntry.attribute("Type", typeName);

  return typeName;
}

const smtk::view::Configuration::Component& qtAttributeTableView::findStyle(
  const smtk::attribute::DefinitionPtr& definition,
  bool isOriginalDefinition) const
{
  static smtk::view::Configuration::Component emptyStyle;

  if (!definition)
  {
    return emptyStyle;
  }

  const auto componentIt = m_internals->AttributeComponents.find(definition->type());
  if (
    componentIt != m_internals->AttributeComponents.end() && componentIt->second.numberOfChildren())
  {
    return componentIt->second;
  }

  const auto styleIt = m_internals->AttributeStyles.find(definition->type());
  if (styleIt != m_internals->AttributeStyles.end())
  {
    return this->uiManager()->findStyle(definition, styleIt->second);
  }

  if (definition->baseDefinition())
  {
    const auto& inheritedStyle = this->findStyle(definition->baseDefinition(), false);
    if (inheritedStyle.numberOfChildren())
    {
      return inheritedStyle;
    }
  }

  return isOriginalDefinition ? this->uiManager()->findStyle(definition) : emptyStyle;
}

void qtAttributeTableView::rebuildAttributeList()
{
  const auto resource = this->attributeResource();
  const auto definition = m_internals->Model->definition();

  if (!resource || !definition)
  {
    m_internals->Model->setAttributes({});
    return;
  }

  std::vector<smtk::attribute::AttributePtr> attributes;
  resource->findAttributes(definition, attributes);

  /*
   * Remove attributes that are not relevant under the view's active category
   * and advance-level settings.
   *
   * Adapt isRelevant() to the exact API exposed by the SMTK version being
   * compiled.
   */
  attributes.erase(
    std::remove_if(
      attributes.begin(),
      attributes.end(),
      [this](const smtk::attribute::AttributePtr& attribute) {
        return !attribute || !attribute->isRelevant();
      }),
    attributes.end());

  std::sort(attributes.begin(), attributes.end(), [](const auto& lhs, const auto& rhs) {
    return lhs->name() < rhs->name();
  });

  m_internals->Model->setAttributes(attributes);
}

void qtAttributeTableView::attributeModified(const smtk::attribute::AttributePtr& attribute)
{
  if (!attribute)
  {
    return;
  }

  /*
   * Inform listeners that an item in the attribute resource changed.
   *
   * SMTK's Signal operation can also be used to identify modified, created,
   * and expunged components to operation observers and other views.
   */
  Q_EMIT this->qtBaseView::modified();
}

void qtAttributeTableView::updateViewWithOperationResults(
  const smtk::operation::Operation& op,
  const std::shared_ptr<smtk::attribute::Attribute>& result)
{
  (void)op;
  bool shouldUpdate = false;
  for (const auto& created : *result->findComponent("created"))
  {
    if (auto att = std::dynamic_pointer_cast<smtk::attribute::Attribute>(created))
    {
      if (att->resource() == this->attributeResource())
      {
        shouldUpdate = true;
        break;
      }
    }
  }
  if (!shouldUpdate)
  {
    for (const auto& expunged : *result->findComponent("expunged"))
    {
      if (auto att = std::dynamic_pointer_cast<smtk::attribute::Attribute>(expunged))
      {
        if (att->resource() == this->attributeResource())
        {
          shouldUpdate = true;
          break;
        }
      }
    }
  }
  if (shouldUpdate)
  {
    this->updateUI();
  }
}

} // namespace extension
} // namespace smtk
