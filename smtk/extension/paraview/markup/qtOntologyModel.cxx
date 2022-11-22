//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/markup/qtOntologyModel.h"

#include "smtk/markup/ontology/Source.h"

#include "smtk/io/Logger.h"

#include <QVariant>

#include <vector>

class qtOntologyModel::Internal
{
public:
  Internal(qtOntologyModel* self, const smtk::markup::ontology::Source& source)
    : m_self(self)
    , m_data(source.classes().begin(), source.classes().end())
  {
  }

  // A reference to the "parent" of this internal object.
  qtOntologyModel* m_self;
  // Rather than keep a raw (and possibly stale) pointer to the ontology source
  // around, just copy the portion we need.
  std::vector<smtk::markup::ontology::Identifier> m_data;
};

qtOntologyModel::qtOntologyModel(const smtk::markup::ontology::Source& source, QObject* parent)
  : QAbstractItemModel(parent)
{
  m_p = new Internal(this, source);
}

qtOntologyModel::~qtOntologyModel()
{
  delete m_p;
}

QVariant qtOntologyModel::data(const QModelIndex& index, int role) const
{
  QVariant result;
  if (index.row() < 0 || static_cast<std::size_t>(index.row()) >= m_p->m_data.size())
  // We don't need this as the switch statements below handle it:
  // index.column() < 0 || index.column() >= qtOntologyModel::Column::Count)
  {
    return result;
  }

  switch (role)
  {
    case Qt::EditRole: // fall through
    case Qt::DisplayRole:
      switch (index.column())
      {
        case static_cast<int>(qtOntologyModel::Column::Name):
          result = QString::fromStdString(m_p->m_data[index.row()].name);
          break;
        case static_cast<int>(qtOntologyModel::Column::URL):
          result = QString::fromStdString(m_p->m_data[index.row()].url);
          break;
        case static_cast<int>(qtOntologyModel::Column::Base):
          result = QString::fromStdString(m_p->m_data[index.row()].base);
          break;
        case static_cast<int>(qtOntologyModel::Column::Description):
          result = QString::fromStdString(m_p->m_data[index.row()].description);
          break;
        default:
          break;
      }
      break;
    default:
      // Do nothing.
      break;
  };
  return result;
}

int qtOntologyModel::columnCount(const QModelIndex& parent) const
{
  (void)parent;
  return static_cast<int>(qtOntologyModel::Column::Count);
}

int qtOntologyModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_p->m_data.size());
}

QModelIndex qtOntologyModel::index(int row, int column, const QModelIndex& parent) const
{
  if (
    parent.isValid() || row < 0 || row >= this->rowCount(QModelIndex()) || column < 0 ||
    column >= this->columnCount(QModelIndex()))
  {
    return QModelIndex(); // No cells have children.
  }
  return this->createIndex(row, column);
}

QModelIndex qtOntologyModel::parent(const QModelIndex& index) const
{
  (void)index;
  // Every item is a direct child of the root node.
  return QModelIndex();
}
