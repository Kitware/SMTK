//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/internal/Fragment.h"

#include "smtk/session/polygon/internal/Util.h"

namespace smtk
{
namespace session
{
namespace polygon
{

EdgeFragment::EdgeFragment()
{
  for (int i = 0; i < 2; ++i)
  {
    m_regionId[i] = -1;
    m_marked[i] = 0;
    m_next[i] = -1;
    m_nsns[i] = true;
  }
}

void EdgeFragment::dump(RegionIdSet& ufind) const
{
  std::cout << "  " << this->lo().x() << " " << this->lo().y() << "    " << this->hi().x() << " "
            << this->hi().y() << "  " << m_edge.name() << ", seg " << m_segment << " regIds "
            << (ufind.find(m_regionId[0]) / 2) << (ufind.find(m_regionId[0]) % 2 == 0 ? "L" : "U")
            << " " << (ufind.find(m_regionId[1]) / 2)
            << (ufind.find(m_regionId[1]) % 2 == 0 ? "L" : "U") << " next " << m_next[0] << " "
            << m_next[1] << "\n";
}

EdgeFragmentComparator::EdgeFragmentComparator(FragmentArray& frag, SweeplinePosition& startPoint)
  : m_sweptFragments(&frag)
  , m_sweepPoint(&startPoint)
{
}

EdgeFragmentComparator::EdgeFragmentComparator(const EdgeFragmentComparator& other)

  = default;

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
bool EdgeFragmentComparator::operator()(FragmentId a, FragmentId b) const
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

  /*
  std::cout
    << "        "
    << a << " (" << lineA.lo().x() << " " << lineA.lo().y() << " -- " << lineA.hi().x() << " " << lineA.hi().y() << ") <? "
    << b << " (" << lineB.lo().x() << " " << lineB.lo().y() << " -- " << lineB.hi().x() << " " << lineB.hi().y() << ")";
    */
  // Simple accept/reject:
  internal::Coord aymax = lineA.lo().y() > lineA.hi().y() ? lineA.lo().y() : lineA.hi().y();
  internal::Coord bymin = lineB.lo().y() < lineB.hi().y() ? lineB.lo().y() : lineB.hi().y();
  if (aymax <= bymin)
  {
    //std::cout << ": Y  1\n";
    return true;
  }
  internal::Coord aymin = lineA.lo().y() < lineA.hi().y() ? lineA.lo().y() : lineA.hi().y();
  internal::Coord bymax = lineB.lo().y() > lineB.hi().y() ? lineB.lo().y() : lineB.hi().y();
  if (aymin >= bymax)
  {
    //std::cout << ": N  2\n";
    return false;
  }

  internal::HighPrecisionCoord dxA =
    static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
      lineA.hi().x() - lineA.lo().x());
  internal::HighPrecisionCoord dxB =
    static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
      lineB.hi().x() - lineB.lo().x());

  if (dxA == 0)
  {
    if (dxB == 0)
    { // Really shouldn't be possible unless a, b share a common vertex where they meet, which the tests above should catch.
      //std::cout << "FRAB! a(" << a << ") and b(" << b << ") are vertical and overlap!\n";
      return a < b;
    }
    //std::cout << "FRAB! a(" << a << ") is vertical and b(" << b << ") intersects its interior!\n";
    // Count a as below b when b@x* is below the sweep point (x*,y*) on the theory that a intersects
    // the sweep point exactly. Immediately to the right of the sweep point, a is vertical while b
    // is not, so a > b.
    internal::HighPrecisionCoord dxBs =
      static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
        m_sweepPoint->position().x() - lineB.lo().x());
    // FIXME: Be more careful about precision here:
    internal::HighPrecisionCoord ybs =
      static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
        lineB.lo().y() + (dxBs / dxB) * (lineB.hi().y() - lineB.lo().y()));
    //std::cout << ": " << ybs << " > " << m_sweepPoint->position().y() << "  3\n";
    return ybs > m_sweepPoint->position().y();
  }

  // A is not vertical but B may be.
  if (dxB == 0)
  {
    // Count a as below b when a@x* is at or below the sweep point (x*,y*) on the theory that b intersects
    // the sweep point exactly. Immediately to the right of the sweep point, b is vertical while a
    // is not, so a < b.
    //
    internal::HighPrecisionCoord dxAs =
      static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
        m_sweepPoint->position().x() - lineA.lo().x());
    // FIXME: Be more careful about precision here:
    internal::HighPrecisionCoord yas =
      static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
        lineA.lo().y() + (dxAs / dxA) * (lineA.hi().y() - lineA.lo().y()));
    //std::cout << ": " << yas << " <= " << m_sweepPoint->position().y() << "  4\n";
    return yas <= m_sweepPoint->position().y();
  }

  // Neither a nor b are vertical.
  // See how they behave at or just up/right of the sweep point.
  internal::HighPrecisionCoord dxAs =
    static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
      m_sweepPoint->position().x() - lineA.lo().x());
  internal::HighPrecisionCoord yas =
    static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
      lineA.lo().y() + (dxAs / dxA) * (lineA.hi().y() - lineA.lo().y()));
  internal::HighPrecisionCoord dxBs =
    static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
      m_sweepPoint->position().x() - lineB.lo().x());
  internal::HighPrecisionCoord ybs =
    static_cast<smtk::session::polygon::internal::HighPrecisionCoord>(
      lineB.lo().y() + (dxBs / dxB) * (lineB.hi().y() - lineB.lo().y()));
  if (yas != ybs)
  {
    //std::cout << ": " << yas << " < " << ybs << "  5\n";
    return yas < ybs;
  }

  internal::HighPrecisionCoord slopeDiff = m_sweepPoint->position().x() > lineA.lo().x()
    ? dxB * (lineA.hi().y() - lineA.lo().y()) - dxA * (lineB.hi().y() - lineB.lo().y())
    : dxA * (lineB.hi().y() - lineB.lo().y()) - dxB * (lineA.hi().y() - lineA.lo().y());

  //std::cout << ": " << slopeDiff << " < 0  6\n";
  return slopeDiff > 0;
}

SweeplinePosition::SweeplinePosition(const internal::Point& posn)
  : m_position(posn)
{
}

SweeplinePosition::SweeplinePosition(const SweeplinePosition& other)

  = default;

/// Advance the sweepline to another position, ignoring invalid points to the left of the current position.
void SweeplinePosition::advance(const internal::Point& pt)
{
  if (pt.x() > m_position.x() || (pt.x() == m_position.x() && pt.y() > m_position.y()))
  {
    m_position = pt;
  }
  /*
  else
  {
  throw std::string("Can not sweep backwards!");
  }
  */
}

} // namespace polygon
} // namespace session
} // namespace smtk
