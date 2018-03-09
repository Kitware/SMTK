//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ResourceIOGroup.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace operation
{

const std::string ResourceIOGroup::m_defaultFileItemName = "filename";

const std::string& ResourceIOGroup::fileItemNameForOperation(const std::string& uniqueName) const
{
  return *(m_fileItemName.values(uniqueName).begin());
}

const std::string& ResourceIOGroup::fileItemNameForOperation(const Operation::Index& index) const
{
  static const std::string nullString = "";
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

  return *(m_fileItemName.values(metadata->uniqueName()).begin());
}

smtk::attribute::FileItemDefinition::Ptr ResourceIOGroup::fileItemDefinitionForOperation(
  const std::string& uniqueName) const
{
  Operation::Specification spec = specification(uniqueName);
  if (spec == nullptr)
  {
    return smtk::attribute::FileItemDefinition::Ptr();
  }

  Operation::Definition parameterDefinition = extractParameterDefinition(spec, uniqueName);
  int i = parameterDefinition->findItemPosition(*(m_fileItemName.values(uniqueName).begin()));
  assert(i >= 0);
  return std::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(
    parameterDefinition->itemDefinition(i));
}

smtk::attribute::FileItemDefinition::Ptr ResourceIOGroup::fileItemDefinitionForOperation(
  const Operation::Index& index) const
{
  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return smtk::attribute::FileItemDefinition::Ptr();
  }

  auto metadata = manager->metadata().get<IndexTag>().find(index);
  if (metadata == manager->metadata().get<IndexTag>().end())
  {
    return smtk::attribute::FileItemDefinition::Ptr();
  }

  Operation::Specification spec = specification(metadata->uniqueName());
  if (spec == nullptr)
  {
    return smtk::attribute::FileItemDefinition::Ptr();
  }

  Operation::Definition parameterDefinition =
    extractParameterDefinition(spec, metadata->uniqueName());
  int i =
    parameterDefinition->findItemPosition(*(m_fileItemName.values(metadata->uniqueName()).begin()));
  assert(i >= 0);
  return std::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(
    parameterDefinition->itemDefinition(i));
}

std::string ResourceIOGroup::resourceForOperation(const Operation::Index& index) const
{
  auto vals = values(index);
  return !vals.empty() ? *vals.begin() : "";
}

std::set<Operation::Index> ResourceIOGroup::operationsForResource(
  const std::string& resourceName) const
{
  std::set<Operation::Index> operations;

  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return operations;
  }

  std::set<Operation::Index> allOperations = this->operations();

  for (auto& index : allOperations)
  {
    if (resourceForOperation(index) == resourceName)
    {
      operations.insert(index);
    }
  }
  return operations;
}
}
}
