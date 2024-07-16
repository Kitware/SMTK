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
#include "smtk/attribute/Resource.h"

#include "ItemDefDataModel.h"

// ------------------------------------------------------------------------
ItemDefDataModel::ItemDefDataModel(QObject* parent)
  : AbstractDataModel(parent)
{
  this->initializeRootItem();
}

// ------------------------------------------------------------------------
void ItemDefDataModel::initializeRootItem()
{
  AbstractDataModel::RootItem = new ItemDefElement;
  AbstractDataModel::RootItem->setData(0, Qt::DisplayRole, "Name");
  AbstractDataModel::RootItem->setData(1, Qt::DisplayRole, "Type");
  AbstractDataModel::RootItem->setData(2, Qt::DisplayRole, "Inherited From");
}

// ------------------------------------------------------------------------
ItemDefDataModel::~ItemDefDataModel() = default;

// ------------------------------------------------------------------------
void ItemDefDataModel::appendBranchToRoot(smtk::attribute::DefinitionPtr def)
{
  std::vector<ItemDefPtr> const& itemDefs = def->localItemDefinitions();
  const QString& attDefType = QString::fromStdString(def->type());

  for (auto const& itemDef : itemDefs)
  {
    ItemDefElement* item = new ItemDefElement(this->RootItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(itemDef->name()));
    item->setData(
      1,
      Qt::DisplayRole,
      QString::fromStdString(smtk::attribute::Item::type2String(itemDef->type())));
    item->setData(2, Qt::DisplayRole, attDefType);
    item->setReferencedData(itemDef);

    if (itemDef->type() == smtk::attribute::Item::GroupType)
    {
      this->appendRecursively(itemDef, item, attDefType);
    }
  }
}

// ------------------------------------------------------------------------
void ItemDefDataModel::appendRecursively(
  smtk::attribute::ItemDefinitionPtr parentItemDef,
  QTreeWidgetItem* parentItem,
  const QString& attDefType)
{
  const GroupDef* group = static_cast<const GroupDef*>(parentItemDef.get());
  const size_t numItems = group->numberOfItemDefinitions();
  for (size_t i = 0; i < numItems; i++)
  {
    ItemDefPtr itemDef = group->itemDefinition(static_cast<int>(i));
    ItemDefElement* item = new ItemDefElement(parentItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(itemDef->name()));
    item->setData(
      1,
      Qt::DisplayRole,
      QString::fromStdString(smtk::attribute::Item::type2String(itemDef->type())));
    item->setData(2, Qt::DisplayRole, attDefType);
    item->setReferencedData(itemDef);

    if (itemDef->type() == smtk::attribute::Item::GroupType)
    {
      this->appendRecursively(itemDef, item, attDefType);
    }
  }
}

// ------------------------------------------------------------------------
void ItemDefDataModel::insert(const Container& props)
{
  // Attribute resource insert. Inserts into either the parent
  // GroupItemDefinition or the Definition.
  const auto itemDef = props.ItemDefinition;
  auto* const parentElement = static_cast<ItemDefElement*>(this->getItem(props.ParentIndex));
  const auto& parentItemDef = parentElement->getReferencedDataConst();
  if (parentItemDef && parentItemDef->type() == smtk::attribute::Item::GroupType)
  {
    auto group = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(parentItemDef);
    group->addItemDefinition(itemDef);
  }
  else
  {
    props.Definition->addItemDefinition(itemDef);
  }

  this->clearAttributes(props.Definition);

  // QAbstractItemModel insert.
  /// TODO insert next to the currentIndex (use its row position).
  /// TODO Also necessary to handle insertion into a GroupItemDefinition (this could be
  // handled by the Information class (just pass in a different parent)).
  const int rowIndex = parentElement->childCount();
  QAbstractItemModel::beginInsertRows(props.ParentIndex, rowIndex, rowIndex);

  ItemDefElement* elem = new ItemDefElement();
  elem->setData(0, Qt::DisplayRole, QString::fromStdString(itemDef->name()));
  elem->setData(
    1,
    Qt::DisplayRole,
    QString::fromStdString(smtk::attribute::Item::type2String(itemDef->type())));
  elem->setData(2, Qt::DisplayRole, QString::fromStdString(props.Definition->type()));
  elem->setReferencedData(itemDef);
  parentElement->insertChild(rowIndex, elem);

  QAbstractItemModel::endInsertRows();
}

// ------------------------------------------------------------------------
void ItemDefDataModel::remove(const QModelIndex& itemIndex, smtk::attribute::DefinitionPtr def)
{
  // Attribute resource remove. Removes from either the parent
  // GroupItemDefinition or the Definition.
  const QModelIndex parentIndex = itemIndex.parent();
  auto* const parentElem = static_cast<ItemDefElement*>(this->getItem(parentIndex));
  const auto& parentItemDef = parentElem->getReferencedDataConst();
  auto* const item = static_cast<ItemDefElement*>(this->getItem(itemIndex));
  const auto& itemDef = item->getReferencedDataConst();
  if (parentItemDef && parentItemDef->type() == smtk::attribute::Item::GroupType)
  {
    auto group = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(parentItemDef);
    group->removeItemDefinition(itemDef);
  }
  else
  {
    def->removeItemDefinition(itemDef);
  }

  this->clearAttributes(def);

  // QAbstractItemModel remove.
  const int row = itemIndex.row();
  AbstractDataModel::removeRows(row, 1, parentIndex);
}

// ------------------------------------------------------------------------
void ItemDefDataModel::clearAttributes(smtk::attribute::DefinitionPtr def)
{
  std::vector<smtk::attribute::AttributePtr> atts;
  auto sys = def->attributeResource();
  sys->findAttributes(def->type(), atts);
  for (const auto& att : atts)
  {
    sys->removeAttribute(att);
  }
}

// ------------------------------------------------------------------------
const smtk::attribute::ItemDefinitionPtr& ItemDefDataModel::get(const QModelIndex& index) const
{
  const QTreeWidgetItem* item = this->getItem(index);
  const ItemDefElement* element = static_cast<const ItemDefElement*>(item);

  return element->getReferencedDataConst();
}
