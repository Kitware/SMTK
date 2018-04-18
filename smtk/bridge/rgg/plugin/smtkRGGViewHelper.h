//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGEditPinViewHelper - Helper classes for create RGG pins
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGViewHelper_h
#define smtkRGGViewHelper_h

#include "smtk/bridge/rgg/operators/CreatePin.h"
#include "smtk/bridge/rgg/plugin/Exports.h"

#include <QTableWidgetItem>

enum RADIUSType
{
  BASE = 0,
  TOP
};
enum DIRECTION
{
  BACK = 0,
  FORWARD
};

// Item data just needs to be greater than 0
class SMTKRGGSESSIONPLUGIN_EXPORT generalItem : public QTableWidgetItem
{
public:
  generalItem() {}
  virtual ~generalItem() {}
  virtual void setData(int role, const QVariant& value);
  bool isPositive(const QVariant& value);
  smtk::bridge::rgg::RGGType getRGGType(int row);
};

// Based on type, radius would have an impact on next and previous radiusItem
// Ex. If the pin consists of a cylinder a, frustum b and another cylinder c,
// modifying the baseR of b would propogate the change to cylinder a. In nutsshell,
// any two adjacent parts should have C0 continuity.
class SMTKRGGSESSIONPLUGIN_EXPORT radiusItem : public generalItem
{
public:
  radiusItem(RADIUSType type);
  virtual ~radiusItem() {}
  virtual void setData(int role, const QVariant& value);
  void checkAndUpdateNeighbour(const int row, const DIRECTION direction, const double r);
  void setRadius(double radius) { generalItem::setData(Qt::EditRole, QVariant(radius)); }
  RADIUSType Type;
};

// Item data can only stay within a certain range where the lower bound is defined
// in the previous item and the upper bound is defined in the next item
// PS: It only applies to items within one column
class SMTKRGGSESSIONPLUGIN_EXPORT rangeItem : public generalItem
{
public:
  rangeItem() {}
  virtual ~rangeItem() {}

  virtual void setData(int role, const QVariant& value);
};

// Item data can only stay within a certain range where the lower bound is defined
// in the previous item and the upper bound is defined in the next item
// PS: It's designed for duct segments table where in a 2*2 matrix, lb is stored
// in (0, 0) and hb is stored in (1, 1)
class SMTKRGGSESSIONPLUGIN_EXPORT dSTableItem : public generalItem
{
public:
  dSTableItem() {}
  virtual ~dSTableItem() {}

  virtual void setData(int role, const QVariant& value);
};

#endif // smtkRGGViewHelper_h
