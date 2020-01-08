//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_GeometryForBackend_h
#define smtk_resource_GeometryForBackend_h

#include "smtk/geometry/Geometry.h"

namespace smtk
{
namespace geometry
{

/**\brief A base class for objects which can provide renderable geometry in a specific format.
  *
  * All concrete geometry providers should inherit this class;
  * it exists so that rendering backends can cast providers to
  * a subclass that allows geometry to be fetched in a format
  * they can handle.
  *
  * The template parameter must be a type (struct, class, or primitive type)
  * that meets the following requirements:
  *
  * 1. The type must be castable to a boolean value that indicates
  *    whether the object is valid (true) or invalid (false).
  * 2. The type (Format) must have a default constructor that
  *    produces an invalid object.
  *
  * Additionally, it is preferable (but not required) for the
  * template parameter type to be move-constructible.
  */
template <typename Format>
class GeometryForBackend : public Geometry
{
public:
  smtkTypeMacro(GeometryForBackend);
  smtkSuperclassMacro(Geometry);
  using DataType = Format;

  virtual ~GeometryForBackend() {}

  /// Return the geometry associated with an object.
  ///
  /// Only call this method after ensuring that generationNumber(obj) != Invalid.
  virtual Format& geometry(const resource::PersistentObject::Ptr&) const = 0;
};

} // namespace resource
} // namespace smtk

#endif // smtk_resource_GeometryForBackend_h
