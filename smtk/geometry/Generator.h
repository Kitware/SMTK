//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_Generator_h
#define smtk_geometry_Generator_h

#include "smtk/common/Generator.h"

#include "smtk/PublicPointerDefs.h"

#include <memory>
#include <tuple>

namespace smtk
{
namespace geometry
{
class Backend;
class Geometry;
}
}

template class SMTKCORE_EXPORT
  smtk::common::Generator<std::tuple<smtk::geometry::ResourcePtr, const smtk::geometry::Backend&>,
    std::unique_ptr<smtk::geometry::Geometry> >;

namespace smtk
{
namespace geometry
{

using Specification = std::tuple<smtk::geometry::ResourcePtr, const smtk::geometry::Backend&>;

/// Declare the class used to _generate_ geometry objects specific to a backend.
///
/// The geometry::Resource class uses this class to create instances of
/// the proper geometry::Geometry subclass.
using Generator = smtk::common::Generator<Specification, GeometryPtr>;

/// Declare the class used to _register_ geometry classes specific to a backend.
///
/// Plugins create objects of this type to register geometry::Geometry
/// subclasses so that the Generator above can instantiate them as needed.
template <typename T>
class Supplier : public smtk::common::GeneratorType<Specification, GeometryPtr, T>
{
public:
  using Specification = geometry::Specification;
  using GeometryPtr = geometry::GeometryPtr;
};

} // namespace geometry
} // namespace smtk

#endif // smtk_geometry_Generator_h
