//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/attribute/Attribute.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include <cassert>

namespace
{
}

namespace smtk
{
namespace operation
{
std::set<Operation::Index> ImporterGroup::operationsForFileName(const std::string& fileName) const
{
  std::set<Operation::Index> ops = operations();
  filterOperationsThatRejectFileName(ops, fileName);
  return ops;
}

std::set<Operation::Index> ImporterGroup::operationsForResourceAndFileName(
  const std::string& resourceName, const std::string& fileName) const
{
  std::set<Operation::Index> ops = operationsForResource(resourceName);
  filterOperationsThatRejectFileName(ops, fileName);
  return ops;
}

void ImporterGroup::filterOperationsThatRejectFileName(
  std::set<Operation::Index>& ops, const std::string& fileName) const
{
  for (auto index = ops.begin(); index != ops.end(); ++index)
  {
    if (fileItemDefinitionForOperation(*index)->isValueValid(fileName) == false)
    {
      ops.erase(index);
    }
  }
}
}
}
