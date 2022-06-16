//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/properties/CoordinateFrame.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/resource/PersistentObject.h"

using smtk::attribute::ConstGroupItemPtr;
using smtk::attribute::DoubleItem;
using smtk::attribute::ReferenceItem;

namespace smtk
{
namespace resource
{
namespace properties
{

CoordinateFrame::CoordinateFrame(
  const smtk::attribute::ConstGroupItemPtr& groupItem,
  const std::string& originName,
  const std::string& xAxisName,
  const std::string& yAxisName,
  const std::string& zAxisName,
  const std::string& parentName)
{
  this->initializeFrom(groupItem.get(), originName, xAxisName, yAxisName, zAxisName, parentName);
}

CoordinateFrame::CoordinateFrame(
  const smtk::attribute::GroupItemPtr& groupItem,
  const std::string& originName,
  const std::string& xAxisName,
  const std::string& yAxisName,
  const std::string& zAxisName,
  const std::string& parentName)
{
  this->initializeFrom(groupItem.get(), originName, xAxisName, yAxisName, zAxisName, parentName);
}

bool CoordinateFrame::initializeFrom(
  const smtk::attribute::GroupItem* groupItem,
  const std::string& originName,
  const std::string& xAxisName,
  const std::string& yAxisName,
  const std::string& zAxisName,
  const std::string& parentName)
{
  auto originItemPtr = groupItem->findAs<DoubleItem>(originName);
  auto xAxisItemPtr = groupItem->findAs<DoubleItem>(xAxisName);
  auto yAxisItemPtr = groupItem->findAs<DoubleItem>(yAxisName);
  auto zAxisItemPtr = groupItem->findAs<DoubleItem>(zAxisName);
  auto parentItemPtr = groupItem->findAs<ReferenceItem>(parentName);

  if (!originItemPtr || !xAxisItemPtr || !yAxisItemPtr || !zAxisItemPtr)
  {
    return false;
  }

  for (std::size_t i = 0; i < 3; ++i)
  {
    if (originItemPtr)
    {
      this->origin[i] = originItemPtr->value(i);
    }
    if (xAxisItemPtr)
    {
      this->xAxis[i] = xAxisItemPtr->value(i);
    }
    if (yAxisItemPtr)
    {
      this->yAxis[i] = yAxisItemPtr->value(i);
    }
    if (zAxisItemPtr)
    {
      this->zAxis[i] = zAxisItemPtr->value(i);
    }
  }

  if (parentItemPtr && parentItemPtr->numberOfValues() == 1 && parentItemPtr->isSet())
  {
    this->parent = parentItemPtr->valueAs<PersistentObject>()->id();
  }

  return true;
}

} // namespace properties
} // namespace resource
} // namespace smtk
