//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_MarkGeometry_h
#define smtk_operation_MarkGeometry_h

#include "smtk/operation/Operation.h"

#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Resource.h"

namespace smtk
{
namespace operation
{

/**\brief Succinctly indicate components/resources whose geometry has changed.
  *
  * This class can be used to mark which components and resources have
  * renderable geometry or when that geometry has been modified.
  *
  * Operations that do not affect renderable geometry should not need to use this class.
  * Operations that modify the geometry of every object reported in the result can
  * simply use the markResult() method.
  * Operations that affect the geometry of some subset of created/modified/expunged
  * components can use other methods on those components individually.
  * This makes it easy for operations to trigger minimal rendering-backend updates.
  */
class SMTKCORE_EXPORT MarkGeometry
{
public:
  smtkTypeMacroBase(smtk::operation::MarkGeometry);
  /// Construct a MarkGeometry that must take in persistent objects (not UUIDs)
  /// in order to mark geometric changes.
  MarkGeometry() = default;
  /// Construct a MarkGeometry that only and always uses the geometry providers
  /// owned by the given resource to mark geometric changes.
  ///
  /// Prefer this constructor when all objects are owned by a single resource.
  /// Do not use this constructor otherwise.
  MarkGeometry(const smtk::geometry::ResourcePtr&);

  /// Mark a single persistent object's geometry as having been
  /// modified (across all backends).
  void markModified(const smtk::resource::PersistentObjectPtr& object);

  /// Mark a single persistent object's geometry as having been
  /// erased (across all backends).
  void erase(const smtk::resource::PersistentObjectPtr& object);
  void erase(const smtk::common::UUID& objectId);

  /// Mark all components referenced by the item as having their
  /// geometry modified (across all backends).
  void markModified(const smtk::attribute::ReferenceItemPtr& item);

  /// Mark all components referenced by the item as having their
  /// geometry erased (across all backends).
  void erase(const smtk::attribute::ReferenceItemPtr& item);

  /// Mark all created/modified/expunged components as having their
  /// geometry modified or deleted (across all backends).
  void markResult(const smtk::operation::Operation::Result& result);

protected:
  smtk::geometry::ResourcePtr m_resource;
};

} // namespace operation
} // namespace smtk

#endif // smtk_operation_MarkGeometry_h
