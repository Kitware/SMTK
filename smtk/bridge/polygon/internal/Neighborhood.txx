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

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Traverse the face loops enumerated by sweeping all neighborhoods.
  *
  * The \a evaluator object must provide a method named "evaluateLoop"
  * that accepts an array of oriented fragments, a region ID, and a set
  * of "border" region IDs.
  */
template<typename T>
void Neighborhood::getLoops(T evaluator)
{
  FragmentArray::iterator fit;
  std::cout << "\nFeepy crawler\n";
  fit = this->m_fragments->begin();
  // The left/lower region of the first fragment to be inserted is always exterior
  // to all faces. It corresponds to a "hole" cut in the infinite plane of all fragments.
  RegionId outside = this->m_regionIds.find(fit->lowerRegion());

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
  for (fit = this->m_fragments->begin(); fit != this->m_fragments->end(); ++fit, ++fid)
    {
    if (!fit->marked(false) && this->m_regionIds.find(fit->ccwRegion(false)) != outside)
      {
      RegionId contained = this->traverseLoop(loopEdges, neighborRegions, fid, false);
      evaluator->evaluateLoop(this->m_regionIds.find(contained), loopEdges, neighborRegions);
      }
    if (!fit->marked(true) && this->m_regionIds.find(fit->ccwRegion(true)) != outside)
      {
      RegionId contained = this->traverseLoop(loopEdges, neighborRegions, fid, true);
      evaluator->evaluateLoop(this->m_regionIds.find(contained), loopEdges, neighborRegions);
      }
    }

  // Now erase the "visited" marks so we can re-traverse if needed:
  for (fit = this->m_fragments->begin(); fit != this->m_fragments->end(); ++fit)
    {
    fit->mark(false, 0);
    fit->mark(true, 0);
    }
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_internal_Neighborhood_txx
