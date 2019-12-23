//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_Resource_h
#define smtk_geometry_Resource_h

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Resource.h"

#include "smtk/geometry/Backend.h"

namespace smtk
{
namespace geometry
{

class Backend;
class Geometry;
class Manager;

/// An abstract base class for SMTK resources that provide geometry for themselves and/or their components.
class SMTKCORE_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(smtk::geometry::Resource);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  virtual ~Resource();

  /// Given a backend, return a provider of geometry for that backend.
  ///
  /// This may return null if (a) neither the resource nor anything it contains
  /// is capable of providing geometry or (b) the application is not linked
  /// to a library that can provide data in a format acceptable to the backend.
  std::unique_ptr<Geometry>& geometry(const Backend& backend);

  /// Visit all existing geometry providers for this resource.
  ///
  /// This will invoke the given \a visitor function on each provider.
  /// Typically, this is used to inform all providers when a given object's geometry
  /// has been modified, like so:
  /// ```c++
  /// geometry::Resource::Ptr rsrc;
  /// PersistentObject::Ptr obj;
  /// rsrc->visitGeometry(
  ///   [&obj](std::unique_ptr<Geometry>& provider)
  ///   {
  ///     provider->markModified(obj);
  ///   }
  /// );
  /// ```
  /// (Note: the pattern above is used by smtk::operation::GeometryMarkup; you
  /// should use that class rather than directly copy the code above to mark
  /// geometry as modified.)
  void visitGeometry(std::function<void(std::unique_ptr<Geometry>&)> visitor);

protected:
  Resource(const smtk::common::UUID& myID, smtk::resource::ManagerPtr manager);
  Resource(const smtk::common::UUID& myID);
  Resource(smtk::resource::ManagerPtr manager = nullptr);

private:
  std::map<Backend::index_t, std::unique_ptr<Geometry> > m_geometry;
};

} // namespace geometry
} // namespace smtk

#endif // smtk_geometry_Resource_h
