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

/// Construct CoordinateFrame object from a given GroupItem
CoordinateFrame::CoordinateFrame(smtk::attribute::ConstGroupItemPtr& groupItem)
{
  auto originItemPtr = groupItem->findAs<DoubleItem>("Origin");
  auto xAxisItemPtr = groupItem->findAs<DoubleItem>("XAxis");
  auto yAxisItemPtr = groupItem->findAs<DoubleItem>("YAxis");
  auto zAxisItemPtr = groupItem->findAs<DoubleItem>("ZAxis");
  auto parentItemPtr = groupItem->findAs<ReferenceItem>("Parent");

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

  if (parentItemPtr && parentItemPtr->numberOfValues() == 1)
  {
    this->parent = parentItemPtr->valueAs<PersistentObject>()->id();
  }
}

} // namespace properties
} // namespace resource
} // namespace smtk
