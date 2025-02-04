//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ResultOps_h
#define smtk_operation_ResultOps_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/ReferenceItem.h"

#include "smtk/operation/Operation.h"

#include "smtk/resource/Resource.h"

#include <set>

namespace smtk
{
namespace operation
{

/// Insert any entries of \a item that are resources into the \a resources set.
///
/// If \a includeProjectChildren is true (the default), then if any resource
/// mentioned in \a item is a project, its child resources will be added.
/// Note that \a includeProjectChildren does not yet recurse projects whose
/// child resources are themselves projects.
SMTKCORE_EXPORT void addResourcesOfReferenceItem(
  const smtk::attribute::ReferenceItem::Ptr& item,
  std::set<smtk::resource::Resource::Ptr>& resources,
  bool includeProjectChildren = true);

/// Construct a set of all newly-created resources referenced in the result.
///
/// Note that unlike the extractResources() function, only new resources
/// are included (rather than any resource referenced, particularly those
/// resources mentioned by a component they own).
SMTKCORE_EXPORT std::set<smtk::resource::Resource::Ptr> createdResourcesOfResult(
  const Operation::Result& result,
  bool includeProjectChildren = true);

/// Construct a set of all modified resources referenced in the result.
SMTKCORE_EXPORT std::set<smtk::resource::Resource::Ptr> modifiedResourcesOfResult(
  const Operation::Result& result);

/// Construct a set of all resources to be expunged after processing the result.
SMTKCORE_EXPORT std::set<smtk::resource::Resource::Ptr> expungedResourcesOfResult(
  const Operation::Result& result,
  bool includeProjectChildren = true);

} // namespace operation
} // namespace smtk

#endif // smtk_operation_ResultOps_h
