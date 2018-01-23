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

#include "smtk/operation/NewOp.h"

#include "smtk/resource/Lock.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace operation
{

typedef std::map<smtk::resource::Resource::Ptr, smtk::resource::Permission> ResourceAccessMap;
typedef std::vector<smtk::attribute::ComponentItemDefinition::Ptr> ComponentDefinitionVector;

/// Construct a map of all of the resources referenced in the specification,
/// along with their permission levels (read/write).
SMTKCORE_NO_EXPORT
ResourceAccessMap extractResourcesAndPermissions(NewOp::Specification specification);

/// Construct a vector of all of the resource component definitions referenced
/// in the specification.
SMTKCORE_NO_EXPORT
ComponentDefinitionVector extractComponentDefinitions(NewOp::Specification specification);
}
}

#endif // smtk_operation_SpecificationOps_h
