//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/DeleterGroup.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include <cassert>

namespace smtk
{
namespace operation
{

Operation::Index DeleterGroup::matchingOperation(const smtk::resource::PersistentObject& obj) const
{
  Operation::Index index;
  for (const auto& candidate : this->operations())
  {
    if (this->operationAcceptsObject(candidate, obj))
    {
      // TODO: We could wait and see if other candidates match better.
      index = candidate;
      break;
    }
  }
  return index;
}

bool DeleterGroup::operationAcceptsObject(
  const Operation::Index& index, const smtk::resource::PersistentObject& obj) const
{
  auto assocRule = this->operationAssociationsRule(index);
  bool acceptable = assocRule && assocRule->isValueValid(obj.shared_from_this());
  return acceptable;
}

smtk::attribute::ConstReferenceItemDefinitionPtr DeleterGroup::operationAssociationsRule(
  const Operation::Index& index) const
{
  using smtk::attribute::ConstReferenceItemDefinitionPtr;
  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return ConstReferenceItemDefinitionPtr();
  }

  auto metadata = manager->metadata().get<IndexTag>().find(index);
  if (metadata == manager->metadata().get<IndexTag>().end())
  {
    return ConstReferenceItemDefinitionPtr();
  }

  Operation::Specification spec = specification(metadata->typeName());
  if (spec == nullptr)
  {
    return ConstReferenceItemDefinitionPtr();
  }

  Operation::Definition parameterDefinition =
    extractParameterDefinition(spec, metadata->typeName());
  return parameterDefinition->associationRule();
}
}
}
