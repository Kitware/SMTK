//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/UngroupingGroup.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include <cassert>
#include <limits>

namespace smtk
{
namespace operation
{

Operation::Index UngroupingGroup::matchingOperation(
  const smtk::resource::PersistentObject& obj) const
{
  Operation::Index index = 0;
  std::size_t indexGen = std::numeric_limits<std::size_t>::max();
  for (const auto& candidate : this->operations())
  {
    std::size_t gen = this->operationObjectDistance(candidate, obj);
    if (gen < indexGen)
    {
      indexGen = gen;
      index = candidate;
      if (gen == 0)
      { // This is the most exact
        break;
      }
    }
  }
  return index;
}
} // namespace operation
} // namespace smtk
