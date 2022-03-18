//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/internal/SweepEvent.h"

namespace smtk
{
namespace session
{
namespace polygon
{

bool SweepEvent::operator<(const SweepEvent& other) const
{
  return (
    m_posn.x() < other.point().x() ||
    (m_posn.x() == other.point().x() &&
     (m_posn.y() < other.point().y() ||
      (m_posn.y() == other.point().y() &&
       (m_type < other.type() ||
        (m_type == other.type() &&
         ( // Types match, perform type-specific comparisons:
           (m_type == SEGMENT_START &&
            (m_edge < other.m_edge || (m_edge == other.m_edge && m_indx < other.m_indx))) ||
           (m_type == SEGMENT_END && (m_frag[0] < other.m_frag[0])) ||
           (m_type == SEGMENT_CROSS &&
            (m_frag[0] < other.m_frag[0] ||
             (m_frag[0] == other.m_frag[0] && (m_frag[1] < other.m_frag[1])))))))))));
}

SweepEvent SweepEvent::SegmentStart(
  const internal::Point& p0,
  const internal::Point& p1,
  const smtk::model::Edge& edge,
  int segId)
{
  SweepEvent event;
  event.m_type = SEGMENT_START;
  if (p0.x() < p1.x() || (p0.x() == p1.x() && p0.y() < p1.y()))
  {
    event.m_posn = p0;
    event.m_frag[0] = +1;
  }
  else
  {
    event.m_posn = p1;
    event.m_frag[0] = -1;
  }
  event.m_edge = edge;
  event.m_indx = segId;
  return event;
}

SweepEvent SweepEvent::SegmentEnd(const internal::Point& posn, RegionIdSet::value_type fragId)
{
  SweepEvent event;
  event.m_type = SEGMENT_END;
  event.m_posn = posn;
  event.m_frag[0] = fragId;
  return event;
}

SweepEvent SweepEvent::SegmentCross(
  const internal::Point& crossPos,
  RegionIdSet::value_type fragId0,
  RegionIdSet::value_type fragId1)
{
  SweepEvent event;
  event.m_type = SEGMENT_CROSS;
  event.m_posn = crossPos;
  event.m_frag[0] = fragId0;
  event.m_frag[1] = fragId1;
  return event;
}

bool SweepEvent::RemoveCrossing(SweepEventSet& queue, FragmentId fragId0, FragmentId fragId1)
{
  for (SweepEventSet::iterator it = queue.begin(); it != queue.end(); ++it)
  {
    switch (it->m_type)
    {
      case SEGMENT_START:
        break;
      case SEGMENT_CROSS:
        if (
          static_cast<FragmentId>(it->m_frag[0]) == fragId0 &&
          static_cast<FragmentId>(it->m_frag[1]) == fragId1)
        {
          queue.erase(it);
          return true;
        }
        break;
      case SEGMENT_END:
        if (
          static_cast<FragmentId>(it->m_frag[0]) == fragId0 ||
          static_cast<FragmentId>(it->m_frag[0]) == fragId1)
        { // Terminate early... crossing event must come before either edge ends.
          return false;
        }
        break;
    }
  }
  return false;
}

} // namespace polygon
} //namespace session
} // namespace smtk
