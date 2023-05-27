//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/internal/Neighborhood.h"

#include "smtk/session/polygon/internal/Config.h"
#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Region.h"
#include "smtk/session/polygon/internal/SweepEvent.h"
#include "smtk/session/polygon/internal/Util.h"

#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"

namespace smtk
{
namespace session
{
namespace polygon
{

Neighborhood::Neighborhood(
  SweeplinePosition& x,
  FragmentArray& fragments,
  SweepEventSet& eventQueue,
  ActiveFragmentTree& active,
  smtk::session::polygon::SessionPtr sess)
  : m_point(&x)
  , m_fragments(&fragments)
  , m_eventQueue(&eventQueue)
  , m_activeEdges(&active)
  , m_resource(sess->resource())
  , m_session(sess)
{
}

int Neighborhood::sweep()
{
  m_outside = -1;
  std::set<SweepEvent>::iterator event;

  while (!m_eventQueue->empty())
  {
    // peek at the first event's point
    event = m_eventQueue->begin();
    internal::Point npt = event->point();
    m_nextPoint = npt;

    // Process end events as long as they correspond to the current point
    //   remove fragment from active queue (immediately)
    //   add fragId to neighborhood
    this->processFragmentEndEvents();

    // Process start events as long as they correspond to the current point
    //   create fragment
    //   add fragment to output
    //   add fragId to queue for insertion after neighborhood processing.
    //   add fragId to neighborhood
    this->processFragmentStartEvents();

    // Advance sweep position to npt (safe now as no active edges have npt as endpt)
    this->advanceSweeplineTo(npt);

    // Process neighborhood of npt
    //   traverse neighboring fragment pairs
    //     set next-frag
    //     set region information
    //     add related regions to pairs spanning vertical rays emanating from npt
    //   traverse queued fragments (i.e., those starting at npt)
    //     insert fragId in active edges
    //     insert fragment (starting at npt) end-event to event queue
    //   traverse active fragments
    //     if fragId in queued fragments (i.e., starting at npt),
    //       test for intersection with its immediate neighbors in active fragments
    //       split fragments that intersect:
    //         remove from active edges
    //         change fragment endpoint to intersection x_i
    //         add fragment back to active edges
    //           (testing again for intersection with immediate neighbors as x_i
    //            may not lie exactly on the original fragment due to integer truncation).
    //         add new start events (at x_i) of second-half of split fragments
    this->processNeighbors();
  }

  this->mergeRelated();
  return 0;
}

/// Return the region ID neighboring a fragment from above.
RegionId Neighborhood::lowerRegionJustAbove(FragmentId frag)
{
  ActiveFragmentTree::iterator edgeNeighbor = m_activeEdges->upper_bound(frag);
  if (edgeNeighbor == m_activeEdges->end())
    return -1;
  return (*m_fragments)[*edgeNeighbor].lowerRegion();
}

/// Return the region ID neighboring a fragment from below.
RegionId Neighborhood::upperRegionJustBelow(FragmentId frag)
{
  ActiveFragmentTree::iterator edgeNeighbor = m_activeEdges->lower_bound(frag);
  if (edgeNeighbor != m_activeEdges->end())
  { // We found the edge or the edge wasn't active but we have an immediate neighbor above.
    if (edgeNeighbor == m_activeEdges->begin())
    { // The edge has no neighbor below:
      return -1;
    }
    --edgeNeighbor;
    return (*m_fragments)[*edgeNeighbor].upperRegion();
  }
  // The edge is not active and there's no edge above it.
  // Thus, if any edges are active, the topmost one is
  // just below \a frag.
  if (m_activeEdges->empty())
    return -1;
  // Not empty => m_activeEdges->rbegin() is valid:
  return (*m_fragments)[*m_activeEdges->rbegin()].upperRegion();
}

/// Return the region ID neighboring a point from above.
RegionId Neighborhood::lowerRegionJustAbove(const internal::Point& pt) const
{
  FragmentId frag = m_activeEdges->boundingFragments(pt, true).second;
  if (frag != static_cast<FragmentId>(-1))
  {
    return (*m_fragments)[frag].lowerRegion();
  }
  return static_cast<RegionId>(-1);
}

/// Return the region ID neighboring a point from below.
RegionId Neighborhood::upperRegionJustBelow(const internal::Point& pt) const
{
  FragmentId frag = m_activeEdges->boundingFragments(pt, true).first;
  if (frag != static_cast<FragmentId>(-1))
  {
    return (*m_fragments)[frag].upperRegion();
  }
  return static_cast<RegionId>(-1);
}

static void printRegionId(RegionId x)
{
  if (x < 0)
    std::cout << "-1 ";
  else
    std::cout << (x / 2) << (x % 2 == 0 ? "L" : "U") << " ";
}

static void printRelating(RegionId a, RegionId b, int caseNum, int debugLevel)
{
  if (debugLevel > 1)
  {
    std::cout << "    Relating ";
    printRegionId(a);
    printRegionId(b);
    std::cout << " (case " << caseNum << ")\n";
  }
}

static void printRelating(RegionId a, RegionId b, RegionId c, int caseNum, int debugLevel)
{
  if (debugLevel > 1)
  {
    std::cout << "    Relating ";
    printRegionId(a);
    printRegionId(b);
    printRegionId(c);
    std::cout << " (case " << caseNum << ")\n";
  }
}

/**\brief Return the orientation of a fragment relative to the neighborhood.
 *
 * Returns true when the neighborhood is placed at the left/lower end of the fragment
 * and true otherwise.
 * This is used to obtain the proper region ID when winding around
 * the edges incident to the neighborhood.
 */
bool Neighborhood::isFragmentOutgoing(const EdgeFragment& frag)
{
  return frag.lo() == m_nextPoint;
}

/// Relate a region between 2 fragments A & B which share a vertex x to neighborhoods just before and after x.
void Neighborhood::relateNeighborhoods(
  FragmentId fA,
  EdgeFragment& fragA,
  bool isOutA,
  FragmentId fB,
  EdgeFragment& fragB,
  bool isOutB,
  RegionId region)
{
  // NB: Inside this method, the vertex "x" shared by fragments A and B
  //     is referred to as "o" (their common origin).

  // Determine whether this pair of fragments crosses +y or -y,
  // which informs us of which regions border the neighborhood
  // and are identical with disjoint boundary curves.
  static internal::Coord yy[2] = { 0, 1 };
  internal::Coord oa[2];
  internal::Coord ob[2];
  oa[0] = (fragA.hi().x() - fragA.lo().x()) * (isOutA ? +1 : -1);
  oa[1] = (fragA.hi().y() - fragA.lo().y()) * (isOutA ? +1 : -1);
  ob[0] = (fragB.hi().x() - fragB.lo().x()) * (isOutB ? +1 : -1);
  ob[1] = (fragB.hi().y() - fragB.lo().y()) * (isOutB ? +1 : -1);

  internal::Point& origin(isOutA ? fragA.lo() : fragA.hi());
  internal::HighPrecisionCoord oaXyy = cross2d(oa, yy);
  internal::HighPrecisionCoord yyXob = cross2d(yy, ob);
  internal::HighPrecisionCoord oaXob = cross2d(oa, ob);

  // oaXyy * yyXob is positive when all three edges are in CCW order
  // and oa-ob is acute or obtuse but not reflex (oaXob > 0).
  if (oaXyy * yyXob > 0)
  {
    if (oaXyy > 0)
    { // Fragment B is incoming and bounded above by a fragment whose lower region should be merged with idB
      RegionId other = this->lowerRegionJustAbove(origin);
      printRelating(region, other, 1, m_debugLevel);
      m_related.insert(std::pair<RegionId, RegionId>(other, region));
    }
    else // (oaXyy < 0)
    { // Fragment A is incoming and bounded below by a fragment whose upper region should be merged with idA
      RegionId other = this->upperRegionJustBelow(origin);
      printRelating(region, other, 2, m_debugLevel);
      m_related.insert(std::pair<RegionId, RegionId>(other, region));
    }
  }
  else if (oaXyy * yyXob < 0)
  {
    if (oaXob > 0)
    {
      // Acute/obtuse angle between A & B; do nothing.
    }
    else // (oaXob <= 0)
    {
      // Reflex angle between A & B; both neighborhoods (above and below) encompassed.
      // In the case of oaXyy > 0, both edges are outgoing (and thus not in activeSegments yet)
      // but lookup using the origin ("o") and either slope (A or B) is OK because there
      // cannot be any incoming edges. So, we perform the lookups using edge that are correct
      // for the oaXyy < 0 case (where A and B are both incoming and the above/below lookups
      // would be incorrect if we swap A and B).
      if (oaXob == 0 && (fA != fB || m_ring.size() > 1))
      {
        smtkWarningMacro(
          m_resource->log(), "Neighborhood of edge fragment is invalid. Expect invalid results.");
      }
      RegionId above = this->lowerRegionJustAbove(origin);
      RegionId below = this->upperRegionJustBelow(origin);
      printRelating(region, above, below, 3, m_debugLevel);
      m_related.insert(std::pair<RegionId, RegionId>(above, region));
      m_related.insert(std::pair<RegionId, RegionId>(below, region));
    }
  }
  else // (oaXyy * yyXob == 0)
  {
    if (oaXob < 0) // A & B reflex
    {
      // Neighborhood is directly connected by an edge to previous or next
      // neighborhood (hence oaXyy*yyXob == 0), but the A-B region also
      // spans the other neighborhood boundary when A & B are reflex (oaXob < 0)
      // so we must mark the region below or above it.
      RegionId other;
      if (oaXyy > 0 || yyXob > 0)
      {
        // Either:
        //     oaXyy > 0 and thus fragB must be along -y (since oaXob < 0),
        // or  yyXob > 0 and thus fragA must be along -y (since oaXob < 0);
        // thus A-B merges with the region just below: the active segment just above B
        other = this->lowerRegionJustAbove(origin);
      }
      else // (oxXyy < 0 || yyXob < 0)
      {
        // Either:
        //     oaXyy < 0 and thus fragB must be along +y (since oaXob < 0),
        // or  yyXob < 0 and thus fragA must be along +y (since oaXob < 0);
        // thus A-B merges with the region just above: the active segment just below A
        other = this->upperRegionJustBelow(
          origin); // when oaXyy == 0, fA isn't in m_activeSegments yet, but the lookup is safe).
      }
      printRelating(region, other, 4, m_debugLevel);
      m_related.insert(std::pair<RegionId, RegionId>(other, region));
    }
    else if (oaXob == 0)
    {
      // A & B are 0 or pi **and** aligned with y axis.
      // We can use outgoing/incoming to decide whether they are up (+y, outgoing)
      // or down (-y, incoming). If both are the same direction, then the opposite
      // region should be linked to this region.
      if (isOutA && isOutB)
      { // Both outgoing, link to region below
        RegionId below = this->lowerRegionJustAbove(origin);
        printRelating(region, below, 5, m_debugLevel);
        m_related.insert(std::pair<RegionId, RegionId>(below, region));
      }
      else if (!isOutA && !isOutB)
      {
        RegionId above = this->upperRegionJustBelow(origin);
        printRelating(region, above, 6, m_debugLevel);
        m_related.insert(std::pair<RegionId, RegionId>(above, region));
      }
    }
  }
}

void Neighborhood::mergeRelated()
{
  std::set<std::pair<RegionId, RegionId>>::iterator relIt;
  RegionId firstOutside = -1;
  for (relIt = m_related.begin(); relIt != m_related.end(); ++relIt)
  {
    if (m_regionIds.mergeSets(relIt->first, relIt->second) < 0)
    {
      m_outside = m_regionIds.find(relIt->first < 0 ? relIt->second : relIt->first);
    }
    // mergeSets will not accept negative region IDs;
    // ensure outside regions merge with each other like so:
    if (relIt->first == -1 && relIt->second >= 0)
    {
      if (firstOutside >= 0)
      {
        m_regionIds.mergeSets(firstOutside, relIt->second);
      }
      else
      {
        firstOutside = relIt->second;
      }
    }
  }
}

/// The space between \a ringA and \a ringB is not interrupted; mark coedges of A/B as same region.
void Neighborhood::assignAndMergeRegions(
  const std::list<FragmentId>::iterator& ringA,
  const std::list<FragmentId>::iterator& ringB)
{
  if (m_debugLevel > 1)
  {
    std::cout << "  A-B: " << *ringA << " " << *ringB << "\n";
  }
  EdgeFragment& fragA((*m_fragments)[*ringA]);
  EdgeFragment& fragB((*m_fragments)[*ringB]);
  // Determine sense wrt neighborhood (isOutX == true => fragment's other vertex hasn't been processed yet).
  bool isOutA = this->isFragmentOutgoing(fragA); // true when m_point is coincident with fragA.lower
  bool isOutB = this->isFragmentOutgoing(fragB);

  RegionIdSet::value_type idA = m_regionIds.find(fragA.ccwRegion(isOutA));
  RegionIdSet::value_type idB = m_regionIds.find(fragB.cwRegion(isOutB));
  RegionIdSet::value_type winner;
  if (idA != idB)
  { // Merge regions on inside of A--B. Add coedges (of exiting edges on inside of A--B) to region.
    winner = m_regionIds.mergeSets(idA, idB);
    RegionIdSet::value_type loser = (winner == idA ? idB : idA);
    // If this is a new region, create a record for it.
    if (m_regions.find(winner) == m_regions.end())
    {
      bool seedSense = !isOutB;
      m_regions[winner] = smtk::make_shared<Region>(*ringB, seedSense);
    }
    if (m_regions.find(loser) != m_regions.end())
    {
      m_regions[winner]->merge(m_regions[loser].get());
      m_regions.erase(loser);
    }
  }
  else
  { // Add coedges (of exiting edges on inside of A--B) to region.
    winner = idA;
  }
  // Link one coedge of B to A.
  fragB.nextFragment(!isOutB) = *ringA;
  fragB.nextFragmentSense(!isOutB) = isOutA;

  this->relateNeighborhoods(*ringA, fragA, isOutA, *ringB, fragB, isOutB, winner);
}

/// Insert \a fragId into \a m_ring if it is between \a ringA and \a ringB
bool Neighborhood::insertFragmentBetween(
  const std::list<FragmentId>::iterator& ringA,
  const std::list<FragmentId>::iterator& ringB,
  FragmentId fragId,
  EdgeFragment& frag,
  const internal::Point& other)
{
  (void)frag;
  EdgeFragment& fragA((*m_fragments)[*ringA]);
  EdgeFragment& fragB((*m_fragments)[*ringB]);
  internal::Point otherA(fragA.lo() == m_nextPoint ? fragA.hi() : fragA.lo());
  internal::Point otherB(fragB.lo() == m_nextPoint ? fragB.hi() : fragB.lo());

  internal::Coord oa[2] = { otherA.x() - m_nextPoint.x(), otherA.y() - m_nextPoint.y() };
  internal::Coord ob[2] = { otherB.x() - m_nextPoint.x(), otherB.y() - m_nextPoint.y() };
  internal::Coord oo[2] = { other.x() - m_nextPoint.x(), other.y() - m_nextPoint.y() };
  internal::HighPrecisionCoord oaXoo = cross2d(oa, oo);
  internal::HighPrecisionCoord ooXob = cross2d(oo, ob);
  internal::HighPrecisionCoord oaXob = cross2d(oa, ob);
  if (
    (oaXoo > 0 && ooXob > 0) || // oaXob < pi and "other" is between them; or...
    (oaXob < 0 &&
     !(ooXob < 0 &&
       oaXoo < 0))) // oaXob > pi and "other" is *not between* the short CCW path between B and A.
  {                 // other is between ringA and ringB. Insert it just before ringB:
    m_ring.insert(ringB, fragId);
    return true;
  }
  else if (oaXoo == 0)
  { // Urk. other is collinear with ringA...
    if (dot2d(oa, oo) < 0 && ooXob > 0)
    {
      // ... but antidirectional with ringA; and properly oriented with ringB.
      m_ring.insert(ringB, fragId);
      return true;
    }
    else
    {
      // TODO. FIXME.
      // Replace ringA with fragId if frag is shorter (or barf if lengths identical? surgery to fix problems could be nasty);
      // queue new SegmentStart for remaining long fragment.
    }
  }
  else if (ooXob == 0)
  { // Urk. other is collinear with ringB...
    if (dot2d(oo, ob) < 0 && oaXoo > 0)
    {
      // ... but antidirectional with ringB; and properly oriented with ringA.
      m_ring.insert(ringB, fragId);
      return true;
    }
    else
    {
      // TODO. FIXME.
      // Replace ringB with fragId if frag is shorter (or barf if lengths identical? surgery to fix problems could be nasty);
      // queue new SegmentStart for remaining long fragment.
    }
  }
  return false; // other is not between ringA and ringB.
}

/**\brief Insert \a frag where it belongs in the ring of fragments incident to \a m_point.
 *
 * The \a other point is the end of \a frag which is not \a m_point.
 * This algorithm works by traversing pre-existing neighborhood fragments to identify when
 * dot(cross(fragIt-m_point x other-m_point),(0,0,1)) changes sign from - to +.
 * Or, identically, it inserts frag between a neighboring pair of points on the ring (a,b)
 * when dot(cross(a-m_point x other-m_point),(0,0,1)) > 0 && dot(cross(other-m_point x b-m_point),(0,0,1)) > 0.
 *
 * If either cross product has zero magnitude, the fragment is collinear with an existing segment.
 * In that case, (1) the shorter fragment is kept in the ring; (2) a new SegmentStart event is queued
 * for the uncovered portion of the longer fragment; (3) the longer fragment is discarded; and
 * (4?) a warning is logged.
 */
void Neighborhood::insertFragment(
  FragmentId fragId,
  EdgeFragment& frag,
  const internal::Point& other)
{
  for (int i = 0; i < 2; ++i)
    if (frag.m_regionId[i] < 0)
      frag.m_regionId[i] = m_regionIds.newSet();

  if (m_ring.size() < 2)
  { // No matter where we insert, the order will be CCW. So insert at beginning:
    m_ring.insert(m_ring.begin(), fragId);
    return;
  }
  std::list<FragmentId>::iterator ringA = m_ring.end();
  --ringA; // "unadvance" before end() to the last ring entry.
  std::list<FragmentId>::iterator ringB = m_ring.begin();
  // Start by processing the implicit fragment-pair between m_ring.end() and m_ring.begin():
  if (this->insertFragmentBetween(ringA, ringB, fragId, frag, other))
    return;
  // Now proceed through the list until we find the right spot.
  ringA = ringB;
  for (++ringB; ringB != m_ring.end(); ++ringA, ++ringB /*, sao = -sbo??? */)
  {
    if (this->insertFragmentBetween(ringA, ringB, fragId, frag, other))
      return;
  }
  std::cerr << "Error. Unable to insert fragment " << fragId
            << " into neighborhood!\n"; // FIXME. Add to log, not cerr/cout.
}

void Neighborhood::queueActiveEdge(FragmentId fragId, EdgeFragment& frag)
{
  (void)frag;
  m_fragmentsToQueue.push_back(fragId);
}

void Neighborhood::removeActiveEdge(FragmentId fragId)
{
  bool did = m_activeEdges->erase(fragId) != 0;
  if (m_debugLevel > 2)
  {
    std::cout << "Removing active edge " << fragId << ". Did? " << (did ? "Y" : "N") << "\n";
    ActiveFragmentTreeType::const_iterator it;
    std::cout << "                 Active fragments: ";
    for (it = m_activeEdges->begin(); it != m_activeEdges->end(); ++it)
    {
      std::cout << " " << *it;
    }
    std::cout << "\n";
  }
}

void Neighborhood::processFragmentEndEvents()
{
  const internal::Point& npt(m_nextPoint);
  std::set<SweepEvent>::iterator event;
  while (!m_eventQueue->empty())
  {
    event = m_eventQueue->begin();
    if (event->point() != npt || event->type() != SweepEvent::SEGMENT_END)
    { // Only process events for the neighborhood of \a npt
      return;
    }

    // Process end events as long as they correspond to the current point
    //   remove fragment from active queue (immediately)
    //   add fragId to neighborhood
    FragmentId fragId = event->m_frag[0];
    EdgeFragment& frag((*m_fragments)[fragId]);

    // Add fragment to neighborhood
    this->insertFragment(fragId, frag, frag.lo());

    // Remove fragment from active edges
    this->removeActiveEdge(fragId);

    // Erase the event we've just processed.
    m_eventQueue->erase(event);
  }
}

void Neighborhood::processFragmentStartEvents()
{
  const internal::Point& npt(m_nextPoint);
  std::set<SweepEvent>::iterator event;
  while (!m_eventQueue->empty())
  {
    event = m_eventQueue->begin();
    if (event->point() != npt || event->type() != SweepEvent::SEGMENT_START)
    { // Only process events for the neighborhood of \a npt
      return;
    }

    // Create a new fragment.
    // The m_hi point is altered as segment crossing are processed but the
    // region IDs of this segment will not change. The regions will be
    // unioned with other regions depending on neighborhood adjacency.
    static EdgeFragment blank;
    FragmentArray::size_type fragId = m_fragments->size();
    // Add fragment to output.
    FragmentArray::iterator it = m_fragments->insert(m_fragments->end(), blank);
    it->m_edge = event->m_edge;
    it->m_segment = event->m_indx;
    it->m_edgeData = this->findStorage<internal::edge>(it->m_edge.entity());
    it->m_edgeData->pointsOfSegment(event->m_indx, it->m_lo, it->m_hi);
    it->m_sense = event->m_frag[0] > 0;
    if (it->m_lo > it->m_hi)
    {
      if (it->m_sense)
      {
        std::cerr << "\nOoops, reversed frag sense\n\n";
      }
      std::swap(it->m_lo, it->m_hi);
    }
    else if (!it->m_sense)
    {
      std::cerr << "\nOoops, non-reversed frag sense\n\n";
    }
    it->m_regionId[0] = it->m_regionId[1] = -1; // Indicate regions are unassigned.

    // Add fragId to neighborhood.
    this->insertFragment(fragId, *it, it->m_hi);

    // Add fragId to queue for insertion after neighborhood processing.
    this->queueActiveEdge(fragId, *it);

    // Erase the event we've just processed.
    m_eventQueue->erase(event);
  }
}

/**\brief Process the neighborhood of one or more event endpoints.
 *
 * When this method is called, m_ring contains a CCW-ordered list
 * of fragments incident to the sweepline position.
 */
void Neighborhood::processNeighbors()
{
  if (m_debugLevel > 0)
  {
    std::cout << "Neighborhood::processNeighbors()\n";
  }

  // I. Remove active edges going out of scope after the neighborhood
  //      has been visited.
  this->removeDeactivatedEdges();

  // II. Merge regions associated with neighboring fragments.
  //     This also marks one co-edge of the pair with the "next"
  //     fragment in the loop bounding a region.

  if (!m_ring.empty())
  { // Note that ringA == ringB is valid (both sides of fragment are the same regionId).
    std::list<FragmentId>::iterator ringA = m_ring.end();
    --ringA; // "unadvance" before end() to the last ring entry.
    std::list<FragmentId>::iterator ringB = m_ring.begin();
    // Start by processing the implicit fragment-pair between m_ring.end() and m_ring.begin():
    this->assignAndMergeRegions(ringA, ringB);
    // Now proceed through the list until we have visited them all.
    ringA = ringB;
    for (++ringB; ringB != m_ring.end(); ++ringA, ++ringB /*, sao = -sbo??? */)
    {
      this->assignAndMergeRegions(ringA, ringB);
    }
  }

  // III. We are done processing the ring; if any incident edges are outgoing,
  //      add their SEGMENT_END events to the event queue.
  std::vector<FragmentId>::iterator it;
  for (it = m_fragmentsToQueue.begin(); it != m_fragmentsToQueue.end(); ++it)
  {
    std::pair<ActiveFragmentTreeType::iterator, bool> result = m_activeEdges->insert(*it);
    if (m_debugLevel > 1)
    {
      std::cout << "Inserting active edge " << *it << ". Did? " << (result.second ? "Y" : "N")
                << "\n";
    }
    EdgeFragment& frag((*m_fragments)[*it]);
    m_eventQueue->insert(SweepEvent::SegmentEnd(frag.m_hi, static_cast<int>(*it)));
    // TODO: Check for neighbor intersections; remove them then check for neighbor intersections with *it and add them.
  }
  m_fragmentsToQueue.clear();
  m_ring.clear();
  if (m_debugLevel > 2)
  {
    ActiveFragmentTreeType::const_iterator it2;
    std::cout << "                 Active fragments: ";
    for (it2 = m_activeEdges->begin(); it2 != m_activeEdges->end(); ++it2)
    {
      std::cout << " " << *it2;
    }
    std::cout << "\n";
  }
}

/**\brief Advance the sweepline to the next event's point.
 *
 * This may do nothing if the next event is coincident with the current point.
 * This may advance to some position other than \a pt if any queued edge fragments
 * end or cross before \a pt.
 */
void Neighborhood::advanceSweeplineTo(const internal::Point& pt)
{
  m_point->advance(pt);
}

/// Process all edges passed to removeActiveEdge(). This is called each time (just before) the sweepline is advanced.
void Neighborhood::removeDeactivatedEdges()
{
  while (!m_fragmentsToDeactivate.empty())
  {
    std::set<FragmentId>::iterator fragIt = m_fragmentsToDeactivate.begin();
    bool did = m_activeEdges->erase(*fragIt) != 0;
    if (m_debugLevel > 2)
    {
      std::cout << "*Deactivating frag " << *fragIt << " did? " << (did ? "Y" : "N") << "\n";
    }
    m_fragmentsToDeactivate.erase(fragIt);
  }
}

void Neighborhood::dumpRegions()
{
  FragmentArray::const_iterator fit;
  std::cout << "\nFragments\n";
  std::size_t i = 0;
  for (fit = m_fragments->begin(); fit != m_fragments->end(); ++fit, ++i)
  {
    std::cout << "  " << i << " ";
    fit->dump(m_regionIds);
  }

  std::cout << "\nRegions\n";
  std::set<RegionId> found = m_regionIds.roots();
  std::cout << "Top-level:";
  for (std::set<RegionId>::const_iterator rit = found.begin(); rit != found.end(); ++rit)
  {
    std::cout << " " << m_regionIds.find(*rit);
  }
  std::cout << "\n";
  for (RegionId x = 0; x < static_cast<RegionId>(m_fragments->size() * 2); ++x)
  {
    if (m_regions.find(x) != m_regions.end())
    {
      smtk::shared_ptr<Region> regRec = m_regions[x];
      if (regRec)
      {
        std::cout << "  Region " << x;
        std::cout << " seed frag " << regRec->m_seedFragment << " sense " << regRec->m_seedSense
                  << "\n"; //" has " << regRec->m_innerLoops.size() << " holes.\n";
      }
    }
  }
}

RegionId Neighborhood::traverseLoop(
  OrientedEdges& result,
  std::set<RegionId>& neighborRegions,
  FragmentId fragId,
  bool orientation)
{
  result.clear();
  neighborRegions.clear();

  FragmentId fragStart = fragId;
  bool orientStart = orientation;
  EdgeFragment* frag = &((*m_fragments)[fragId]);
  RegionId lr = m_regionIds.find(frag->ccwRegion(orientStart));
  std::map<smtk::model::Edge, int> already;
  if (m_debugLevel > 1)
  {
    std::cout << "   Traverse Loop [ ";
  }
  do
  {
    frag->mark(orientation, 1);
    bool edgeOrient = !(orientation ^ frag->orientation()); // sense in which edge is used
    if ((already[frag->edge()] & (edgeOrient ? 0x02 : 0x01)) == 0)
    {
      // Only output an edge once (no matter how many of its segments we encounter).
      // TODO: This means that edge-splits required must already have been performed and
      //       the fragments updated with new model-edge UUIDs so there are no
      //       partial edge uses or discontiguous segment uses.
      already[frag->edge()] |= (edgeOrient ? 0x02 : 0x01);
      result.push_back(std::make_pair(frag->edge(), edgeOrient));
    }
    neighborRegions.insert(m_regionIds.find(frag->ccwRegion(!orientation)));
    fragId = frag->nextFragment(orientation);
    orientation = frag->nextFragmentSense(orientation);
    if (m_debugLevel > 1)
    {
      std::cout << " " << fragId << " " << (orientation ? "+" : "-");
    }
    frag = &((*m_fragments)[fragId]);
  } while ((fragId != fragStart || orientation != orientStart) &&
           !frag->marked(orientation) // stop infinity on buggy edges
  );
  if (m_debugLevel > 1)
  {
    std::cout << "]";
    for (auto nr : neighborRegions)
    {
      std::cout << " " << nr;
    }
    std::cout << "\n";
  }
  return lr;
}

void Neighborhood::dumpLoop(
  OrientedEdges& loopEdges,
  RegionId contained,
  std::set<RegionId>& neighborRegions)
{
  std::cout << "Loop around region " << (contained / 2) << (contained % 2 == 0 ? "L" : "U")
            << " with neighbors ";
  std::set<RegionId>::const_iterator rit;
  for (rit = neighborRegions.begin(); rit != neighborRegions.end(); ++rit)
  {
    printRegionId(*rit);
  }
  std::cout << "\n";

  for (OrientedEdges::const_iterator oe = loopEdges.begin(); oe != loopEdges.end(); ++oe)
  {
    if (oe->first.vertices().empty())
    {
      std::cout << "  "
                << "periodic  " << oe->first.name() << "\n";
    }
    else
    {
      model::Vertices endpts = oe->first.vertices();
      double* a = endpts[endpts.size() == 1 ? 0 : (oe->second ? 0 : 1)].coordinates();
      double* b = endpts[endpts.size() == 1 ? 0 : (oe->second ? 1 : 0)].coordinates();
      std::cout << "  " << a[0] << " " << a[1] << " -- " << b[0] << " " << b[1] << "  "
                << oe->first.name() << "\n";
    }
  }
}

// Try generating output faces.
void Neighborhood::dumpRegions2()
{
  FragmentArray::iterator fit;
  std::cout << "\nCreepy crawler\n";
  fit = m_fragments->begin();
  // The left/lower region of the first fragment to be inserted is always exterior
  // to all faces. It corresponds to a "hole" cut in the infinite plane of all fragments.
  RegionId outside = m_regionIds.find(fit->lowerRegion());

  // Traverse every fragment. For each unprocessed coedge, if the region is equivalent
  // to outside, queue the other coedge (if unprocessed) as an output face.
  OrientedEdges loopEdges;
  std::set<RegionId> neighborRegions;
  std::set<RegionId>::const_iterator pit;
  FragmentId fid = 0;
  for (fit = m_fragments->begin(); fit != m_fragments->end(); ++fit, ++fid)
  {
    std::cout << "Fragment " << fid << "-\n";
    if (!fit->marked(false) && m_regionIds.find(fit->ccwRegion(false)) != outside)
    {
      RegionId contained = this->traverseLoop(loopEdges, neighborRegions, fid, false);
      this->dumpLoop(loopEdges, contained, neighborRegions);
    }
    std::cout << "Fragment " << fid << "+\n";
    if (!fit->marked(true) && m_regionIds.find(fit->ccwRegion(true)) != outside)
    {
      RegionId contained = this->traverseLoop(loopEdges, neighborRegions, fid, true);
      this->dumpLoop(loopEdges, contained, neighborRegions);
    }
    std::cout << "\n";
  }

  // Now erase the "visited" marks so we can re-dump if needed:
  for (fit = m_fragments->begin(); fit != m_fragments->end(); ++fit)
  {
    fit->mark(false, 0);
    fit->mark(true, 0);
  }
}

} // namespace polygon
} //namespace session
} // namespace smtk
