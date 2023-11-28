//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ArcCreator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"

#include <cassert>
#include <limits>

namespace smtk
{
namespace operation
{

const std::string ArcCreator::s_defaultArcDestinationItemName = "to";

std::string ArcCreator::arcDestinationItemNameForOperation(const std::string& typeName) const
{
  return *(m_arcDestinationItemName.values(typeName).begin());
}

std::string ArcCreator::arcDestinationItemNameForOperation(const Operation::Index& index) const
{
  static const std::string nullString;
  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return nullString;
  }

  auto metadata = manager->metadata().get<IndexTag>().find(index);
  if (metadata == manager->metadata().get<IndexTag>().end())
  {
    return nullString;
  }

  return *(m_arcDestinationItemName.values(metadata->typeName()).begin());
}

smtk::attribute::ReferenceItemDefinition::Ptr ArcCreator::arcDestinationItemDefinitionForOperation(
  const std::string& typeName) const
{
  Operation::Specification spec = specification(typeName);
  if (spec == nullptr)
  {
    return smtk::attribute::ReferenceItemDefinition::Ptr();
  }

  Operation::Definition parameterDefinition = extractParameterDefinition(spec, typeName);
  int i =
    parameterDefinition->findItemPosition(*(m_arcDestinationItemName.values(typeName).begin()));
  assert(i >= 0);
  return std::dynamic_pointer_cast<smtk::attribute::ReferenceItemDefinition>(
    parameterDefinition->itemDefinition(i));
}

smtk::attribute::ReferenceItemDefinition::Ptr ArcCreator::arcDestinationItemDefinitionForOperation(
  const Operation::Index& index) const
{
  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return smtk::attribute::ReferenceItemDefinition::Ptr();
  }

  auto metadata = manager->metadata().get<IndexTag>().find(index);
  if (metadata == manager->metadata().get<IndexTag>().end())
  {
    return smtk::attribute::ReferenceItemDefinition::Ptr();
  }

  Operation::Specification spec = specification(metadata->typeName());
  if (spec == nullptr)
  {
    return smtk::attribute::ReferenceItemDefinition::Ptr();
  }

  Operation::Definition parameterDefinition =
    extractParameterDefinition(spec, metadata->typeName());
  int i = parameterDefinition->findItemPosition(
    *(m_arcDestinationItemName.values(metadata->typeName()).begin()));
  assert(i >= 0);
  return std::dynamic_pointer_cast<smtk::attribute::ReferenceItemDefinition>(
    parameterDefinition->itemDefinition(i));
}

std::set<std::string> ArcCreator::allArcTypes() const
{
  std::set<std::string> arcTypes;
  for (const auto& candidate : this->operations())
  {
    auto candidateArcTypes = this->values(candidate);
    arcTypes.insert(candidateArcTypes.begin(), candidateArcTypes.end());
  }
  return arcTypes;
}

std::set<std::pair<std::string, Operation::Index>> ArcCreator::allArcCreators() const
{
  std::set<std::pair<std::string, Operation::Index>> result;
  for (const auto& candidate : this->operations())
  {
    auto candidateArcTypes = this->values(candidate);
    for (const auto& arcType : candidateArcTypes)
    {
      result.emplace(arcType, candidate);
    }
  }
  return result;
}

std::unordered_set<Operation::Index> ArcCreator::matchingOperations(
  const smtk::resource::PersistentObject& from,
  const smtk::resource::PersistentObject& to) const
{
  std::unordered_set<Operation::Index> result;
  for (const auto& candidate : this->operations())
  {
    auto fromRule = this->operationAssociationsRule(candidate);
    if (fromRule && fromRule->isValueValid(from.shared_from_this()))
    {
      // FIXME: Fetch arcDestinationItemName and use it instead of "to"
      auto toRule = this->operationReferenceItemRule(candidate, "to");
      if (toRule && toRule->isValueValid(to.shared_from_this()))
      {
        result.insert(candidate);
      }
    }
  }
  return result;
}
} // namespace operation
} // namespace smtk
