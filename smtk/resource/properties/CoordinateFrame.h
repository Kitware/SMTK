//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_properties_CoordinateFrame_h
#define smtk_resource_properties_CoordinateFrame_h

#include <array>
#include <memory>

#include "smtk/attribute/GroupItem.h"
#include "smtk/common/UUID.h"

namespace smtk
{
namespace resource
{
/**\brief Namespace for non-POD data that can be attached to resources and components.
  *
  */
namespace properties
{

/// A Coordinate reference frame described in a hierarchical fashion
/// with respect to its parent component.
/// Defined by an origin point and 3 axis vectors.
struct SMTKCORE_EXPORT CoordinateFrame
{
public:
  /// Origin of this frame in parents coordinates.
  std::array<double, 3> origin{ 0, 0, 0 };
  /// X Axis of this frame in parents coordinates.
  std::array<double, 3> xAxis{ 1, 0, 0 };
  /// Y Axis of this frame in parents coordinates.
  std::array<double, 3> yAxis{ 0, 1, 0 };
  /// Z Axis of this frame in parents coordinates.
  std::array<double, 3> zAxis{ 0, 0, 1 };
  /// Hierarchical parent whose transform should be concatenated, if any.
  /// If parent is null, then the coordinate frame transforms to world space.
  smtk::common::UUID parent;

  /// Allow default constructor.
  CoordinateFrame() = default;

  /// Construct CoordinateFrame object from a given GroupItem.
  CoordinateFrame(
    const smtk::attribute::ConstGroupItemPtr& groupItem,
    const std::string& originName = "Origin",
    const std::string& xAxisName = "XAxis",
    const std::string& yAxisName = "YAxis",
    const std::string& zAxisName = "ZAxis",
    const std::string& parentName = "Parent");
  CoordinateFrame(
    const smtk::attribute::GroupItemPtr& groupItem,
    const std::string& originName = "Origin",
    const std::string& xAxisName = "XAxis",
    const std::string& yAxisName = "YAxis",
    const std::string& zAxisName = "ZAxis",
    const std::string& parentName = "Parent");

protected:
  bool initializeFrom(
    const smtk::attribute::GroupItem* groupItem,
    const std::string& originName,
    const std::string& xAxisName,
    const std::string& yAxisName,
    const std::string& zAxisName,
    const std::string& parentName);
};

} // namespace properties
} // namespace resource
} // namespace smtk

#endif // smtk_resource_properties_CoordinateFrame_h
