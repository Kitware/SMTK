//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#pragma once

#include <QSortFilterProxyModel>

#include "smtk/extension/qt/Exports.h"

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtExpressionSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit qtExpressionSortFilterProxyModel(QObject* parent = nullptr);
  void setCurrentRow(int row);
  void enableExpressionCreation(bool create);

protected:
  bool isIgnored(const QModelIndex& index) const;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  int m_currentRow = -1;
  bool m_createExpression = false;
};

} // namespace extension
} // namespace smtk
