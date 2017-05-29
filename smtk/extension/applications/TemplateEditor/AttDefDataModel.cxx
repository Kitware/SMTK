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
#include "smtk/attribute/System.h"

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
void AttDefDataModel::populate(smtk::attribute::SystemPtr system)
{
  this->System = system;
  DefinitionPtrVec defs;
  this->System->findBaseDefinitions(defs);

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
  smtk::attribute::DefinitionPtr parentDef, QTreeWidgetItem* parentItem)
{
  DefinitionPtrVec defsConcrete;
  this->System->derivedDefinitions(parentDef, defsConcrete);

  for (auto const& def : defsConcrete)
  {
    AttDefElement* item = new AttDefElement(parentItem);
    item->setData(0, Qt::DisplayRole, QString::fromStdString(def->type()));
    item->setReferencedData(def);

    this->appendRecursively(def, item);
  }
}

// -----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr const& AttDefDataModel::getAttDef(const QModelIndex& index) const
{
  const QTreeWidgetItem* item = this->getItem(index);
  const AttDefElement* element = static_cast<const AttDefElement*>(item);

  return element->getReferencedDataConst();
}

// -----------------------------------------------------------------------------
bool AttDefDataModel::hasDerivedTypes(const QModelIndex& index) const
{
  auto def = this->getAttDef(index);

  DefinitionPtrVec defVec;
  def->system()->derivedDefinitions(def, defVec);
  return defVec.size() > 0;
}

// -----------------------------------------------------------------------------
void AttDefDataModel::addAttDef(DefProperties const& props)
{
  smtk::attribute::DefinitionPtr newDef =
    this->System->createDefinition(props.Type, props.BaseType);

  newDef->setIsUnique(props.IsUnique);
  newDef->setIsAbstract(props.IsAbstract);
  newDef->setLabel(props.Label);

  // An empty QModelIndex() means insertion in the root node.
  const auto dataMatch = newDef->baseDefinition();
  const auto parentIndex =
    props.BaseType.empty() ? QModelIndex() : this->findElementByData(this->RootItem, dataMatch);

  const auto parentItem = this->getItem(parentIndex);
  const int rowIndex = parentItem->childCount();

  QAbstractItemModel::beginInsertRows(parentIndex, rowIndex, rowIndex);

  AttDefElement* item = new AttDefElement();
  item->setData(0, Qt::DisplayRole, QString::fromStdString(props.Type));
  item->setReferencedData(newDef);
  parentItem->insertChild(rowIndex, item); /* sets item's parent */

  QAbstractItemModel::endInsertRows();
}

// -----------------------------------------------------------------------------
void AttDefDataModel::removeAttDef(const QModelIndex& attDefIndex)
{
  const int row = attDefIndex.row();
  const QModelIndex parentIndex = attDefIndex.parent();

  auto child = static_cast<AttDefElement*>(this->getItem(attDefIndex));
  if (!this->System->removeDefinition(child->getReferencedDataConst()))
  {
    return;
  }

  QAbstractItemModel::beginRemoveRows(parentIndex, row, row);

  QTreeWidgetItem* parent = this->getItem(parentIndex);
  parent->removeChild(child);

  QAbstractItemModel::endRemoveRows();
}

// -----------------------------------------------------------------------------
QModelIndex AttDefDataModel::findElementByData(
  QTreeWidgetItem* parent, const smtk::attribute::DefinitionPtr& dataMatch)
{
  const int count = parent->childCount();
  for (int i = 0; i < count; i++)
  {
    const auto childItem = static_cast<AttDefElement*>(parent->child(i));
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
QModelIndex AttDefDataModel::findItemByType(QTreeWidgetItem* parent, const std::string& type)
{
  const int count = parent->childCount();
  for (int i = 0; i < count; i++)
  {
    const auto childItem = parent->child(i);

    const std::string childType = childItem->data(0, Qt::DisplayRole).toString().toStdString();
    if (childType == type)
    {
      return QAbstractItemModel::createIndex(parent->indexOfChild(childItem), 0, childItem);
    }

    const QModelIndex index = this->findItemByType(childItem, type);
    if (index.isValid())
    {
      return index;
    }
  }

  return QModelIndex();
}

// -----------------------------------------------------------------------------
const std::string AttDefDataModel::getAttDefType(const QModelIndex& index) const
{
  return this->data(index, Qt::DisplayRole).toString().toStdString();
}
