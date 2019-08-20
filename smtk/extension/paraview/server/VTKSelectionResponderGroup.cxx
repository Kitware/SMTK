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

std::string VTKSelectionResponderGroup::resourceForOperation(const Operation::Index& index) const
{
  auto vals = values(index);
  return !vals.empty() ? *vals.begin() : "";
}

std::string VTKSelectionResponderGroup::resourceForOperation(const std::string& operationName) const
{
  auto vals = values(operationName);
  return !vals.empty() ? *vals.begin() : "";
}

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
    std::string resourceTypeName = this->resourceForOperation(index);
    if (selectedResource->isOfType(resourceTypeName))
    {
      operations.insert(index);
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
    resources.insert(resourceForOperation(index));
  }
  return resources;
}
}
}
