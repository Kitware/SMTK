//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/internal/Fragment.h"

#include "smtk/bridge/polygon/internal/Util.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

EdgeFragment::EdgeFragment()
{
  for (int i = 0; i < 2; ++i)
    {
    this->m_regionId[i] = -1;
    this->m_marked[i] = 0;
    this->m_next[i] = -1;
    this->m_nsns[i] = true;
    }
}

void EdgeFragment::dump(RegionIdSet& ufind) const
{
  std::cout
    << "  " << this->lo().x()/1182720.0 << " " << this->lo().y()/1182720.0
    << " -- " << this->hi().x()/1182720.0 << " " << this->hi().y()/1182720.0
    << "  " << this->m_edge.name() << ", seg " << this->m_segment
    << " regIds "
    << (ufind.find(this->m_regionId[0]) / 2)
    << (ufind.find(this->m_regionId[0]) % 2 == 0 ? "L" : "U")
    << " "
    << (ufind.find(this->m_regionId[1]) / 2)
    << (ufind.find(this->m_regionId[1]) % 2 == 0 ? "L" : "U")
    << " next " << this->m_next[0] << " " << this->m_next[1]
    << "\n";
}

EdgeFragmentComparator::EdgeFragmentComparator(FragmentArray& frag, SweeplinePosition& startPoint)
  : m_sweptFragments(&frag), m_sweepPoint(&startPoint)
{
}

EdgeFragmentComparator::EdgeFragmentComparator(const EdgeFragmentComparator& other)
  : m_sweptFragments(other.m_sweptFragments), m_sweepPoint(other.m_sweepPoint)
{
}

/**\brief Return true when line a lies to the left of and/or below line b for the current sweepLocation.
  *
  * When the sweep position is at an intersection point of 2 or more lines, we order
  * the lines according to their behavior just to the left (or below, in the case of
  * vertical lines) of the sweep position.
  *
  * As noted by Boissonat and Preparata, predicates for testing relationships between
  * lines require increased-precision operations according to the degree of the
  * equations in the predicate.
  */
bool EdgeFragmentComparator::operator() (FragmentId a, FragmentId b) const
{
  FragmentId fsize = this->fragments()->size();
  // If a is invalid, it is "above" any valid segments:
  if (a >= fsize)
    return false;
  // Now we know a is valid (or we would have returned false).

  // A segment is never less than itself.
  if (b == a)
    return false;

  // Valid segments are always below invalid ones:
  if (b >= fsize)
    return true;

  // Both a and b are valid:
  EdgeFragment& lineA((*this->fragments())[a]);
  EdgeFragment& lineB((*this->fragments())[b]);

#if 1
  // I. Compare y coordinates of fragment lo() points.
  //    Since active fragments do not cross (or have their hi() coordinates
  //    altered to an intersection point) comparing low coordinates is
  //    valid as long as fragments are removed once their hi() neighborhood
  //    has been processed.
  if (lineA.lo().y() < lineB.lo().y())
    return true;
  else if (lineA.lo().y() > lineB.lo().y())
    return false;

  // II. Compare slopes of fragments when they have identical lo().y() coords.
  //     Multiple fragments can start at the same point but shouldn't have the
  //     same slope. Note that because fragments always go from low to high
  //     x coordinates, we do not have to worry about the direction of the
  //     inequality changing because of negative delta_x values in the slope.
  //     (delta_x >= 0 for all fragments).
  internal::HighPrecisionCoord dxA = lineA.hi().x() - lineA.lo().x();
  internal::HighPrecisionCoord dxB = lineB.hi().x() - lineB.lo().x();
  internal::HighPrecisionCoord slopeDiff =
    dxB * (lineA.hi().y() - lineA.lo().y()) -
    dxA * (lineB.hi().y() - lineB.lo().y());

  if (slopeDiff < 0)
    return true;
  else if (slopeDiff > 0)
    return false;
#elif 0
  // A slower but perhaps not-conceptually-flawed test to find which half-space
  // of lineA the lower point of lineB lies in.

  internal::HighPrecisionCoord halfspace;
  if (lineA.hi().x() == lineA.lo().x())
    { // vertical lines are a corner case...
    halfspace = -deltacross2d(lineA.lo(), lineA.hi(), lineA.lo(), lineB.lo());
    }
  else
    {
    halfspace = deltacross2d(lineA.lo(), lineA.hi(), lineA.lo(), lineB.lo());
    }
  if (halfspace != 0)
    {
    return halfspace > 0; // ((A0A1 cross A0B0) dot (0,0,1)) > 0 => A01 under/to-left-of B01
    }

  halfspace = deltacross2d(lineA.lo(), lineA.hi(), lineB.lo(), lineB.hi());
  if (halfspace != 0)
    {
    return halfspace > 0; // B0 is on line A01 but B1 is above it
    }

  // halfspace is 0 again. This can happen when A01 and B01 are parallel.
  // Make sure that the line with its lo (or failing that, hi) coordinate
  // to the lower/left is "less than" the other.
  return (
    lineA.lo().x() < lineB.lo().x() ||
    (lineA.lo().x() == lineB.lo().x() &&
     (lineA.lo().y() < lineB.lo().y() ||
      (lineA.lo().y() == lineB.lo().y() &&
       (lineA.hi().x() < lineB.hi().x() ||
        (lineA.hi().x() == lineB.hi().x() &&
         (lineA.hi().y() < lineB.hi().y() ||
          (lineA.hi().y() == lineB.hi().y())))))))
  ) ? true : false;
#else
  // Choose the fragment with the "lower/lefter" point;
  // we will use its slope to find where it intercepts the other
  // fragment's x-coordinate.
  // Because we need its slope, it is a special case when vertical.

  internal::HighPrecisionCoord dAx = lineA.hi().x() - lineA.lo().x();
  internal::HighPrecisionCoord dBx = lineB.hi().x() - lineB.lo().x();

  internal::HighPrecisionCoord dpAx = this->m_sweepPoint->position().x() - lineA.lo().x();
  internal::HighPrecisionCoord dpBx = this->m_sweepPoint->position().x() - lineB.lo().x();

  /*
  std::cout
    << "  " << a << " <? " << b
    << "  @ " << this->m_sweepPoint->position().x() << " " << this->m_sweepPoint->position().y() << "\n";
    */
  if (dAx == 0) {
    if (dBx == 0) {
      if (dpAx != 0 || dpBx != 0) {
        std::cerr << "Lines a (" << dpAx << ") b (" << dpBx << ") should not be active!\n";
        return false;
      }
      return lineA.lo().y() < lineB.lo().y() || (lineA.lo().y() == lineB.lo().y() && lineA.hi().y() < lineB.hi().y());
    }
    // since A is vertical and must bracket the sweep point to be active,
    // we know the "intersection point" of A with the sweep point is the sweep point.
    // Thus A < B iff pBy < sweep.y()
    internal::HighPrecisionCoord dBy = lineB.hi().y() - lineB.lo().y();
    internal::HighPrecisionCoord pBy = lineB.lo().y() + (dBy/dBx) * dpBx; // FIXME: not enough precision for degree-3 op.
    return this->m_sweepPoint->position().y() < pBy;
  } else if (dBx == 0) {
    // since B is vertical and must bracket the sweep point to be active,
    // we know the "intersection point" of B with the sweep point is the sweep point.
    // Thus A < B iff pAy < sweep.y()
    internal::HighPrecisionCoord dAy = lineA.hi().y() - lineA.lo().y();
    internal::HighPrecisionCoord pAy = lineA.lo().y() + (dAy/dAx) * dpAx; // FIXME: not enough precision for degree-3 op.
    return pAy <= this->m_sweepPoint->position().y(); // equality still implies A < B since A is not vertical and B is.
  }
  // The general case
  internal::HighPrecisionCoord dBy = lineB.hi().y() - lineB.lo().y();
  internal::HighPrecisionCoord pBy = lineB.lo().y() + (dBy/dBx) * dpBx; // FIXME: not enough precision for degree-3 op.
  internal::HighPrecisionCoord dAy = lineA.hi().y() - lineA.lo().y();
  internal::HighPrecisionCoord pAy = lineA.lo().y() + (dAy/dAx) * dpAx; // FIXME: not enough precision for degree-3 op.
  if (pAy != pBy)
    {
    std::cout
      << "                xxx  a " << a << " b " << b
      << "   ( " << this->m_sweepPoint->position().x()/2.31e13 << " " << this->m_sweepPoint->position().y()/2.31e13 << " ) "
      << " pAy @ " << (pAy/2.31e13) << " pBy @ " << (pBy/2.31e13)
      << " test: " << (pAy < pBy ? "a < b   T" : "a >= b F")
      << "\n";
    return pAy < pBy;
    }
  // If we get here, pAy == pBy. This can happen when multiple fragments meet at a neighborhood.
  // That neighborhood may be above or below the sweep point. If above, then A and B are incoming
  // (i.e., the fragments share a common final endpoint) and moving the test position just to
  // the left will return the proper comparison.
  // If below, then A and B are outgoing (i.e., the fragments share a common start point) and
  // moving the test position just to the right will return the proper comparison.
  //
  // We can only have the neighborhood where A and B meet exactly match the test position after
  // we have processed the neighborhood (since we always remove fragments from the active set
  // before processing a neighborhood and only add fragments to the active set after processing
  // a neighborhood).
  bool ptBelowSweep = (pAy < this->m_sweepPoint->position().y());
  bool isOutgoing = (internal::Point(this->m_sweepPoint->position().x(), pAy) == lineA.hi());
  std::cout
    << "                xxx  a " << a << " b " << b
    << "   ( " << this->m_sweepPoint->position().x()/2.31e13 << " " << this->m_sweepPoint->position().y()/2.31e13 << " ) "
    << "intersection " << (ptBelowSweep ? " BELOW " : " ABOVE ")
    << " sweep @ y=" << (pAy/2.31e13)
    << "  mA " << (dAy/2.31e13) << "/" << (dAx/2.31e13)
    << "  mB " << (dBy/2.31e13) << "/" << (dBx/2.31e13)
    << " test "
    << (dAy/2.31e13) << " * " << (dBx/2.31e13)
    << (ptBelowSweep ? " > " : " < ")
    << (dAx/2.31e13) << " * " << (dBy/2.31e13)
    << "?   " << ((ptBelowSweep ? dAy*dBx > dAx*dBy : dAy*dBx < dAx*dBy) ? "T" : "F")
    << (isOutgoing ? " out" : " in")
    << "\n";
  return (ptBelowSweep ^ (isOutgoing)) ?
    dAy*dBx > dAx*dBy :  // equiv to slope(A) < slope(B) in quadrant I
    dAy*dBx < dAx*dBy;   // equiv to slope(A) > slope(B) in quadrant I or slope(A) < slope(B) in quadrant III.
  //return pAy < pBy || (pAy == pBy && dAy*dBx < dAx*dBy); // FIXME: not enough precision for degree-3 op.

#endif

  // We are here because of a problem. No two line segments should be
  // collinear. If they are, one corresponding fragment should be discarded
  // before being added to m_fragments. So, we should complain but not die.
  std::cerr
    << "Error: trying to insert coincident, collinear fragments " << a << " and " << b << " into active fragment tree.";
  return a < b;
}

SweeplinePosition::SweeplinePosition(const internal::Point& posn)
  : m_position(posn)
{
}

SweeplinePosition::SweeplinePosition(const SweeplinePosition& other)
  : m_position(other.m_position)
{
}

/// Advance the sweepline to another position, ignoring invalid points to the left of the current position.
void SweeplinePosition::advance(const internal::Point& pt)
{
  if (
    pt.x() > this->m_position.x() ||
    (pt.x() == this->m_position.x() && pt.y() > this->m_position.y()))
    {
    this->m_position = pt;
    }
  /*
  else
  {
  throw std::string("Can not sweep backwards!");
  }
  */
}

    } // namespace smtk
  } // namespace bridge
} // namespace polygon
