//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_SpecificationOps_h
#define smtk_operation_SpecificationOps_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"

#include "smtk/operation/Metadata.h"
#include "smtk/operation/Operation.h"

#include "smtk/resource/Lock.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace operation
{

/// smtk::operation::Operation::Specification is a typedef for
/// std::shared_ptr<smtk::attribute::Resource>. In the future, it could
/// become a class that simply holds a
/// std::shared_ptr<smtk::attribute::Resource>. For now, though, we construct
/// an API around Specification, giving it functions that are unique to its role
/// as an operation specification.

typedef std::vector<smtk::attribute::ComponentItemDefinition::Ptr> ComponentDefinitionVector;

/// Return a new set of parameters for an operation.
SMTKCORE_EXPORT
Operation::Parameters createParameters(
  Operation::Specification specification,
  const std::string& operatorName,
  const std::string& parametersName);

/// Return parameters for an operation if they already exist or a new parameters object otherwise.
SMTKCORE_EXPORT
Operation::Parameters extractParameters(
  Operation::Specification specification,
  const std::string& operatorName);

/// Return the definition for the operation.
SMTKCORE_EXPORT
Operation::Definition extractParameterDefinition(
  Operation::Specification specification,
  const std::string& operatorName);

/// Return the definition for the operation result.
SMTKCORE_EXPORT
Operation::Definition extractResultDefinition(
  Operation::Specification specification,
  const std::string& operatorName);

/// Construct a set of all of the resources referenced in the result.
SMTKCORE_EXPORT
std::set<
  std::weak_ptr<smtk::resource::Resource>,
  std::owner_less<std::weak_ptr<smtk::resource::Resource>>>
extractResources(Operation::Result result);

/// Construct a map of all of the resources referenced in the parameters and not
/// in the result, along with their lock types (Read/Write/DoNotLock).
SMTKCORE_EXPORT
ResourceAccessMap extractResourcesAndLockTypes(Operation::Parameters parameters);

/// Construct a map of all of the resources referenced in the specification and
/// not in the result, along with their lock types (Read/Write/DoNotLock).
SMTKCORE_EXPORT
ResourceAccessMap extractResourcesAndLockTypes(Operation::Specification specification);

/// Obtain locks for all resources in a ResourceAccessMap.
///
/// If \a nonBlocking is false (the default), this method will block until all the
/// \a resources are locked and will never return a null pointer. This may result
/// in a deadlock if you are not careful.
///
/// If \a nonBlocking is true (the default is false), then always return immediately.
/// In this case, the return value may be a null pointer (indicating that at least one
/// of the \a resources could not be locked without blocking).
std::unique_ptr<smtk::resource::ScopedLockSetGuard> lockResources(
  const ResourceAccessMap& resourcesAndLockTypes,
  bool nonBlocking = false);

/// Construct a vector of all of the resource component definitions referenced
/// in the specification.
SMTKCORE_EXPORT
ComponentDefinitionVector extractComponentDefinitions(Operation::Specification specification);

/// Construct a set of all of the operator tags referenced in the
/// specification.
SMTKCORE_EXPORT std::set<std::string> extractTagNames(Operation::Specification specification);

/// Add a tag to the specification.
SMTKCORE_EXPORT bool addTag(Operation::Specification specification, const std::string& tagName);

/// Add a tag to the specification.
SMTKCORE_EXPORT bool addTag(
  Operation::Specification specification,
  const std::string& tagName,
  const std::set<std::string>& tagValues);

/// Remove a tag from the specification.
SMTKCORE_EXPORT bool removeTag(Operation::Specification specification, const std::string& tagName);

/// Retrieve a tag's values.
SMTKCORE_EXPORT std::set<std::string> tagValues(
  Operation::Specification specification,
  const std::string& tagName);
} // namespace operation
} // namespace smtk

#endif // smtk_operation_SpecificationOps_h
