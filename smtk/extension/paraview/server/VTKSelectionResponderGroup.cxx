//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"
#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"
#include "smtk/resource/MetadataContainer.h"

namespace smtk
{
namespace view
{

std::set<smtk::operation::Operation::Index> VTKSelectionResponderGroup::operationsForResource(
  const smtk::resource::ResourcePtr& selectedResource) const
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
    for (const auto& resourceTypeName : this->values(index))
    {
      if (selectedResource->isOfType(resourceTypeName))
      {
        operations.insert(index);
        break;
      }
    }
  }
  return operations;
}

std::set<std::string> VTKSelectionResponderGroup::supportedResources() const
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
    for (const auto& resourceTypeName : this->values(index))
    {
      resources.insert(resourceTypeName);
    }
  }
  return resources;
}
}
}
