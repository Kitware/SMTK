//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME jobtablemodel.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_jobtablemodel_h
#define __smtk_extension_cumulus_jobtablemodel_h

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>

#include "cJSON.h"
#include "job.h"

namespace cumulus
{

class JobTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  enum ColumnNames {
    JOB_ID = 0,
    MACHINE,
    JOB_NAME ,
    JOB_STATUS,
    COLUMN_COUNT
  };

  explicit JobTableModel(QObject *parentObject = 0);

  QModelIndex parent(const QModelIndex &) const {return QModelIndex();}

  int rowCount(const QModelIndex & theModelIndex = QModelIndex()) const;
  int columnCount(const QModelIndex & modelIndex = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  QVariant data(const QModelIndex & modelIndex, int role = Qt::DisplayRole) const;

  bool removeRows(int row, int count, const QModelIndex &);

  bool insertRows(int row, int count, const QModelIndex &);

signals:
  void rowCountChanged();

public slots:
  void jobsUpdated(QList<Job> jobs);

private:
  QList<Job> m_jobs;
};

} // end namespace

#endif
