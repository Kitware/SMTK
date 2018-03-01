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

#include "smtk/operation/Metadata.h"
#include "smtk/operation/Operation.h"

#include "smtk/resource/Lock.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace operation
{

/// smtk::operation::Operation::Specification is a typedef for
/// std::shared_ptr<smtk::attribute::Collection>. In the future, it could
/// become a class that simply holds a
/// std::shared_ptr<smtk::attribute::Collection>. For now, though, we construct
/// an API around Specification, giving it functions that are unique to its role
/// as an operation specification.

typedef std::map<smtk::resource::Resource::Ptr, smtk::resource::Permission> ResourceAccessMap;
typedef std::vector<smtk::attribute::ComponentItemDefinition::Ptr> ComponentDefinitionVector;

/// Construct a map of all of the resources referenced in the specification,
/// along with their permission levels (read/write).
SMTKCORE_NO_EXPORT
ResourceAccessMap extractResourcesAndPermissions(Operation::Specification specification);

/// Construct a vector of all of the resource component definitions referenced
/// in the specification.
SMTKCORE_NO_EXPORT
ComponentDefinitionVector extractComponentDefinitions(Operation::Specification specification);

/// Construct a set of all of the operator tags referenced in the
/// specification.
SMTKCORE_NO_EXPORT std::set<std::string> extractTagNames(Operation::Specification specification);

/// Add a tag to the specification.
SMTKCORE_NO_EXPORT bool addTag(Operation::Specification specification, const std::string& tagName);

/// Add a tag to the specification.
SMTKCORE_NO_EXPORT bool addTag(Operation::Specification specification, const std::string& tagName,
  const std::set<std::string>& tagValues);

/// Remove a tag from the specification.
SMTKCORE_NO_EXPORT bool removeTag(
  Operation::Specification specification, const std::string& tagName);

/// Retrieve a tag's values.
SMTKCORE_NO_EXPORT std::set<std::string> tagValues(
  Operation::Specification specification, const std::string& tagName);
}
}

#endif // smtk_operation_SpecificationOps_h
