//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QTreeWidgetItem>

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"

#include "ItemDefinitionHelper.h"
#include "ItemDefinitionsDataModel.h"

// ------------------------------------------------------------------------
ItemDefinitionsDataModel::ItemDefinitionsDataModel(QObject* parent)
  : AbstractDataModel(parent)
{
  this->initializeRootItem();
}

// ------------------------------------------------------------------------
void ItemDefinitionsDataModel::initializeRootItem()
{
  AbstractDataModel::RootItem = new ItemDefElement;
  AbstractDataModel::RootItem->setData(0, Qt::DisplayRole, "Name");
  AbstractDataModel::RootItem->setData(1, Qt::DisplayRole, "Type");
  AbstractDataModel::RootItem->setData(2, Qt::DisplayRole, "Inherited From");
}

// ------------------------------------------------------------------------
ItemDefinitionsDataModel::~ItemDefinitionsDataModel() = default;

// ------------------------------------------------------------------------
void ItemDefinitionsDataModel::appendBranchToRoot(smtk::attribute::DefinitionPtr def)
{
  const size_t numItems = def->numberOfItemDefinitions();
  std::vector<ItemDefPtr> const& itemDefs = def->localItemDefinitions();
  const QString& attDefType = QString::fromStdString(def->type());

  for (auto const& itemDef : itemDefs)
  {
    ItemDefElement* item = new ItemDefElement(this->RootItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(itemDef->name()));
    item->setData(1, Qt::DisplayRole,
      QString::fromStdString(smtk::attribute::Item::type2String(itemDef->type())));
    item->setData(2, Qt::DisplayRole, attDefType);
    item->setReferencedData(itemDef);

    if (itemDef->type() == smtk::attribute::Item::GROUP)
    {
      this->appendRecursively(itemDef, item, attDefType);
    }
  }
}

// ------------------------------------------------------------------------
void ItemDefinitionsDataModel::appendRecursively(smtk::attribute::ItemDefinitionPtr parentItemDef,
  QTreeWidgetItem* parentItem, const QString& attDefType)
{
  const GroupDef* group = static_cast<const GroupDef*>(parentItemDef.get());
  const size_t numItems = group->numberOfItemDefinitions();
  for (size_t i = 0; i < numItems; i++)
  {
    ItemDefPtr itemDef = group->itemDefinition(i);
    ItemDefElement* item = new ItemDefElement(parentItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(itemDef->name()));
    item->setData(1, Qt::DisplayRole,
      QString::fromStdString(smtk::attribute::Item::type2String(itemDef->type())));
    item->setData(2, Qt::DisplayRole, attDefType);
    item->setReferencedData(itemDef);

    if (itemDef->type() == smtk::attribute::Item::GROUP)
    {
      this->appendRecursively(itemDef, item, attDefType);
    }
  }
}

// ------------------------------------------------------------------------
void ItemDefinitionsDataModel::insertItem(ItemDefProperties const& props)
{
  const auto itemDef = ItemDefinitionHelper::create(
    props.Definition, smtk::attribute::Item::string2Type(props.Type), props.Name);

  const auto parentElement = static_cast<ItemDefElement*>(this->getItem(props.ParentNode));
  const auto& parentItemDef = parentElement->getReferencedDataConst();
  if (parentItemDef && parentItemDef->type() == smtk::attribute::Item::GROUP)
  {
    auto group = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(parentItemDef);
    group->addItemDefinition(itemDef);
  }
  else
  {
    props.Definition->addItemDefinition(itemDef);
  }

  this->clearAttributes(props.Definition);

  /// TODO insert next to the currentIndex (use its row position)
  const int rowIndex = parentElement->childCount();
  QAbstractItemModel::beginInsertRows(props.ParentNode, rowIndex, rowIndex);

  ItemDefElement* elem = new ItemDefElement();
  elem->setData(0, Qt::DisplayRole, QString::fromStdString(props.Name));
  elem->setData(1, Qt::DisplayRole, QString::fromStdString(props.Type));
  elem->setData(2, Qt::DisplayRole, QString::fromStdString(props.Definition->type()));
  elem->setReferencedData(itemDef);
  parentElement->insertChild(rowIndex, elem); /* sets item's parent */

  QAbstractItemModel::endInsertRows();
}

// ------------------------------------------------------------------------
void ItemDefinitionsDataModel::removeItem(
  const QModelIndex& itemIndex, smtk::attribute::DefinitionPtr def)
{
  const QModelIndex parentIndex = itemIndex.parent();

  const auto parentElem = static_cast<ItemDefElement*>(this->getItem(parentIndex));
  const auto& parentItemDef = parentElem->getReferencedDataConst();
  const auto item = static_cast<ItemDefElement*>(this->getItem(itemIndex));
  auto itemDef = item->getReferencedDataConst();
  if (parentItemDef && parentItemDef->type() == smtk::attribute::Item::GROUP)
  {
    auto group = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(parentItemDef);
    group->removeItemDefinition(itemDef);
  }
  else
  {
    def->removeItemDefinition(itemDef);
  }

  /// TODO share this definition through the base class

  /// TODO handle issue:  itemDef-changed -> view-isInvalid -> required to
  // invalidate/destroy view or auto-update.

  this->clearAttributes(def);

  const int row = itemIndex.row();
  QAbstractItemModel::beginRemoveRows(parentIndex, row, row);

  auto parent = this->getItem(parentIndex);
  parent->removeChild(item);

  QAbstractItemModel::endRemoveRows();
}

// ------------------------------------------------------------------------
void ItemDefinitionsDataModel::clearAttributes(smtk::attribute::DefinitionPtr def)
{
  std::vector<smtk::attribute::AttributePtr> atts;
  auto sys = def->system();
  sys->findAttributes(def->type(), atts);
  for (const auto& att : atts)
  {
    sys->removeAttribute(att);
  }
}

// ------------------------------------------------------------------------
const smtk::attribute::ItemDefinitionPtr& ItemDefinitionsDataModel::getItemDef(
  const QModelIndex& index) const
{
  const QTreeWidgetItem* item = this->getItem(index);
  const ItemDefElement* element = static_cast<const ItemDefElement*>(item);

  return element->getReferencedDataConst();
}
