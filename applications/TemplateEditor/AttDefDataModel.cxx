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
#include "smtk/attribute/Resource.h"

#include "AttDefDataModel.h"

// -----------------------------------------------------------------------------
AttDefDataModel::AttDefDataModel(QObject* parent)
  : AbstractDataModel(parent)
{
  this->initializeRootItem();
}

// -----------------------------------------------------------------------------
AttDefDataModel::~AttDefDataModel() = default;

// -----------------------------------------------------------------------------
void AttDefDataModel::initializeRootItem()
{
  AbstractDataModel::RootItem = new AttDefElement;
  AbstractDataModel::RootItem->setData(0, Qt::DisplayRole, "Type");
  //AbstractDataModel::RootItem->setData(1, Qt::DisplayRole, "Included In");
}

// -----------------------------------------------------------------------------
void AttDefDataModel::populate(smtk::attribute::ResourcePtr resource)
{
  this->Resource = resource;
  DefinitionPtrVec defs;
  this->Resource->findBaseDefinitions(defs);

  for (auto const& def : defs)
  {
    AttDefElement* item = new AttDefElement(this->RootItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(def->type()));
    item->setReferencedData(def);

    this->appendRecursively(def, item);
  }
}

// -----------------------------------------------------------------------------
void AttDefDataModel::appendRecursively(
  smtk::attribute::DefinitionPtr parentDef,
  QTreeWidgetItem* parentItem)
{
  DefinitionPtrVec defsConcrete;
  this->Resource->derivedDefinitions(parentDef, defsConcrete);

  for (auto const& def : defsConcrete)
  {
    AttDefElement* item = new AttDefElement(parentItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(def->type()));
    item->setReferencedData(def);

    this->appendRecursively(def, item);
  }
}

// -----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr const& AttDefDataModel::get(const QModelIndex& index) const
{
  const QTreeWidgetItem* item = this->getItem(index);
  const AttDefElement* element = static_cast<const AttDefElement*>(item);

  return element->getReferencedDataConst();
}

// -----------------------------------------------------------------------------
bool AttDefDataModel::hasDerivedTypes(const QModelIndex& index) const
{
  auto def = this->get(index);

  DefinitionPtrVec defVec;
  def->attributeResource()->derivedDefinitions(def, defVec);
  return !defVec.empty();
}

// -----------------------------------------------------------------------------
void AttDefDataModel::insert(const AttDefContainer& props)
{
  // Attribute resource insert.
  smtk::attribute::DefinitionPtr newDef =
    this->Resource->createDefinition(props.Type, props.BaseType);

  newDef->setIsUnique(props.IsUnique);
  newDef->setIsAbstract(props.IsAbstract);
  newDef->setLabel(props.Label);

  const auto dataMatch = newDef->baseDefinition();
  // TODO Instead of findElementByData, just include the current QModelIndex in
  // props, this way the insertion point (parent) is already known.
  const auto parentIndex =
    props.BaseType.empty() ? QModelIndex() : this->findElementByData(this->RootItem, dataMatch);

  auto* const parentItem = this->getItem(parentIndex);
  const int rowIndex = parentItem->childCount();

  // QAbstractItemModel insert.
  QAbstractItemModel::beginInsertRows(parentIndex, rowIndex, rowIndex);

  AttDefElement* item = new AttDefElement();
  item->setData(0, Qt::DisplayRole, QString::fromStdString(props.Type));
  item->setReferencedData(newDef);
  parentItem->insertChild(rowIndex, item); /* sets item's parent */

  QAbstractItemModel::endInsertRows();
}

// -----------------------------------------------------------------------------
void AttDefDataModel::remove(const QModelIndex& attDefIndex)
{
  // Attribute resource remove.
  const QModelIndex parentIndex = attDefIndex.parent();
  auto* const child = static_cast<AttDefElement*>(this->getItem(attDefIndex));
  if (!this->Resource->removeDefinition(child->getReferencedDataConst()))
  {
    return;
  }

  // QAbstractItemModel remove.
  const int row = attDefIndex.row();
  AbstractDataModel::removeRows(row, 1, parentIndex);
}

// -----------------------------------------------------------------------------
QModelIndex AttDefDataModel::findElementByData(
  QTreeWidgetItem* parent,
  const smtk::attribute::DefinitionPtr& dataMatch)
{
  const int count = parent->childCount();
  for (int i = 0; i < count; i++)
  {
    auto* const childItem = static_cast<AttDefElement*>(parent->child(i));
    const smtk::attribute::DefinitionPtr& data = childItem->getReferencedDataConst();
    if (data == dataMatch)
    {
      return QAbstractItemModel::createIndex(parent->indexOfChild(childItem), 0, childItem);
    }

    const QModelIndex index = this->findElementByData(childItem, dataMatch);
    if (index.isValid())
    {
      return index;
    }
  }

  return QModelIndex();
}

// -----------------------------------------------------------------------------
const std::string AttDefDataModel::getType(const QModelIndex& index) const
{
  return this->data(index, Qt::DisplayRole).toString().toStdString();
}
