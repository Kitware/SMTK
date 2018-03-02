//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ReaderGroup.h"

#include "smtk/operation/Manager.h"

#include <cassert>

namespace smtk
{
namespace operation
{

bool ReaderGroup::registerOperation(
  const std::string& operatorName, const std::string& resourceName, const std::string& fileItemName)
{
  Operation::Specification spec = specification(operatorName);
  Operation::Parameters parameters = extractParameters(spec, operatorName);

  if (parameters == nullptr)
  {
    return false;
  }

  return (parameters->findFile(fileItemName) != nullptr &&
    Group::registerOperation(operatorName, { resourceName }) &&
    m_fileItemName.registerOperation(operatorName, { fileItemName }));
}

std::shared_ptr<smtk::operation::Operation> ReaderGroup::readerForResource(
  const std::string& resourceName) const
{
  auto manager = m_manager.lock();

  // Groups are only defined in the context of operation metadata, which
  // is held by a manager.
  if (!manager)
  {
    return std::shared_ptr<smtk::operation::Operation>();
  }

  std::set<Operation::Index> operationIndices = operations();
  for (auto& index : operationIndices)
  {
    std::set<std::string> resourceNames = readsResources(index);
    if (resourceNames.find(resourceName) != resourceNames.end())
    {
      return manager->create(index);
    }
  }

  return std::shared_ptr<smtk::operation::Operation>();
}

std::set<std::string> ReaderGroup::readsResources(const Operation::Index& index) const
{
  return values(index);
}
}
}
