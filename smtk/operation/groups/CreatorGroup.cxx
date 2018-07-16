//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace operation
{

std::string CreatorGroup::resourceForOperation(const Operation::Index& index) const
{
  auto vals = values(index);
  return !vals.empty() ? *vals.begin() : "";
}

std::string CreatorGroup::resourceForOperation(const std::string& operationName) const
{
  auto vals = values(operationName);
  return !vals.empty() ? *vals.begin() : "";
}

std::set<Operation::Index> CreatorGroup::operationsForResource(
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

std::set<std::string> CreatorGroup::supportedResources() const
{
  std::set<std::string> resources;

  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return resources;
  }

  std::set<Operation::Index> allOperations = this->operations();

  for (auto& index : allOperations)
  {
    resources.insert(resourceForOperation(index));
  }
  return resources;
}
}
}
