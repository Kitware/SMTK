//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/WriterGroup.h"
#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Manager.h"

#include <cassert>

namespace smtk
{
namespace operation
{

bool WriterGroup::registerOperation(
  const std::string& operatorName, const std::string& resourceName, const std::string& fileItemName)
{
  Operation::Specification spec = specification(operatorName);

  if (!spec)
  {
    return false;
  }

  Operation::Parameters parameters = extractParameters(spec, operatorName);

  if (parameters == nullptr)
  {
    return false;
  }

  // If we have a valid file item name, ensure it is propertly registered.
  // Operations for this group are not required to have a file name.
  if (parameters->findFile(fileItemName) != nullptr)
  {
    if (!m_fileItemName.registerOperation(operatorName, { fileItemName }))
    {
      return false;
    }
  }

  return Group::registerOperation(operatorName, { resourceName });
}

std::shared_ptr<smtk::operation::Operation> WriterGroup::writerForResource(
  const std::string& resourceName) const
{
  std::set<Operation::Index> indices = operationsForResource(resourceName);

  if (indices.empty())
  {
    return std::shared_ptr<smtk::operation::Operation>();
  }

  auto manager = m_manager.lock();

  // Groups are only defined in the context of operation metadata, which
  // is held by a manager.
  if (!manager)
  {
    return std::shared_ptr<smtk::operation::Operation>();
  }
  return manager->create(*indices.begin());
}
}
}
