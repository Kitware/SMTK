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
#include "smtk/geometry/Geometry.h"

#include <memory>

namespace smtk
{
/// A subsystem for attaching renderable geometry to components and resources.
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
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  ~Resource() override;

  /// Given a backend, return a provider of geometry for that backend.
  ///
  /// This may return null if (a) neither the resource nor anything it contains
  /// is capable of providing geometry or (b) the application is not linked
  /// to a library that can provide data in a format acceptable to the backend.
  std::unique_ptr<Geometry>& geometry(const Backend& backend);

  /// Return the first geometry provider (for any backend).
  ///
  /// This may return null if (a) neither the resource nor anything it contains
  /// is capable of providing geometry or (b) the application is not linked
  /// to a library that can provide data in a format acceptable to the backend.
  ///
  /// Use this method when querying for the existence of geometry or for object
  /// bounds, since it terminates earlier than visitGeometry(). Note, however,
  /// that:
  /// 1. It assumes geometry objects for multiple backends will provide geometry
  ///    and bounds identically for the same set of persistent objects.
  /// 2. It assumes that the resource has been added to a resource manager
  ///    that has a geometry manager observing it and that some generator for
  ///    this resource type and any backend was registered with the geometry
  ///    manager *before* this resource was added to the resource manager.
  ///    Otherwise, no geometry object will have been constructed and there
  ///    is no way to obtain the geometry manager from the resource manager to
  ///    attempt construction if iteration of existing Geometry objects fails.
  std::unique_ptr<Geometry>& geometry();

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
  /// (Note: the pattern above is used by smtk::operation::MarkGeometry; you
  /// should use that class rather than directly copy the code above to mark
  /// geometry as modified.)
  void visitGeometry(std::function<void(std::unique_ptr<Geometry>&)> visitor);
  void visitGeometry(std::function<void(const std::unique_ptr<Geometry>&)> visitor) const;

  Resource(Resource&&) = default;

  /// Copy renderable geometry from \a source into this resource.
  ///
  /// This method is intended to be called by subclasses that choose to
  /// override smtk::resource::Resource::copyInitialize().
  /// This method does nothing if \a options.copyGeometry() is false, so
  /// it is safe to call it without checking \a options yourself.
  void copyGeometry(
    const std::shared_ptr<const Resource>& source,
    smtk::resource::CopyOptions& options);

protected:
  Resource(const smtk::common::UUID& myID, smtk::resource::ManagerPtr manager);
  Resource(const smtk::common::UUID& myID);
  Resource(smtk::resource::ManagerPtr manager = nullptr);

private:
  std::map<Backend::index_t, std::unique_ptr<Geometry>> m_geometry;
};

} // namespace geometry
} // namespace smtk

#endif // smtk_geometry_Resource_h
