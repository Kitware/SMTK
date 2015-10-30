//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_bridge_polygon_Fragment_h
#define __smtk_bridge_polygon_Fragment_h

#include "smtk/bridge/polygon/internal/Config.h"

#include "smtk/model/Edge.h"

#include "smtk/common/UnionFind.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

struct EdgeFragment;
struct EdgeFragmentComparator;
struct Neighborhood;
struct Region;
struct SweepEvent;
struct SweeplinePosition;

typedef size_t FragmentId;
typedef std::set<SweepEvent> SweepEventSet;
typedef std::vector<SweepEvent> SweepEventArray;
typedef std::vector<EdgeFragment> FragmentArray;
typedef int RegionId;
typedef smtk::common::UnionFind<RegionId> RegionIdSet;
typedef std::map<RegionId, smtk::shared_ptr<Region> > RegionDefinitions;

/// Structure to hold information about a portion of an edge-segment forming part of an output loop.
struct EdgeFragment
{
  internal::Point m_lo; // Low is relative to the sweep direction (left to right, bottom to top).
  internal::Point m_hi; // High is relative to the sweep direction.
  smtk::model::Edge m_edge; // SMTK model information
  internal::EdgePtr m_edgeData; // Private edge data (sequence of points defining segments)
  int m_segment; // Offset into edge's point sequence defining the segment containing this fragment.
  bool m_sense; // True when fragment and model edge are codirectional; false when they are antidirectional.
  RegionId m_regionId[2]; // Union-Find region to each side of edge; 0: region CCW of edge, 1: region CW of edge.
  FragmentId m_next[2]; // Next co-fragment in region; 0: opposite of fragment dir, 1: along fragment dir.
  bool m_nsns[2]; // Sense of next co-fragment in region; 0: opposite of fragment dir, 1: along fragment dir.
  int m_marked[2]; // Has this co-edge been output?

  EdgeFragment();

  smtk::model::Edge edge() const { return this->m_edge; }
  bool orientation() const { return this->m_sense; }

  internal::Point& lo() { return this->m_lo; }
  const internal::Point& lo() const { return this->m_lo; }

  internal::Point& hi() { return this->m_hi; }
  const internal::Point& hi() const { return this->m_hi; }

  /// Return the ID of the region above the fragment.
  RegionIdSet::value_type& upperRegion() { return this->m_regionId[1]; }
  RegionIdSet::value_type upperRegion() const { return this->m_regionId[1]; }
  /// Return the ID of the region below the fragment.
  RegionIdSet::value_type& lowerRegion() { return this->m_regionId[0]; }
  RegionIdSet::value_type lowerRegion() const { return this->m_regionId[0]; }

  /**\brief Return the ID of the region just counter-clockwise (CCW) of the fragment...
    *
    * ... when winding around the lower (\a fromLowerEnd is true) or
    * upper (\a fromLowerEnd is false) endpoint of the fragment.
    *
    * You can also think of \a fromLowerEnd as representing the
    * orientation of the co-fragment you wish to consider;
    * calling ccwRegion(false) returns ID of the region to the
    * left of the reversed co-fragment (from hi() to lo()) while
    * calling ccwRegion(true) returns ID of the region to the
    * left of the forward co-fragment (from lo() to hi()).
    */
  RegionIdSet::value_type& ccwRegion(bool fromLowerEnd) { return this->m_regionId[fromLowerEnd ? 1 : 0]; }
  RegionIdSet::value_type ccwRegion(bool fromLowerEnd) const { return this->m_regionId[fromLowerEnd ? 1 : 0]; }
  /**\brief Return the ID of the region just clockwise (CW) of the fragment...
    *
    * ... when winding around the lower (\a fromLowerEnd is true) or
    * upper (\a fromLowerEnd is false) endpoint of the fragment.
    */
  RegionIdSet::value_type& cwRegion(bool fromLowerEnd) { return this->m_regionId[fromLowerEnd ? 0 : 1]; }
  RegionIdSet::value_type cwRegion(bool fromLowerEnd) const { return this->m_regionId[fromLowerEnd ? 0 : 1]; }

  /**\brief Return the next fragment bounding the region to the left of the fragment.
    *
    */
  FragmentId& nextFragment(bool forwardDir) { return this->m_next[forwardDir ? 1 : 0]; }
  bool& nextFragmentSense(bool forwardDir) { return this->m_nsns[forwardDir ? 1 : 0]; }

  /// Mark a co-fragment as visited (or not).
  void mark(bool orientation, int markVal)
    {
    this->m_marked[orientation ? 1 : 0] = markVal;
    }

  /// Return the markings on the forward (orientation true) or backward (false) co-fragment.
  int marked(bool orientation) const
    {
    return this->m_marked[orientation ? 1 : 0];
    }

  /// Debug dump of fragment
  void dump(RegionIdSet& ufind) const;
};

/// Functor to compare indices into a vector of EdgeFragments based on which fragment is above the other.
struct EdgeFragmentComparator
{
  FragmentArray* m_sweptFragments;
  SweeplinePosition* m_sweepPoint;

  EdgeFragmentComparator(FragmentArray& frag, SweeplinePosition& startPoint);
  EdgeFragmentComparator(const EdgeFragmentComparator& other);

  /// Return the array of fragments the comparator indexes into.
  FragmentArray* fragments() const
    {
    return this->m_sweptFragments;
    }

  bool operator() (FragmentId a, FragmentId b) const;
};

struct SweeplinePosition
{
  internal::Point m_position;
  SweeplinePosition(const internal::Point& posn);
  SweeplinePosition(const SweeplinePosition& other);

  /// Return the current sweepline position
  internal::Point& position() { return this->m_position; }

  /// Return the current sweepline position
  const internal::Point& position() const { return this->m_position; }

  /// Advance the sweepline to another position, ignoring invalid points to the left of the current position.
  void advance(const internal::Point& pt);

  bool operator < (const internal::Point& other)
    {
    return this->m_position < other;
    }
  bool operator > (const internal::Point& other)
    {
    return this->m_position > other;
    }
  bool operator == (const internal::Point& other)
    {
    return this->m_position == other;
    }
  bool operator != (const internal::Point& other)
    {
    return this->m_position != other;
    }
};



    } // namespace smtk
  } // namespace bridge
} // namespace polygon 

#endif // __smtk_bridge_polygon_Fragment_h
