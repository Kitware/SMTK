//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_polygon_internal_Neighborhood_h
#define smtk_session_polygon_internal_Neighborhood_h

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/internal/ActiveFragmentTree.h"

#include "smtk/common/UnionFind.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Resource.h"

#include <list>
#include <map>
#include <set>
#include <vector>

namespace smtk
{
namespace session
{
namespace polygon
{

typedef std::vector<std::pair<smtk::model::Edge, bool>> OrientedEdges;

/**\brief Represent the neighborhood of a sweepline point, x.
  *
  * This holds a CCW-ordered list of edges incident to x, plus an
  * array of fragments
  */
class Neighborhood
{
public:
  Neighborhood(
    SweeplinePosition& x,
    FragmentArray& fragments,
    SweepEventSet& eventQueue,
    ActiveFragmentTree& active,
    smtk::session::polygon::SessionPtr sess);

  int sweep();

  template<typename T>
  void getLoops(T evaluator);

  // Internal methods:

  RegionId lowerRegionJustAbove(FragmentId frag);
  RegionId upperRegionJustBelow(FragmentId frag);

  RegionId lowerRegionJustAbove(const internal::Point& pt) const;
  RegionId upperRegionJustBelow(const internal::Point& pt) const;

  bool isFragmentOutgoing(const EdgeFragment& frag);

  void relateNeighborhoods(
    FragmentId fA,
    EdgeFragment& fragA,
    bool isOutA,
    FragmentId fB,
    EdgeFragment& fragB,
    bool isOutB,
    RegionId region);
  void mergeRelated();

  void assignAndMergeRegions(
    const std::list<FragmentId>::iterator& ringA,
    const std::list<FragmentId>::iterator& ringB);

  bool insertFragmentBetween(
    const std::list<FragmentId>::iterator& ringA,
    const std::list<FragmentId>::iterator& ringB,
    FragmentId fragId,
    EdgeFragment& frag,
    const internal::Point& other);
  void insertFragment(FragmentId fragId, EdgeFragment& frag, const internal::Point& other);
  void queueActiveEdge(FragmentId fragId, EdgeFragment& frag);
  void removeActiveEdge(FragmentId fragId);
  void processFragmentEndEvents();
  void processFragmentStartEvents();
  void processNeighbors();
  void advanceSweeplineTo(const internal::Point& pt);
  void removeDeactivatedEdges();
  void dumpRegions();
  RegionId traverseLoop(
    OrientedEdges& result,
    std::set<RegionId>& neighborRegions,
    FragmentId fragId,
    bool orientation);
  void dumpLoop(OrientedEdges& loopEdges, RegionId contained, std::set<RegionId>& neighborRegions);
  void dumpRegions2();
  void setDebugLevel(int lvl) { m_debugLevel = lvl; }

  template<typename T>
  typename T::Ptr findStorage(const smtk::common::UUID& uid)
  {
    return m_session->findStorage<T>(uid);
  }

  SweeplinePosition* m_point;  // The position used for ordering line segments in m_activeEdges.
  internal::Point m_nextPoint; // The next point the sweepline will advance to.
  FragmentArray* m_fragments;
  SweepEventSet* m_eventQueue;
  ActiveFragmentTree* m_activeEdges;
  RegionIdSet m_regionIds;
  RegionDefinitions m_regions;
  std::vector<FragmentId> m_fragmentsToQueue;
  std::set<FragmentId> m_fragmentsToDeactivate;
  std::list<FragmentId> m_ring; // offsets into m_fragments that order a neighborhood CCW
  std::set<std::pair<RegionId, RegionId>>
    m_related; // regions containing other regions (first = parent, second=child)
  RegionId m_outside;
  smtk::model::Resource::Ptr m_resource;
  smtk::session::polygon::SessionPtr m_session;
  int m_debugLevel{ 0 };
};

} // namespace polygon
} //namespace session
} // namespace smtk

#endif // smtk_session_polygon_internal_Neighborhood_h
