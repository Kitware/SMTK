//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_Geometry_h
#define smtk_geometry_Geometry_h

#include "smtk/geometry/Backend.h"
#include "smtk/resource/PersistentObject.h"

#include <array>
#include <atomic>
#include <limits>

namespace smtk
{
namespace resource
{
class CopyOptions;
}
namespace geometry
{

/**\brief A base class for objects which can provide renderable geometry.
  *
  * Resources which have renderable data should provide access
  * to that data by returning a concrete instance of this class.
  * Subclasses present in other libraries (which have additional dependencies
  * on e.g., VTK, VTK-m) provide the API by which the geometry is exchanged.
  * However, this base class provides related functionality independent of
  * any rendering that all renderable objects must share in common.
  *
  * Resources register a Generator with their resource index as the input
  * and a subclass of Geometry as the output.
  * Then, applications or scripts can ask the resource for a geometry
  * provider and query component/resource geometric bounds. They can
  * also mark components and resources as "dirty" (indicating that any
  * cached geometry or bounds information should be discarded).
  * Each resource produces a single instance of some subclass of this class.
  * The geometry instance for a resource should be capable of providing
  * geometry for the entire resource and/or any of its components as needed.
  *
  * Producing renderable geometry can be computationally expensive,
  * so this class provides a generation number associated with the geometry.
  * So long as the generation number is unchanged, so is the geometry.
  * Classes (such as vtkModelMultiBlockSource) need only compare the
  * generation number to the value they obtained when last querying the
  * geometryProvider; if it is the same, then no changes have occurred and
  * the prior geometry can be used (saving the expense of recomputing it).
  *
  * Similarly, it is best for objects to cache bounds. These may even be
  * available without resorting to discretizing the model geometry, so
  * this class also provides a method for obtaining bounds as a 6-tuple
  * (xmin, xmax, ymin, ymax, zmin, zmax).
  *
  * \sa smtk::session::vtk::source::Geometry
  * \sa smtk::session::polygon::Geometry
  */
class SMTKCORE_EXPORT Geometry : smtkEnableSharedPtr(Geometry)
{
public:
  /// An integral type to hold monotonically increasing generation numbers
  /// (a.k.a. generation numbers, sequence numbers, version numbers,
  /// timestamps) used to indicate when the tessellation has changed.
  ///
  /// Note that 0 is reserved; no object should have generation number 0.
  using GenerationNumber = size_t;

  /// A special generation number that marks an object as having no geometric representation.
  static constexpr GenerationNumber Invalid = std::numeric_limits<GenerationNumber>::lowest();

  /// The lowest valid generation number (which may be used as the initial value).
  static constexpr GenerationNumber Initial = std::numeric_limits<GenerationNumber>::lowest() + 1;

  /// A bounding box is represented as an array of 6 numbers,
  /// ordered xmin, xmax, ymin, ymax, zmin, zmax.
  using BoundingBox = std::array<double, 6>;

  /// The signature of functions used to visit all objects with tessellation data.
  using Visitor = std::function<bool(const resource::PersistentObject::Ptr&, GenerationNumber)>;

  /// The data needed to request a geometry provider (a resource and backend).
  ///
  /// The "using" statements below this class declaration reference this
  /// to make registering geometry providers simpler and less error-prone;
  using Specification = std::tuple<smtk::resource::ResourcePtr, const smtk::geometry::Backend&>;

  /// The return type of the geometry-provider generator class.
  ///
  /// Geometry providers are unique pointers to ensure that copies of cached
  /// geometry are not copied accidentally.
  using UniquePtr = std::unique_ptr<smtk::geometry::Geometry>;

  smtkTypeMacroBase(Geometry);
  Geometry()
    : m_lastModified(Initial)
  {
  }

  virtual ~Geometry() = default;

  /// Every provider must indicate the backend for which it is specialized.
  virtual const Backend& backend() const = 0;

  /// Every provider must indicate the resource it provides geometry for.
  virtual ResourcePtr resource() const = 0;

  /// Indication of when the geometry has last been updated.
  ///
  /// This may be used by consumers to determine whether any
  /// object's renderable geometry has been updated.
  ///
  /// Consumers are responsible for remembering the previously
  /// returned value. Whenever this method returns a larger number,
  /// at least one object has been updated.
  ///
  /// Providers are responsible for updating m_lastModified.
  GenerationNumber lastModified() const { return m_lastModified; }

  ///@name Query methods
  ///@{
  /// These methods are invoked by consumers of geometry.

  /// Return a monotonically increasing number that changes whenever the object's geometry changes.
  virtual GenerationNumber generationNumber(const resource::PersistentObject::Ptr&) const = 0;
  /// Populate the bounds (\a bds) with the given object's bounds.
  virtual void bounds(const resource::PersistentObject::Ptr&, BoundingBox& bds) const = 0;
  /// Return true if the resource owning this geometryProvider must be read-locked before rendering prep.
  ///
  /// The default is to return true (i.e., conservatively assume zero-copying is performed).
  virtual bool readLockRequired() const { return true; }
  /// Visit each persistent object that has renderable geometry.
  virtual void visit(Visitor fn) const = 0;
  ///@}

  ///@name Modfication methods
  ///@{
  /// These methods are typically invoked by operations
  /// that create or modify objects with renderable geometry.
  /// There is no requirement that these methods be used by subclasses
  /// (there may be alternate methods)

  /// Mark the object's geometry as modified.
  /// Calling this should increment the generation number and
  /// recompute any tessellation the next time the object is queried.
  /// This is a way for applications to "force refresh" a tessellation.
  virtual void markModified(const resource::PersistentObject::Ptr& obj) = 0;

  /// Inform the geometry provider that an object is about to be or has been deleted.
  /// This is different from markModified(), which simply removes any
  /// cached geometry; this method tells the geometry provider never to ask
  /// for the geometry of the given \a uid again.
  virtual bool erase(const smtk::common::UUID& uid) = 0;
  ///@}

  ///@name Copy methods
  ///@{
  /// This method is used by smtk::geometry::Resource::copyGeometry()
  /// to shallow-copy renderable geometry into a cloned/copied resource.
  ///
  /// The default implementation (since it does not know the form of
  /// the renderable geometry) does nothing. If your backend's implementation
  /// uses the geometry::Cache, copying is supported as long as
  /// copy-construction of the renderable geometry is supported.
  ///
  /// This method should return true on success and false otherwise.
  /// If \a options.copyGeometry() is false, this method should always
  /// return true (i.e., it is not an error to omit geometry when asked
  /// to omit it).
  virtual bool copyGeometry(const UniquePtr& sourceGeometry, smtk::resource::CopyOptions& options);
  ///@}

protected:
  std::atomic<GenerationNumber> m_lastModified;
};

} // namespace geometry
} // namespace smtk

#endif // smtk_geometry_Geometry_h
