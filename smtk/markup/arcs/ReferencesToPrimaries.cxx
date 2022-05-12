//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/arcs/ReferencesToPrimaries.h"

#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/DiscreteGeometry.h"
#include "smtk/markup/IdSpace.h"

namespace smtk
{
namespace markup
{

namespace
{

bool rangesOverlap(const std::array<IdType, 2>& aa, const std::array<IdType, 2>& bb)
{
  bool result = aa[0] <= bb[1] && aa[1] >= bb[0];
  return result;
}

} // anonymous namespace
namespace arcs
{

bool ReferencesToPrimaries::contains(const SpatialData* from, const SpatialData* to) const
{
  static thread_local std::vector<AssignedIds*> fromAssignments;
  static thread_local std::vector<AssignedIds*> toAssignments;

  if (!from || !to)
  {
    return false;
  }

  if (const auto* dgFrom = dynamic_cast<const DiscreteGeometry*>(from))
  {
    if (const auto* dgTo = dynamic_cast<const DiscreteGeometry*>(to))
    {
      dgFrom->assignedIds(fromAssignments);
      dgTo->assignedIds(toAssignments);
      for (const auto& fromAssignment : fromAssignments)
      {
        // The "from" node must be referential.
        if (fromAssignment->nature() != IdNature::Referential)
        {
          continue;
        }
        for (const auto& toAssignment : toAssignments)
        {
          // The "to" node must be primary or non-exclusive.
          if (
            toAssignment->nature() == IdNature::Referential ||
            toAssignment->nature() == IdNature::Unassigned)
          {
            continue;
          }

          if (
            fromAssignment->space() == toAssignment->space() &&
            rangesOverlap(fromAssignment->range(), toAssignment->range()))
          {
            // TODO: Refine this by searching for IDs that match in both ranges
            //       using the contains() method to bisect?
            return true;
          }
        }
      }
    }
  }
  return false;
}

std::size_t ReferencesToPrimaries::outDegree(const SpatialData* from) const
{
  std::size_t result = 0;
  if (const auto* dg = dynamic_cast<const DiscreteGeometry*>(from))
  {
    std::vector<AssignedIds*> assignments;
    dg->assignedIds(assignments);
    std::set<DiscreteGeometry*> visited;
    for (const auto& assignment : assignments)
    {
      if (assignment->nature() == IdNature::Referential)
      {
        auto destinations = assignment->space()->assignedIds(
          assignment->range()[0], assignment->range()[1], IdNature::Unassigned);
        for (const auto& destination : destinations)
        {
          auto* node = destination->nodeAs<DiscreteGeometry>();
          if (node && visited.insert(node).second)
          {
            ++result;
          }
        }
      }
    }
  }
  return result;
}

std::size_t ReferencesToPrimaries::inDegree(const SpatialData* to) const
{
  std::size_t result = 0;
  if (const auto* dg = dynamic_cast<const DiscreteGeometry*>(to))
  {
    std::vector<AssignedIds*> assignments;
    dg->assignedIds(assignments);
    std::set<DiscreteGeometry*> visited;
    for (const auto& assignment : assignments)
    {
      if (
        assignment->nature() == IdNature::Primary || assignment->nature() == IdNature::NonExclusive)
      {
        auto destinations = assignment->space()->assignedIds(
          assignment->range()[0], assignment->range()[1], IdNature::Referential);
        for (const auto& destination : destinations)
        {
          auto* node = destination->nodeAs<DiscreteGeometry>();
          if (node && visited.insert(node).second)
          {
            ++result;
          }
        }
      }
    }
  }
  return result;
}

} // namespace arcs
} // namespace markup
} // namespace smtk
