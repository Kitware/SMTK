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
  qtTimeZoneRegionModel(QObject* parent = nullptr);
  ~qtTimeZoneRegionModel() override;
  void initialize();

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  //virtual Qt::ItemFlags flags(const QModelIndex& modelIndex) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)
    const override;

  QString regionId(const QModelIndex& index) const;
  QModelIndex findModelIndex(const QString& region) const;

private:
  class TimeZoneRegionModelInternal;
  TimeZoneRegionModelInternal* Internal;
}; // class qtTimeZoneRegionModel
}; // namespace extension
}; // namespace smtk

#endif
