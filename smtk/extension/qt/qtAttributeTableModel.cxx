//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeTableModel.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/utility/Queries.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/io/Logger.h"

#include <QBrush>
#include <QColor>
#include <QFont>

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>

namespace smtk
{
namespace extension
{

namespace
{

QString itemLabel(const smtk::attribute::ItemDefinitionPtr& definition)
{
  if (!definition)
  {
    return {};
  }

  const auto& label = definition->label();
  return QString::fromStdString(label.empty() ? definition->name() : label);
}

} // anonymous namespace

qtAttributeTableModel::qtAttributeTableModel(QObject* parent)
  : QAbstractTableModel(parent)
{
}

void qtAttributeTableModel::setAttributeResource(const smtk::attribute::ResourcePtr& resource)
{
  if (m_resource == resource)
  {
    return;
  }

  beginResetModel();
  m_resource = resource;
  m_definition.reset();
  m_attributes.clear();
  m_columns.clear();
  endResetModel();
}

smtk::attribute::ResourcePtr qtAttributeTableModel::attributeResource() const
{
  return m_resource;
}

void qtAttributeTableModel::setDefinition(const smtk::attribute::DefinitionPtr& definition)
{
  if (m_definition == definition)
  {
    return;
  }

  beginResetModel();

  m_definition = definition;
  m_attributes.clear();
  m_columns.clear();

  if (m_definition)
  {
    rebuildColumns();
  }

  endResetModel();
}

smtk::attribute::DefinitionPtr qtAttributeTableModel::definition() const
{
  return m_definition;
}

void qtAttributeTableModel::setAttributes(
  const std::vector<smtk::attribute::AttributePtr>& attributes)
{
  beginResetModel();
  m_attributes = attributes;
  endResetModel();
}

const std::vector<smtk::attribute::AttributePtr>& qtAttributeTableModel::attributes() const
{
  return m_attributes;
}

void qtAttributeTableModel::setAttributeModifiedCallback(AttributeModifiedCallback callback)
{
  m_attributeModified = std::move(callback);
}

void qtAttributeTableModel::setColumnDisplay(
  ColumnDisplay display,
  const std::set<std::string>& itemPaths,
  const std::vector<SharedColumn>& sharedColumns)
{
  if (
    m_columnDisplay == display && m_columnItemPaths == itemPaths &&
    m_sharedColumns == sharedColumns)
  {
    return;
  }

  beginResetModel();
  m_columnDisplay = display;
  m_columnItemPaths = itemPaths;
  m_sharedColumns = sharedColumns;
  if (m_definition)
  {
    rebuildColumns();
  }
  endResetModel();
}

int qtAttributeTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_attributes.size());
}

int qtAttributeTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_columns.size());
}

QVariant qtAttributeTableModel::data(const QModelIndex& index, int role) const
{
  if (
    !index.isValid() || index.row() < 0 || index.column() < 0 || index.row() >= this->rowCount() ||
    index.column() >= this->columnCount())
  {
    return {};
  }

  const auto attribute = this->attributeForRow(index.row());

  const auto* descriptor = this->columnDescriptor(index.column());

  if (!attribute || !descriptor)
  {
    return {};
  }

  /*
   * The attribute-name column does not correspond to an SMTK Item, so it
   * does not participate in item validity or default-value coloring.
   */
  if (descriptor->Kind == ColumnKind::AttributeName)
  {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      return QString::fromStdString(attribute->name());
    }

    return {};
  }

  const auto item = this->itemForIndex(index);

  if (!item)
  {
    if (descriptor->isShared())
    {
      if (role == Qt::DisplayRole)
      {
        return QStringLiteral("—");
      }
      if (role == Qt::BackgroundRole)
      {
        return QBrush(QColor(Qt::gray));
      }
      if (role == Qt::ToolTipRole)
      {
        return QStringLiteral("No configured item is active for this row.");
      }
    }
    return {};
  }

  const bool itemIsActive = this->isItemActive(item);
  /*
   * Conditional child items that are not active for the parent's current
   * discrete value remain present in the table but are displayed as
   * unavailable.
   */
  if (!itemIsActive)
  {
    if (role == Qt::DisplayRole)
    {
      return QStringLiteral("—");
    }

    if (role == Qt::ForegroundRole)
    {
      return QBrush(QColor(Qt::darkGray));
    }

    if (role == Qt::BackgroundRole)
    {
      return QBrush(QColor(Qt::gray));
    }

    if (role == Qt::ToolTipRole)
    {
      return QStringLiteral("This item is not active for the currently selected value.");
    }

    /*
     * Do not apply invalid or default-value coloring to an inactive child.
     */
    if (role == Qt::FontRole)
    {
      return {};
    }
  }

  /*
   * Use the cell background to communicate item state.
   *
   * Invalid takes precedence over default-value status since invalid data
   * generally requires the user's attention.
   */
  if (role == Qt::BackgroundRole)
  {
    if (m_uiManager && !item->isValid())
    {
      return QBrush(m_uiManager->correctedInvalidValueColor());
    }

    auto valueItem = dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
    if (
      m_uiManager && valueItem && descriptor->Kind == ColumnKind::ItemValue &&
      valueItem->isUsingDefault(descriptor->Element))
    {
      return QBrush(m_uiManager->correctedDefaultValueColor());
    }
  }

  /*
   * Optionally alter the text appearance of default values to provide an
   * additional visual cue.
   */
  if (role == Qt::ForegroundRole)
  {
    auto valueItem = dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
    if (
      valueItem && descriptor->Kind == ColumnKind::ItemValue &&
      valueItem->isUsingDefault(descriptor->Element))
    {
      return QBrush(QColor(90, 90, 90));
    }
  }

  /*
   * Invalid values may also be rendered using a bold font.
   */
  if (role == Qt::FontRole && !item->isValid())
  {
    QFont font;
    font.setBold(true);
    return font;
  }

  if (role == Qt::ToolTipRole)
  {
    QString tooltip = this->itemToolTip(item);

    if (!item->isValid())
    {
      tooltip += QStringLiteral("\n\nThis item is currently invalid.");
    }
    else
    {
      auto valueItem = dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
      if (
        valueItem && descriptor->Kind == ColumnKind::ItemValue &&
        valueItem->isUsingDefault(descriptor->Element))
      {
        tooltip += QStringLiteral("\n\nThis value is using its default.");
      }
    }

    return tooltip;
  }

  if (descriptor->Kind == ColumnKind::ItemEnabledState)
  {
    auto voidItem = std::dynamic_pointer_cast<smtk::attribute::VoidItem>(item);

    if (!voidItem)
    {
      return {};
    }

    if (role == Qt::CheckStateRole)
    {
      return voidItem->isEnabled() ? Qt::Checked : Qt::Unchecked;
    }

    return {};
  }

  if (descriptor->Kind == ColumnKind::ItemSummary)
  {
    if (role == Qt::DisplayRole)
    {
      return QStringLiteral("Configured");
    }

    return {};
  }

  return this->valueItemData(item, descriptor->Element, role);
}

bool qtAttributeTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (
    !index.isValid() || index.row() < 0 || index.column() < 0 || index.row() >= rowCount() ||
    index.column() >= columnCount())
  {
    return false;
  }

  const auto attribute = attributeForRow(index.row());
  const auto* descriptor = columnDescriptor(index.column());

  if (!attribute || !descriptor)
  {
    return false;
  }

  bool modified = false;
  smtk::attribute::ItemPtr modifiedItem;

  if (descriptor->Kind == ColumnKind::AttributeName && role == Qt::EditRole)
  {
    const std::string newName = value.toString().toStdString();

    if (newName.empty() || newName == attribute->name())
    {
      return false;
    }

    // Depending on the SMTK version and application policy,
    // renaming may be better performed through an operation.
    modified = attribute->attributeResource()->rename(attribute, newName);
  }
  else
  {
    const auto item = itemForIndex(index);
    if (!item)
    {
      return false;
    }
    /*
     * Do not allow values to be assigned through the table model when a
     * conditional child is inactive.
     */
    if (!this->isItemActive(item))
    {
      return false;
    }

    if (descriptor->Kind == ColumnKind::ItemEnabledState && role == Qt::CheckStateRole)
    {
      auto voidItem = std::dynamic_pointer_cast<smtk::attribute::VoidItem>(item);

      if (voidItem)
      {
        const bool enabled = value.toInt() == static_cast<int>(Qt::Checked);

        if (voidItem->isEnabled() != enabled)
        {
          voidItem->setIsEnabled(enabled);
          modified = true;
        }
      }
    }
    else if (descriptor->Kind == ColumnKind::ItemValue && role == Qt::EditRole)
    {
      modified = setValueItemData(item, descriptor->Element, value);
    }

    if (modified)
    {
      /*
       * itemForIndex() resolves the actual active candidate for a shared
       * column. Retaining that item ensures the Signal operation reports the
       * path that changed rather than the logical column's configuration name.
       */
      modifiedItem = item;
    }
  }

  if (!modified)
  {
    return false;
  }

  /*
   * Refresh the complete row because a discrete selection may affect the
   * relevance, validity, default state, or enabled state of other items in the
   * same attribute.
   */
  Q_EMIT dataChanged(
    this->index(index.row(), 0),
    this->index(index.row(), this->columnCount() - 1),
    { Qt::DisplayRole,
      Qt::EditRole,
      Qt::CheckStateRole,
      Qt::BackgroundRole,
      Qt::ForegroundRole,
      Qt::FontRole,
      Qt::ToolTipRole });
  if (m_attributeModified)
  {
    std::vector<std::string> itemPaths;
    if (modifiedItem)
    {
      itemPaths.push_back(modifiedItem->path());
    }
    m_attributeModified(attribute, itemPaths);
  }

  return true;
}

QVariant qtAttributeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return {};
  }

  if (orientation == Qt::Vertical)
  {
    return section + 1;
  }

  const auto* descriptor = columnDescriptor(section);
  if (!descriptor)
  {
    return {};
  }

  return QString::fromStdString(descriptor->Label);
}

Qt::ItemFlags qtAttributeTableModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return Qt::NoItemFlags;
  }

  const auto* descriptor = columnDescriptor(index.column());
  if (!descriptor)
  {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (descriptor->Kind == ColumnKind::AttributeName)
  {
    return result | Qt::ItemIsEditable;
  }

  const auto item = itemForIndex(index);
  if (!item)
  {
    return result;
  }

  /*
   * Conditional child items remain visible in the table so the column schema is
   * stable, but inactive children cannot be edited.
   */
  if (!this->isItemActive(item))
  {
    return Qt::ItemIsSelectable;
  }

  if (!item->isEnabled() && descriptor->Kind != ColumnKind::ItemEnabledState)
  {
    return Qt::ItemIsSelectable;
  }
  switch (descriptor->Kind)
  {
    case ColumnKind::ItemEnabledState:
      result |= Qt::ItemIsUserCheckable;
      break;

    case ColumnKind::ItemValue:
      result |= Qt::ItemIsEditable;
      break;

    case ColumnKind::ItemSummary:
      break;

    case ColumnKind::AttributeName:
      break;
  }

  return result;
}

smtk::attribute::AttributePtr qtAttributeTableModel::attributeForRow(int row) const
{
  if (row < 0 || row >= static_cast<int>(m_attributes.size()))
  {
    return nullptr;
  }

  return m_attributes[static_cast<std::size_t>(row)];
}

smtk::attribute::ItemPtr qtAttributeTableModel::itemForIndex(const QModelIndex& index) const
{
  /*
   * Keep all QModelIndex-to-SMTK-item translation in one method. This avoids
   * duplicating item-path resolution in data(), setData(), flags(), delegates,
   * and validation code.
   */
  if (!index.isValid())
  {
    return nullptr;
  }

  const auto attribute = this->attributeForRow(index.row());
  const auto* descriptor = this->columnDescriptor(index.column());

  if (!attribute || !descriptor)
  {
    return nullptr;
  }

  if (descriptor->isShared())
  {
    /*
     * A shared column has no single ItemPath. Resolve every configured path
     * against this row's attribute and select the candidate whose complete
     * conditional-parent chain is active. The attribute utility proved that
     * at most one candidate can be active for any enumeration of the common
     * controlling item.
     */
    smtk::attribute::ItemPtr activeItem;
    for (const auto& candidate : descriptor->Alternatives)
    {
      const auto item = attribute->itemAtPath(candidate.Path);
      if (!item || !this->isItemActive(item))
      {
        continue;
      }

      // Static validation should make this impossible. Refuse to choose an
      // arbitrary value if a malformed resource violates the invariant.
      if (activeItem)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Multiple active items found for shared table column \"" << descriptor->Label << "\".");
        return nullptr;
      }
      activeItem = item;
    }
    return activeItem;
  }

  if (descriptor->ItemPath.empty())
  {
    return nullptr;
  }
  return attribute->itemAtPath(descriptor->ItemPath);
}

const qtAttributeTableModel::ColumnDescriptor* qtAttributeTableModel::columnDescriptor(
  int column) const
{
  if (column < 0 || column >= static_cast<int>(m_columns.size()))
  {
    return nullptr;
  }

  return &m_columns[static_cast<std::size_t>(column)];
}

void qtAttributeTableModel::refreshAttribute(const smtk::attribute::AttributePtr& attribute)
{
  const auto iterator = std::find(m_attributes.begin(), m_attributes.end(), attribute);

  if (iterator == m_attributes.end())
  {
    return;
  }

  const int row = static_cast<int>(std::distance(m_attributes.begin(), iterator));

  if (columnCount() == 0)
  {
    return;
  }

  Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
}

smtk::attribute::ItemDefinitionPtr qtAttributeTableModel::itemDefinitionForIndex(
  const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return nullptr;
  }

  if (const auto item = this->itemForIndex(index))
  {
    const auto* descriptor = this->columnDescriptor(index.column());
    if (descriptor && descriptor->isShared())
    {
      /*
       * The definition for a shared column is row-dependent: one row may use
       * aValue while another uses bValue. qtAttributeTableDelegate asks this
       * method for the active definition so it can construct the correct
       * editor and validator.
       *
       * Item::definition() returns a ConstItemDefinitionPtr from this const
       * method, while this model's public API historically returns the
       * non-const ItemDefinitionPtr consumed by the delegate. Do not remove
       * constness with a cast. Instead, identify the active definition by
       * address and return the non-const pointer retained during schema
       * validation in SharedItemCandidate.
       */
      const auto itemDefinition = item->definition();
      for (const auto& candidate : descriptor->Alternatives)
      {
        if (candidate.Definition.get() == itemDefinition.get())
        {
          return candidate.Definition;
        }
      }
    }
  }

  const auto* descriptor = this->columnDescriptor(index.column());
  if (!descriptor)
  {
    return nullptr;
  }

  /*
   * Ordinary columns always use their schema definition. An inactive shared
   * column also reaches this fallback; its first validated candidate provides
   * stable type metadata for non-row-specific queries, while flags() prevents
   * creation of an editor until a candidate becomes active.
   */
  return descriptor->Definition;
}

bool qtAttributeTableModel::isValueSet(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return false;
  }

  const auto item = this->itemForIndex(index);

  const auto valueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);

  if (!valueItem)
  {
    return false;
  }

  const auto* descriptor = this->columnDescriptor(index.column());

  if (!descriptor || descriptor->Element >= valueItem->numberOfValues())
  {
    return false;
  }

  return valueItem->isSet(descriptor->Element);
}

bool qtAttributeTableModel::isDiscrete(const QModelIndex& index) const
{
  const auto item = this->itemForIndex(index);

  const auto valueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);

  return valueItem && valueItem->isDiscrete();
}

bool qtAttributeTableModel::isSharedColumn(const QModelIndex& index) const
{
  const auto* descriptor = index.isValid() ? this->columnDescriptor(index.column()) : nullptr;
  return descriptor && descriptor->isShared();
}

QStringList qtAttributeTableModel::discreteValues(const QModelIndex& index) const
{
  QStringList values;

  const auto item = this->itemForIndex(index);

  const auto valueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);

  if (!valueItem || !valueItem->isDiscrete())
  {
    return values;
  }

  const auto* definition =
    dynamic_cast<const smtk::attribute::ValueItemDefinition*>(valueItem->definition().get());

  if (!definition)
  {
    return values;
  }

  /*
   * Populate the editor using the enumerations defined by the item's
   * ValueItemDefinition.
   *
   * If your SMTK version supports ValueItem::relevantEnums(), that method can
   * be used here instead to filter out enumerations that are not currently
   * relevant.
   */
  const std::size_t numberOfDiscreteValues = definition->numberOfDiscreteValues();

  for (std::size_t i = 0; i < numberOfDiscreteValues; ++i)
  {
    values.push_back(QString::fromStdString(definition->discreteEnum(i)));
  }

  return values;
}

QStringList qtAttributeTableModel::possibleDiscreteValues(const QModelIndex& index) const
{
  QStringList values;
  const auto* descriptor = index.isValid() ? this->columnDescriptor(index.column()) : nullptr;
  if (!descriptor || !descriptor->isShared())
  {
    return this->discreteValues(index);
  }

  // Include candidates that happen to be inactive in all current rows so
  // resizeColumnsToContents() still accounts for their longest enumeration.
  for (const auto& candidate : descriptor->Alternatives)
  {
    const auto definition =
      std::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(candidate.Definition);
    if (!definition || !definition->isDiscrete())
    {
      continue;
    }
    for (std::size_t i = 0; i < definition->numberOfDiscreteValues(); ++i)
    {
      const QString value = QString::fromStdString(definition->discreteEnum(i));
      if (!values.contains(value))
      {
        values.push_back(value);
      }
    }
  }
  return values;
}

bool qtAttributeTableModel::isItemActive(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return false;
  }

  return this->isItemActive(this->itemForIndex(index));
}

bool qtAttributeTableModel::isItemActive(const smtk::attribute::ItemPtr& item) const
{
  if (!item)
  {
    return false;
  }

  /*
   * Walk up the item's parent hierarchy. Whenever the parent is a discrete
   * ValueItem, verify that the current item is one of that parent's active
   * children.
   *
   * Walking the complete hierarchy also supports nested conditional items:
   *
   *   discrete A
   *     -> discrete B
   *          -> child C
   */
  smtk::attribute::ItemPtr current = item;

  while (current)
  {
    const auto parent = current->owningItem();

    if (!parent)
    {
      // Reached a top-level attribute item.
      return true;
    }

    auto parentValueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(parent);

    if (parentValueItem && parentValueItem->isDiscrete())
    {
      if (!parentValueItem->isChildActive(current))
      {
        return false;
      }
    }

    current = parent;
  }

  return true;
}

QString qtAttributeTableModel::currentDiscreteValue(const QModelIndex& index) const
{
  const auto item = this->itemForIndex(index);

  const auto valueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);

  if (!valueItem || !valueItem->isDiscrete())
  {
    return {};
  }

  const auto* descriptor = this->columnDescriptor(index.column());

  if (!descriptor)
  {
    return {};
  }

  const std::size_t element = descriptor->Element;

  if (element >= valueItem->numberOfValues() || !valueItem->isSet(element))
  {
    return {};
  }

  const auto* definition =
    dynamic_cast<const smtk::attribute::ValueItemDefinition*>(valueItem->definition().get());

  if (!definition)
  {
    return {};
  }

  const std::size_t discreteIndex = valueItem->discreteIndex(element);

  if (discreteIndex >= definition->numberOfDiscreteValues())
  {
    return {};
  }

  return QString::fromStdString(definition->discreteEnum(discreteIndex));
}

void qtAttributeTableModel::rebuildColumns()
{
  m_columns.clear();
  m_columns.push_back({ ColumnKind::AttributeName, "Attribute", {}, 0, nullptr });

  if (!m_definition)
  {
    return;
  }

  const std::size_t numberOfItems = m_definition->numberOfItemDefinitions();

  for (std::size_t i = 0; i < numberOfItems; ++i)
  {
    const auto itemDefinition = m_definition->itemDefinition(static_cast<int>(i));

    if (!itemDefinition)
    {
      continue;
    }
    appendItemDefinitionColumns(itemDefinition, itemDefinition->name(), {});
  }

  if (m_columnDisplay == ColumnDisplay::All)
  {
    this->appendSharedColumns();
    return;
  }

  m_columns.erase(
    std::remove_if(
      std::next(m_columns.begin()),
      m_columns.end(),
      [this](const ColumnDescriptor& column) {
        const bool isTopLevel = column.ItemPath.find('/') == std::string::npos;

        switch (m_columnDisplay)
        {
          case ColumnDisplay::TopLevelNonGroup:
            return !isTopLevel ||
              std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(column.Definition) !=
              nullptr;

          case ColumnDisplay::TopLevelDiscrete:
          {
            const auto valueDefinition =
              std::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(column.Definition);
            return !isTopLevel || !valueDefinition || !valueDefinition->isDiscrete();
          }

          case ColumnDisplay::UserSpecified:
            return m_columnItemPaths.find(column.ItemPath) == m_columnItemPaths.end();

          case ColumnDisplay::All:
            return false;
        }
        return false;
      }),
    m_columns.end());

  this->appendSharedColumns();
}

void qtAttributeTableModel::appendSharedColumns()
{
  /*
   * Track paths already claimed by an earlier logical column. Allowing one
   * conditional child to supply two shared columns would make the resulting
   * schema ambiguous and could expose two editors for the same underlying
   * value.
   */
  std::set<std::string> consumedPaths;
  for (const auto& shared : m_sharedColumns)
  {
    if (shared.Name.empty() && shared.Label.empty())
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Ignoring an unnamed shared table column.");
      continue;
    }

    const auto validation =
      smtk::attribute::utility::validateExclusiveConditionalItems(m_definition, shared.ItemPaths);
    if (!validation)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Ignoring shared table column \"" << shared.Label << "\": " << validation.error);
      continue;
    }

    /*
     * validateExclusiveConditionalItems() handles definition-tree and
     * activation semantics. The checks below are table-specific: a single
     * table cell can currently edit only one fixed value, and candidates must
     * have a common Item::Type so display, editing, and sorting are coherent.
     */
    bool compatible = true;
    smtk::attribute::Item::Type itemType = smtk::attribute::Item::NUMBER_OF_TYPES;
    for (std::size_t i = 0; i < validation.itemDefinitions.size(); ++i)
    {
      const auto valueDefinition = std::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(
        validation.itemDefinitions[i]);
      if (
        !valueDefinition || valueDefinition->isExtensible() ||
        valueDefinition->numberOfRequiredValues() != 1)
      {
        compatible = false;
        break;
      }
      if (i == 0)
      {
        itemType = valueDefinition->type();
      }
      else if (valueDefinition->type() != itemType)
      {
        compatible = false;
        break;
      }
      if (consumedPaths.find(shared.ItemPaths[i]) != consumedPaths.end())
      {
        compatible = false;
        break;
      }
    }

    if (!compatible)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Ignoring shared table column \"" << shared.Label
                                          << "\": candidates must be unique, scalar, "
                                             "non-extensible value items of the same type.");
      continue;
    }

    consumedPaths.insert(shared.ItemPaths.begin(), shared.ItemPaths.end());

    // A valid shared column replaces the candidates' ordinary columns under
    // modes such as All; UserSpecified configurations normally never added
    // those ordinary paths in the first place.
    const std::set<std::string> candidatePaths(shared.ItemPaths.begin(), shared.ItemPaths.end());
    m_columns.erase(
      std::remove_if(
        std::next(m_columns.begin()),
        m_columns.end(),
        [&candidatePaths](const ColumnDescriptor& column) {
          return candidatePaths.find(column.ItemPath) != candidatePaths.end();
        }),
      m_columns.end());

    ColumnDescriptor descriptor;
    descriptor.Kind = ColumnKind::ItemValue;
    descriptor.Label = shared.Label.empty() ? shared.Name : shared.Label;
    descriptor.Element = 0;
    // Keep a deterministic fallback definition for rows where no candidate is
    // active. Row-specific editor queries use Alternatives instead.
    descriptor.Definition = validation.itemDefinitions.front();
    descriptor.ControllingItemPath = validation.controllingPath;
    for (std::size_t i = 0; i < shared.ItemPaths.size(); ++i)
    {
      descriptor.Alternatives.push_back({ shared.ItemPaths[i], validation.itemDefinitions[i] });
    }
    m_columns.push_back(std::move(descriptor));
  }
}

void qtAttributeTableModel::appendItemDefinitionColumns(
  const smtk::attribute::ItemDefinitionPtr& itemDefinition,
  const std::string& itemPath,
  const std::string& labelPrefix)
{
  if (!itemDefinition)
  {
    return;
  }

  QString baseLabel = itemLabel(itemDefinition);

  if (!labelPrefix.empty())
  {
    baseLabel = QString::fromStdString(labelPrefix) + QStringLiteral(" / ") + baseLabel;
  }

  if (std::dynamic_pointer_cast<smtk::attribute::VoidItemDefinition>(itemDefinition))
  {
    m_columns.push_back(
      { ColumnKind::ItemEnabledState, baseLabel.toStdString(), itemPath, 0, itemDefinition });

    return;
  }

  if (
    auto valueDefinition =
      std::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(itemDefinition))
  {
    const std::size_t requiredValues = valueDefinition->numberOfRequiredValues();

    /*
     * Extensible items do not have a stable value-column schema. Represent the
     * parent item itself using a summary column.
     */
    if (valueDefinition->isExtensible())
    {
      m_columns.push_back(
        { ColumnKind::ItemSummary, baseLabel.toStdString(), itemPath, 0, itemDefinition });
    }
    else
    {
      const std::size_t columnCount = std::max<std::size_t>(requiredValues, 1);

      for (std::size_t element = 0; element < columnCount; ++element)
      {
        QString label = baseLabel;

        if (columnCount > 1)
        {
          label += QStringLiteral(" %1").arg(element + 1);
        }

        m_columns.push_back(
          { ColumnKind::ItemValue, label.toStdString(), itemPath, element, itemDefinition });
      }
    }

    /*
     * A discrete ValueItem can own conditional child items. Add columns for
     * every possible child definition so the table maintains a stable column
     * schema regardless of which enumeration is selected in an individual row.
     *
     * Whether a particular child is active is determined dynamically for each
     * attribute row.
     */
    if (valueDefinition->isDiscrete())
    {
      for (const auto& childEntry : valueDefinition->childrenItemDefinitions())
      {
        const auto& childName = childEntry.first;

        const auto& childDefinition = childEntry.second;

        if (!childDefinition)
        {
          continue;
        }

        /*
         * Construct the path used by Attribute::itemAtPath() to locate the
         * child item.
         *
         * Verify that "/" is the ItemPath separator used by the SMTK version
         * being compiled. If SMTK exposes a path-separator constant, prefer it.
         */
        const std::string childPath = itemPath + "/" + childName;

        /*
         * Pass the parent's label as a prefix so headers appear as:
         *
         *   Boundary Type / Velocity
         *   Boundary Type / Temperature
         */
        this->appendItemDefinitionColumns(childDefinition, childPath, baseLabel.toStdString());
      }
    }

    return;
  }

  // Complex items are represented as summaries in this first version.
  m_columns.push_back(
    { ColumnKind::ItemSummary, baseLabel.toStdString(), itemPath, 0, itemDefinition });
}

QVariant qtAttributeTableModel::valueItemData(
  const smtk::attribute::ItemPtr& item,
  std::size_t element,
  int role) const
{
  /*
   * Discrete ValueItems should display the enumeration name rather than
   * exposing the underlying integer, double, or string value.
   */
  if (auto valueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item))
  {
    if (valueItem->isDiscrete())
    {
      if (element >= valueItem->numberOfValues())
      {
        return {};
      }

      if (!valueItem->isSet(element))
      {
        return role == Qt::DisplayRole ? QVariant(QStringLiteral("—")) : QVariant();
      }

      if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
        const auto* definition =
          dynamic_cast<const smtk::attribute::ValueItemDefinition*>(valueItem->definition().get());

        if (!definition)
        {
          return {};
        }

        const std::size_t discreteIndex = valueItem->discreteIndex(element);

        if (discreteIndex >= definition->numberOfDiscreteValues())
        {
          return {};
        }

        return QString::fromStdString(definition->discreteEnum(discreteIndex));
      }

      return {};
    }
  }

  /*
   * Existing non-discrete handling follows.
   */
  if (auto doubleItem = std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(item))
  {
    if (element >= doubleItem->numberOfValues())
    {
      return {};
    }

    if (!doubleItem->isSet(element))
    {
      return role == Qt::DisplayRole ? QVariant(QStringLiteral("—")) : QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      return doubleItem->value(element);
    }
  }
  else if (auto intItem = std::dynamic_pointer_cast<smtk::attribute::IntItem>(item))
  {
    if (element >= intItem->numberOfValues())
    {
      return {};
    }

    if (!intItem->isSet(element))
    {
      return role == Qt::DisplayRole ? QVariant(QStringLiteral("—")) : QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      return static_cast<qlonglong>(intItem->value(element));
    }
  }
  else if (auto stringItem = std::dynamic_pointer_cast<smtk::attribute::StringItem>(item))
  {
    if (element >= stringItem->numberOfValues())
    {
      return {};
    }

    if (!stringItem->isSet(element))
    {
      return {};
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      return QString::fromStdString(stringItem->value(element));
    }
  }

  return {};
}

bool qtAttributeTableModel::setDiscreteValue(
  const smtk::attribute::ValueItemPtr& item,
  std::size_t element,
  const QString& enumName)
{
  if (!item || !item->isDiscrete() || element >= item->numberOfValues())
  {
    return false;
  }

  const auto* definition =
    dynamic_cast<const smtk::attribute::ValueItemDefinition*>(item->definition().get());

  if (!definition)
  {
    return false;
  }

  const std::string requestedEnum = enumName.toStdString();
  /*
   * The combo box presents enumeration names. Find the corresponding
   * definition index and assign that index to the ValueItem.
   */
  for (std::size_t discreteIndex = 0; discreteIndex < definition->numberOfDiscreteValues();
       ++discreteIndex)
  {
    if (definition->discreteEnum(discreteIndex) != requestedEnum)
    {
      continue;
    }

    /*
     * Avoid generating a modification notification when the selection did
     * not actually change.
     */
    if (item->isSet(element) && (item->discreteIndex(element) == static_cast<int>(discreteIndex)))
    {
      return false;
    }

    return item->setDiscreteIndex(element, discreteIndex);
  }

  return false;
}

bool qtAttributeTableModel::setValueItemData(
  const smtk::attribute::ItemPtr& item,
  std::size_t element,
  const QVariant& value)
{
  auto valueItem = std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);

  if (!valueItem || element >= valueItem->numberOfValues())
  {
    return false;
  }

  /*
   * An invalid QVariant represents an explicit request from the editor to
   * unset this value.
   */
  if (!value.isValid())
  {
    if (!valueItem->isSet(element))
    {
      // The value is already unset, so nothing changed.
      return false;
    }

    valueItem->unset(element);
    return true;
  }

  /*
   * Discrete items are committed using their enumeration name.
   */
  if (valueItem->isDiscrete())
  {
    return this->setDiscreteValue(valueItem, element, value.toString());
  }

  if (auto doubleItem = std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(item))
  {
    if (element >= doubleItem->numberOfValues())
    {
      return false;
    }

    bool ok = false;
    const double newValue = value.toDouble(&ok);

    return ok && doubleItem->setValue(element, newValue);
  }

  if (auto intItem = std::dynamic_pointer_cast<smtk::attribute::IntItem>(item))
  {
    if (element >= intItem->numberOfValues())
    {
      return false;
    }

    bool ok = false;
    const qlonglong newValue = value.toLongLong(&ok);

    if (
      !ok || newValue < std::numeric_limits<int>::min() ||
      newValue > std::numeric_limits<int>::max())
    {
      return false;
    }

    return intItem->setValue(element, static_cast<int>(newValue));
  }

  if (auto stringItem = std::dynamic_pointer_cast<smtk::attribute::StringItem>(item))
  {
    if (element >= stringItem->numberOfValues())
    {
      return false;
    }

    return stringItem->setValue(element, value.toString().toStdString());
  }

  return false;
}

QString qtAttributeTableModel::itemToolTip(const smtk::attribute::ItemPtr& item) const
{
  if (!item || !item->definition())
  {
    return {};
  }

  const auto definition = item->definition();

  QString result = QString::fromStdString(definition->name());

  if (!definition->briefDescription().empty())
  {
    result += QStringLiteral("\n");
    result += QString::fromStdString(definition->briefDescription());
  }

  return result;
}

} // namespace extension
} // namespace smtk
