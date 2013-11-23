#include "smtk/Qt/qtEntityItemModel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"

#include <QtCore/QVariant>

QEntityItemModel::QEntityItemModel(smtk::model::StoragePtr model, QObject* parent)
  : m_storage(model), QAbstractItemModel(parent)
{
  this->m_deleteOnRemoval = true;
}

QEntityItemModel::~QEntityItemModel()
{
}

QModelIndex QEntityItemModel::index(int row, int column, const QModelIndex& parent) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
}

QModelIndex QEntityItemModel::parent(const QModelIndex& child) const
{
  return QModelIndex();
}

bool QEntityItemModel::hasChildren(const QModelIndex& parent) const
{
      return parent.isValid() ? false : (rowCount() > 0);
}

int QEntityItemModel::rowCount(const QModelIndex& parent) const
{
  return this->m_subset.size();
}

QVariant QEntityItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    {
    return QVariant();
    }
  if (orientation == Qt::Horizontal)
    {
    switch (section)
      {
    case 0:
      return "Type";
    case 1:
      return "Dimension";
    case 2:
      return "Name";
      }
    }
  // ... could add "else" here to present per-row header.
  return QVariant();
}

QVariant QEntityItemModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid() && role == Qt::DisplayRole)
    {
    int row = index.row();
    int col = index.column();
    smtk::util::UUID uid = this->m_subset[row];
    switch (col)
      {
    case 0:
        {
        smtk::model::Entity* link = this->m_storage->findEntity(uid);
        if (link)
          {
          return QVariant(smtk::model::Entity::flagSummary(link->entityFlags()).c_str());
          }
        }
      break;
    case 1:
        {
        smtk::model::Entity* link = this->m_storage->findEntity(uid);
        if (link)
          {
          return QVariant(link->dimension());
          }
        }
      break;
    case 2:
        {
        smtk::model::StringList& names(this->m_storage->stringProperty(uid, "name"));
        return QVariant(names.empty() ? "" : names[0].c_str());
        }
      break;
      }
    }
  return QVariant();
}

bool QEntityItemModel::insertRows(int position, int rows, const QModelIndex& parent)
{
  beginInsertRows(QModelIndex(), position, position + rows - 1);

  int maxPos = position + rows;
  for (int row = position; row < maxPos; ++row)
    {
    smtk::util::UUID uid = this->m_storage->addEntityOfTypeAndDimension(smtk::model::INVALID, -1);
    this->m_subset.insert(this->m_subset.begin() + row, uid);
    this->m_reverse[uid] = row;
    }

  endInsertRows();
  return true;
}

bool QEntityItemModel::removeRows(int position, int rows, const QModelIndex& parent)
{
  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  smtk::util::UUIDArray uids(this->m_subset.begin() + position, this->m_subset.begin() + position + rows);
  this->m_subset.erase(this->m_subset.begin() + position, this->m_subset.begin() + position + rows);
  int maxPos = position + rows;
  for (smtk::util::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
    {
    this->m_reverse.erase(this->m_reverse.find(*it));
    if (this->m_deleteOnRemoval)
      {
      this->m_storage->removeEntity(*it);
      // FIXME: Should we keep a set of all it->second.relations()
      //        and emit "didChange" events for all the relations
      //        that were affected by removal of this entity?
      }
    }

  endRemoveRows();
  return true;
}

bool QEntityItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  bool didChange = false;
  if(index.isValid() && role == Qt::EditRole)
    {
    smtk::model::Entity* entity;
    int row = index.row();
    int col = index.column();
    switch (col)
      {
    case 0:
      entity = this->m_storage->findEntity(this->m_subset[row]);
      if (entity)
        {
        didChange = entity->setEntityFlags(value.value<int>());
        }
      break;
    case 1:
      // FIXME: No way to change dimension yet.
      break;
    case 2:
      this->m_storage->setStringProperty(
        this->m_subset[row], "name",
        value.value<QString>().toStdString());
      didChange = true;
      break;
      }
    if (didChange)
      {
      emit(dataChanged(index, index));
      }
    }
  return didChange;
}

Qt::ItemFlags QEntityItemModel::flags(const QModelIndex& index) const
{
  if(!index.isValid())
    return Qt::ItemIsEnabled;

  // TODO: Check to make sure column is not "information-only".
  //       We don't want to allow people to randomly edit an enum string.
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}
