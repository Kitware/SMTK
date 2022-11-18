//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_arcs_ReferencesToPrimaries_txx
#define smtk_markup_arcs_ReferencesToPrimaries_txx

#include "smtk/markup/arcs/ReferencesToPrimaries.h"
#include "smtk/markup/arcs/ReferencesToPrimaries.txx"

#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/DiscreteGeometry.h"
#include "smtk/markup/Resource.h"

namespace smtk
{
namespace markup
{
namespace arcs
{

template<typename Functor>
smtk::common::Visited ReferencesToPrimaries::outVisitor(const SpatialData* from, Functor ff) const
{
  smtk::common::Visited result = smtk::common::Visited::Empty;
  if (const auto* dg = dynamic_cast<const DiscreteGeometry*>(from))
  {
    smtk::common::VisitorFunctor<Functor> visitor(ff);
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
            result = smtk::common::Visited::All;
            if (visitor(node) == smtk::common::Visit::Halt)
            {
              result = smtk::common::Visited::Some;
              return result;
            }
          }
        }
      }
    }
  }
  return result;
}

template<typename Functor>
smtk::common::Visited ReferencesToPrimaries::inVisitor(const SpatialData* to, Functor ff) const
{
  smtk::common::Visited result = smtk::common::Visited::Empty;
  if (const auto* dg = dynamic_cast<const DiscreteGeometry*>(to))
  {
    smtk::common::VisitorFunctor<Functor> visitor(ff);
    std::vector<AssignedIds*> assignments;
    dg->assignedIds(assignments);
    std::set<const DiscreteGeometry*> visited;
    for (const auto& assignment : assignments)
    {
      if (
        assignment->nature() != IdNature::Referential &&
        assignment->nature() != IdNature::Unassigned)
      {
        auto destinations = assignment->space()->assignedIds(
          assignment->range()[0], assignment->range()[1], IdNature::Referential);
        for (const auto& destination : destinations)
        {
          auto* node = destination->nodeAs<DiscreteGeometry>();
          if (node && visited.insert(node).second)
          {
            result = smtk::common::Visited::All;
            if (visitor(node) == smtk::common::Visit::Halt)
            {
              result = smtk::common::Visited::Some;
              return result;
            }
          }
        }
      }
    }
  }
  return result;
}

template<typename ResourcePtr, typename Functor>
smtk::common::Visited ReferencesToPrimaries::visitAllOutgoingNodes(ResourcePtr rr, Functor ff) const
{
  const auto* rsrc = dynamic_cast<const smtk::markup::Resource*>(rr);
  if (!rsrc)
  {
    return smtk::common::Visited::Empty;
  }
  auto domainNames = rsrc->domains().keys();
  smtk::common::VisitorFunctor<Functor> visitor(ff);
  for (const auto& domainName : domainNames)
  {
    auto space = std::dynamic_pointer_cast<IdSpace>(rsrc->domains().find(domainName));
    if (space)
    {
      auto referencesOut =
        space->assignedIds(space->range()[0], space->range()[1], IdNature::Referential);
      for (const auto& assignment : referencesOut)
      {
        if (
          visitor(assignment->template nodeAs<const typename ReferencesToPrimaries::FromType>()) ==
          smtk::common::Visit::Halt)
        {
          return smtk::common::Visited::Some;
        }
      }
    }
  }
  return smtk::common::Visited::All;
}

template<typename ResourcePtr, typename Functor>
smtk::common::Visited ReferencesToPrimaries::visitAllIncomingNodes(ResourcePtr rr, Functor ff) const
{
  const auto* rsrc = dynamic_cast<const smtk::markup::Resource*>(rr);
  if (!rsrc)
  {
    return smtk::common::Visited::Empty;
  }
  auto domainNames = rsrc->domains().keys();
  smtk::common::VisitorFunctor<Functor> visitor(ff);
  for (const auto& domainName : domainNames)
  {
    auto space = std::dynamic_pointer_cast<IdSpace>(rsrc->domains().find(domainName));
    if (space)
    {
      auto primariesIn =
        space->assignedIds(space->range()[0], space->range()[1], IdNature::Unassigned);
      for (const auto& assignment : primariesIn)
      {
        if (
          assignment->nature() != IdNature::Referential &&
          assignment->nature() != IdNature::Unassigned)
        {
          if (
            visitor(assignment->nodeAs<const typename ReferencesToPrimaries::ToType>()) ==
            smtk::common::Visit::Halt)
          {
            return smtk::common::Visited::Some;
          }
        }
      }
    }
  }
}

} // namespace arcs
} // namespace markup
} // namespace smtk

#endif // smtk_markup_arcs_ReferencesToPrimaries_txx
