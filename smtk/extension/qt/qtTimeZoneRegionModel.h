//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtTimeZoneRegionModel - Qt item model for time zone regions
// .SECTION Description
// .SECTION See Also
// qtItem

// This class might be better implemented as a singleton, but it is not
// clear that the added complexity (for thread-safe use) is justified.

#ifndef __smtk_extension_qtTimeZoneRegionModel_h
#define __smtk_extension_qtTimeZoneRegionModel_h

#include "smtk/extension/qt/Exports.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <QVariant>

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT qtTimeZoneRegionModel : public QAbstractItemModel
    {
      Q_OBJECT

    public:
      qtTimeZoneRegionModel(QObject *parent = NULL);
      virtual ~qtTimeZoneRegionModel();
      void initialize();

      virtual QModelIndex index(
        int row, int column, const QModelIndex& parent = QModelIndex()) const;
      virtual QModelIndex parent(const QModelIndex& index) const;
      virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
      virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
      virtual QVariant data(
        const QModelIndex& index, int role = Qt::DisplayRole) const;
      //virtual Qt::ItemFlags flags(const QModelIndex& modelIndex) const;
      virtual QVariant headerData(
        int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

      QString regionId(const QModelIndex& index) const;
      QModelIndex findModelIndex(const QString& region) const;
    private:
      class TimeZoneRegionModelInternal;
      TimeZoneRegionModelInternal *Internal;
    }; // class qtTimeZoneRegionModel
  }; // namespace extension
}; // namespace smtk

#endif
