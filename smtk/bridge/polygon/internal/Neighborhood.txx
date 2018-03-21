//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_Neighborhood_txx
#define __smtk_session_polygon_internal_Neighborhood_txx

#include "smtk/bridge/polygon/internal/Neighborhood.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Traverse the face loops enumerated by sweeping all neighborhoods.
  *
  * The \a evaluator object must provide a method named "evaluateLoop"
  * that accepts an array of oriented fragments, a region ID, and a set
  * of "border" region IDs.
  */
template <typename T>
void Neighborhood::getLoops(T evaluator)
{
  FragmentArray::iterator fit;
  fit = m_fragments->begin();
  if (fit == m_fragments->end())
  {
    return;
  }

  // Traverse every fragment. For each unprocessed coedge,
  // if the region it bounds is not marked as "exterior", then queue the loop
  // it resides on.
  // The outer loop will always be the first one encountered.
  // All other loops with the same region ID are inner loops or embedded
  // edges (when both sides of the coedge belong to the same region).
  OrientedEdges loopEdges;
  std::set<RegionId> neighborRegions;
  std::set<RegionId>::const_iterator pit;
  FragmentId fid = 0;
  RegionId outside = m_regionIds.find(m_outside);
  for (fit = m_fragments->begin(); fit != m_fragments->end(); ++fit, ++fid)
  {
    if (!fit->marked(false) && m_regionIds.find(fit->ccwRegion(false)) != outside)
    {
      RegionId contained = this->traverseLoop(loopEdges, neighborRegions, fid, false);
      evaluator->evaluateLoop(m_regionIds.find(contained), loopEdges, neighborRegions);
    }
    if (!fit->marked(true) && m_regionIds.find(fit->ccwRegion(true)) != outside)
    {
      RegionId contained = this->traverseLoop(loopEdges, neighborRegions, fid, true);
      evaluator->evaluateLoop(m_regionIds.find(contained), loopEdges, neighborRegions);
    }
  }

  // Now erase the "visited" marks so we can re-traverse if needed:
  for (fit = m_fragments->begin(); fit != m_fragments->end(); ++fit)
  {
    fit->mark(false, 0);
    fit->mark(true, 0);
  }
}

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_internal_Neighborhood_txx
