//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkRGGViewHelper.h"

#include <QComboBox>
#include <QVariant>
#include <iostream>

using namespace smtk::bridge::rgg;

void generalItem::setData(int role, const QVariant& value)
{
  if (this->tableWidget() != nullptr && role == Qt::EditRole)
  {
    if (!this->isPositive(value))
    {
      return;
    }
  }
  QTableWidgetItem::setData(role, value);
}

bool generalItem::isPositive(const QVariant& value)
{
  double vDouble = value.toDouble();
  if (vDouble <= 0.0)
  {
    return false;
  }
  else
  {
    return true;
  }
}
smtk::bridge::rgg::RGGType generalItem::getRGGType(int row)
{
  QTableWidget* pt = this->tableWidget();
  QWidget* tmpWidget = pt->cellWidget(row, 0);
  QComboBox* comboBox = dynamic_cast<QComboBox*>(tmpWidget);
  int index = comboBox->currentIndex();
  return static_cast<RGGType>(index);
}

radiusItem::radiusItem(RADIUSType type)
  : Type(type)
{
}

void radiusItem::setData(int role, const QVariant& value)
{
  if (this->tableWidget() != nullptr && role == Qt::EditRole)
  {
    if (!this->isPositive(value))
    {
      return;
    }
    // Check and update neighbours
    // Based on radius type, this function would update radius in current row(
    // baseR, topR) then use the recursive function checkAndUpdateNeighbour to
    // update forward and backward neighbours accordingly.
    int row(this->row()), column(this->column());
    bool back(false), forward(false);
    if (this->getRGGType(row) == RGGType::FRUSTUM)
    { // We only care about one direction
      back = (this->Type == RADIUSType::BASE) ? true : false;
      forward = (this->Type == RADIUSType::BASE) ? false : true;
    }
    else if (this->getRGGType(row) == RGGType::CYLINDER)
    { // We need to propograte back and forward
      back = true;
      forward = true;
      // Handle current row
      int otherRaidusCol = column + ((this->Type == RADIUSType::BASE) ? 1 : -1);
      radiusItem* item = dynamic_cast<radiusItem*>(this->tableWidget()->item(row, otherRaidusCol));
      item->setRadius(value.toDouble());
    }
    double vDouble = value.toDouble();
    std::cout << "at "
              << "row=" << row << " col=" << column << " back=" << back << " forward=" << forward
              << std::endl;
    if (back)
    {
      this->checkAndUpdateNeighbour(--row, DIRECTION::BACK, vDouble);
    }
    if (forward)
    {
      this->checkAndUpdateNeighbour(++row, DIRECTION::FORWARD, vDouble);
    }
  }
  QTableWidgetItem::setData(role, value);
}

void radiusItem::checkAndUpdateNeighbour(const int row, const DIRECTION direction, const double r)
{
  std::cout << "checkAndUpdateNeighbour, row=" << row << std::endl;
  QTableWidget* pt = this->tableWidget();
  bool isBack = (direction == DIRECTION::BACK);
  int bound = (isBack ? 0 : (pt->rowCount() - 1));
  // base condition
  if (row < 0 || row >= pt->rowCount())
  {
    return;
  }
  if (this->getRGGType(row) == RGGType::FRUSTUM)
  {
    int col = isBack ? 3 : 2; // Update the top radius if it's back
    radiusItem* item = dynamic_cast<radiusItem*>(this->tableWidget()->item(row, col));
    item->setRadius(r);
    return;
  }
  if (this->getRGGType(row) == CYLINDER)
  { // Update both top and back radius
    radiusItem* backRItem = dynamic_cast<radiusItem*>(this->tableWidget()->item(row, 3));
    backRItem->setRadius(r);

    radiusItem* topRItem = dynamic_cast<radiusItem*>(this->tableWidget()->item(row, 2));
    topRItem->setRadius(r);
    if (row == bound)
    {
      return;
    }
  }
  // Recursive call
  int nextRow = row + (isBack ? -1 : 1);
  this->checkAndUpdateNeighbour(nextRow, direction, r);
}

void rangeItem::setData(int role, const QVariant& value)
{
  if (this->tableWidget() != nullptr && role == Qt::EditRole)
  {
    // Fetch neghbours' bound then check current value
    QTableWidget* widget = this->tableWidget();
    double lb(0), ub(1);
    int rowNum = widget->rowCount(), row(this->row()), col(this->column());
    if (row > 0)
    {
      rangeItem* beforeItem = dynamic_cast<rangeItem*>(widget->item(row - 1, col));
      if (beforeItem)
      {
        lb = beforeItem->text().toDouble();
      }
    }
    if (row < (rowNum - 1))
    {
      rangeItem* afterItem = dynamic_cast<rangeItem*>(widget->item(row - 1, col));
      if (afterItem)
      {
        ub = afterItem->text().toDouble();
      }
    }
    double dValue = value.toDouble();
    // std::cout << "Test value=" <<dValue << " at row=" << row <<
    // " col=" << col << " lb=" << lb << " ub=" << ub <<std::endl;
    if (dValue <= lb || dValue >= ub)
    {
      return;
    }
  }
  generalItem::setData(role, value);
}
