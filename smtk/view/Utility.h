//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_Utility_h
#define smtk_view_Utility_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/TypeName.h"

#include <functional>
#include <map>
#include <string>
#include <utility>

namespace smtk
{
namespace view
{

/// A structure for grouping components by their resources with shared pointers.
struct SharedResourceMapValue
{
  /// A resource.
  std::shared_ptr<smtk::resource::Resource> m_resource;
  /// A subset of components held by m_resource.
  std::unordered_set<std::shared_ptr<smtk::resource::Component>> m_components;
};

using ResourceMap =
  std::unordered_map<smtk::resource::Resource*, std::unordered_set<smtk::resource::Component*>>;

using SharedResourceMap = std::unordered_map<smtk::common::UUID, SharedResourceMapValue>;

/**\brief Compute a visibility map from a set of object-pointers.
  *
  * This function turns a flat set of object pointers into a map
  * from a resource to a set of component pointers. If an object
  * is a resource, it is represented in the map as an entry with
  * the resource as the key and an null component pointer as the
  * value.
  */
ResourceMap SMTKCORE_EXPORT
objectsToResourceMap(const std::set<smtk::resource::PersistentObject*>& objects);

SharedResourceMap SMTKCORE_EXPORT
objectsToSharedResourceMap(const std::set<smtk::resource::PersistentObject*>& objects);

} // namespace view
} // namespace smtk

#endif
