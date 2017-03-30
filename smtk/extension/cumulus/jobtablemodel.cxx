//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "cJSON.h"
#include "job.h"
#include <jobtablemodel.h>

#include <QtCore/QDebug>

namespace cumulus
{

JobTableModel::JobTableModel(QObject* parentObject)
  : QAbstractTableModel(parentObject)
{
}

int JobTableModel::rowCount(const QModelIndex& modelIndex) const
{
  if (!modelIndex.isValid())
    return m_jobs.size();
  else
    return 0;
}

int JobTableModel::columnCount(const QModelIndex&) const
{
  return COLUMN_COUNT;
}

QVariant JobTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
      case JOB_ID:
        return QVariant("Job Id");
      case MACHINE:
        return QVariant("Machine");
      case JOB_NAME:
        return QVariant("Job Name");
      case JOB_STATUS:
        return QVariant("Status");
      default:
        return QVariant();
    }
  }

  return QVariant();
}

QVariant JobTableModel::data(const QModelIndex& modelIndex, int role) const
{
  if (!modelIndex.isValid() || modelIndex.column() + 1 > COLUMN_COUNT)
    return QVariant();

  Job job = m_jobs[modelIndex.row()];

  if (role == Qt::DisplayRole)
  {
    switch (modelIndex.column())
    {
      case JOB_ID:
        return QVariant(job.id());
      case MACHINE:
        return QVariant(job.machine());
      case JOB_NAME:
        return QVariant(job.name());
      case JOB_STATUS:
        return QVariant(job.status());
      default:
        return QVariant();
    }
  }
  else if (role == Qt::UserRole)
  {
    return QVariant::fromValue(job);
  }
  else
  {
    return QVariant();
  }
}

bool JobTableModel::removeRows(int row, int count, const QModelIndex&)
{
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  endRemoveRows();
  return true;
}

bool JobTableModel::insertRows(int row, int count, const QModelIndex&)
{
  beginInsertRows(QModelIndex(), row, row + count - 1);
  endInsertRows();
  return true;
}

void JobTableModel::jobsUpdated(QList<Job> jobs)
{
  beginResetModel();
  m_jobs = jobs;
  endResetModel();
}

} // end namespace
