//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAttributeTableModel_h
#define smtk_extension_qtAttributeTableModel_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"

#include <QAbstractTableModel>
#include <QStringList>

#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace extension
{

class qtUIManager; // Needed to access color information
/**
 * @brief Adapts a collection of SMTK attributes to Qt's table-model API.
 *
 * The model does not copy the values stored by the attributes. Calls to
 * data() read directly from SMTK items, and calls to setData() modify those
 * items directly.
 *
 * All attributes are expected to share a common definition so that a single
 * stable set of table columns can be constructed.
 */
class qtAttributeTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  enum class ColumnDisplay
  {
    All,
    TopLevelNonGroup,
    TopLevelDiscrete,
    UserSpecified
  };

  enum class ColumnKind
  {
    AttributeName,
    ItemValue,
    ItemEnabledState,
    ItemSummary
  };

  struct ColumnDescriptor
  {
    /// Describes how the table should interpret and edit this column.
    ColumnKind Kind{ ColumnKind::ItemValue };

    /// Text displayed by the horizontal table header.
    std::string Label;

    /**
   * Path used to retrieve the corresponding item from an attribute.
   *
   * A path is used instead of an item name so nested items can eventually be
   * supported without changing the model's column representation.
   */
    std::string ItemPath;

    /**
   * Value index represented by this column.
   *
   * For example, a three-component DoubleItem may be represented by three
   * columns whose Element values are 0, 1, and 2.
   */
    std::size_t Element{ 0 };

    /// Definition that describes the item's type, constraints, and metadata.
    smtk::attribute::ItemDefinitionPtr Definition;
  };

  using AttributeModifiedCallback = std::function<void(const smtk::attribute::AttributePtr&)>;

  explicit qtAttributeTableModel(QObject* parent = nullptr);
  ~qtAttributeTableModel() override = default;

  void setAttributeResource(const smtk::attribute::ResourcePtr& resource);

  smtk::attribute::ResourcePtr attributeResource() const;

  void setDefinition(const smtk::attribute::DefinitionPtr& definition);

  smtk::attribute::DefinitionPtr definition() const;

  void setAttributes(const std::vector<smtk::attribute::AttributePtr>& attributes);

  const std::vector<smtk::attribute::AttributePtr>& attributes() const;

  void setAttributeModifiedCallback(AttributeModifiedCallback callback);

  /**
   * @brief Set which item definitions should be represented by table columns.
   *
   * User-specified paths use the same slash-separated syntax as ItemPath.
   * The attribute-name column is always present.
   */
  void setColumnDisplay(ColumnDisplay display, const std::set<std::string>& itemPaths = {});

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)
    const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  smtk::attribute::AttributePtr attributeForRow(int row) const;

  smtk::attribute::ItemPtr itemForIndex(const QModelIndex& index) const;

  const ColumnDescriptor* columnDescriptor(int column) const;

  void refreshAttribute(const smtk::attribute::AttributePtr& attribute);

  /**
   * @brief Return the item definition represented by a table index.
   *
   * The index must belong to this source model. Proxy indices should be mapped
   * to the source model before calling this method.
   */
  smtk::attribute::ItemDefinitionPtr itemDefinitionForIndex(const QModelIndex& index) const;

  /**
   * @brief Set the UI Manager
   *
   * This is needed in order to access colors for invalid and
   * default values
   */
  void setUIManager(qtUIManager* man) { m_uiManager = man; }

  /**
   * @brief Return true if the ValueItem element represented by this index
   * currently has an explicitly set value.
   */
  bool isValueSet(const QModelIndex& index) const;

  /**
   * @brief Return true if the item represented by an index is a discrete
   * ValueItem.
   */
  bool isDiscrete(const QModelIndex& index) const;

  /**
   * @brief Return the enumeration names currently available for a discrete item.
   *
   * These names are used by the table delegate to populate a QComboBox.
   */
  QStringList discreteValues(const QModelIndex& index) const;

  /**
   * @brief Return the currently selected discrete enumeration name.
   */
  QString currentDiscreteValue(const QModelIndex& index) const;

  /**
   * @brief Return true if the item represented by the index is currently active.
   *
   * An item can be inactive when it is a conditional child of a discrete
   * ValueItem whose current enumeration does not activate the child.
   */
  bool isItemActive(const QModelIndex& index) const;

private:
  void rebuildColumns();

  void appendItemDefinitionColumns(
    const smtk::attribute::ItemDefinitionPtr& itemDefinition,
    const std::string& itemPath,
    const std::string& labelPrefix);

  QVariant valueItemData(const smtk::attribute::ItemPtr& item, std::size_t element, int role) const;

  bool setValueItemData(
    const smtk::attribute::ItemPtr& item,
    std::size_t element,
    const QVariant& value);

  QString itemToolTip(const smtk::attribute::ItemPtr& item) const;

  /**
   * @brief Set a discrete ValueItem using its enumeration name.
   */
  bool setDiscreteValue(
    const smtk::attribute::ValueItemPtr& item,
    std::size_t element,
    const QString& enumName);

  /**
   * @brief Return true if the item and all conditional parents are active.
   */
  bool isItemActive(const smtk::attribute::ItemPtr& item) const;
  smtk::attribute::ResourcePtr m_resource;

  smtk::attribute::DefinitionPtr m_definition;

  std::vector<smtk::attribute::AttributePtr> m_attributes;
  std::vector<ColumnDescriptor> m_columns;
  ColumnDisplay m_columnDisplay{ ColumnDisplay::All };
  std::set<std::string> m_columnItemPaths;

  AttributeModifiedCallback m_attributeModified;
  smtk::extension::qtUIManager* m_uiManager;
};

} // namespace extension
} // namespace smtk

#endif
