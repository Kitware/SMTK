//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ExporterGroup.h"
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
std::set<Operation::Index> ExporterGroup::operationsForFileName(const std::string& fileName) const
{
  std::set<Operation::Index> ops = operations();
  filterOperationsThatRejectFileName(ops, fileName);
  return ops;
}

std::set<Operation::Index> ExporterGroup::operationsForResourceAndFileName(
  const std::string& resourceName, const std::string& fileName) const
{
  std::set<Operation::Index> ops = operationsForResource(resourceName);
  filterOperationsThatRejectFileName(ops, fileName);
  return ops;
}

void ExporterGroup::filterOperationsThatRejectFileName(
  std::set<Operation::Index>& ops, const std::string& fileName) const
{
  for (auto index = ops.begin(); index != ops.end();)
  {
    if (!fileItemDefinitionForOperation(*index)->isValueValid(fileName))
    {
      ops.erase(index++);
    }
    else
    {
      ++index;
    }
  }
}
}
}
