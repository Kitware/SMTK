//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtExpressionSortFilterProxyModel.h"

using namespace smtk::extension;

qtExpressionSortFilterProxyModel::qtExpressionSortFilterProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
{
}

bool qtExpressionSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right)
  const
{
  // Ensures that the default options "Please Select" and "Create..." are always at the top, and the list remains ordered
  if (isIgnored(left))
  {
    return !(isIgnored(right));
  }

  if (isIgnored(right))
  {
    return false;
  }

  // Continue with base implementation
  return QSortFilterProxyModel::lessThan(left, right);
}

bool qtExpressionSortFilterProxyModel::filterAcceptsRow(
  int source_row,
  const QModelIndex& source_parent) const
{
  // If the row is 0 (ie "Please Select") or the current selected expression or row 1 if expression creation is enabled
  // those rows are always a part of the filter
  if (source_row == 0 || (source_row == 1 && m_createExpression) || source_row == m_currentRow)
  {
    return true;
  }

  // Continue with base implementation
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool qtExpressionSortFilterProxyModel::isIgnored(const QModelIndex& index) const
{
  // Indicates whether this index is one of the defaults
  QString value = index.data(Qt::DisplayRole).toString();
  return (value == "Please Select" || value == "Create...");
}

void qtExpressionSortFilterProxyModel::setCurrentRow(int row)
{
  m_currentRow = row;
}

void qtExpressionSortFilterProxyModel::enableExpressionCreation(bool create)
{
  m_createExpression = create;
}
