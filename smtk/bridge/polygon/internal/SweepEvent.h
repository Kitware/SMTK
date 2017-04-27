//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_SweepEvent_h
#define __smtk_session_polygon_internal_SweepEvent_h

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/internal/Fragment.h"

#include "smtk/model/Edge.h"

#include <map>
#include <set>
#include <vector>

namespace smtk
{
namespace bridge
{
namespace polygon
{

class SweepEvent;

/// Sweep events ordered by their left-, lower-most point coordinates.
typedef std::set<SweepEvent> SweepEventSet;

/**\brief Structure to hold data for a sweepline event (segment start, segment end, segment crossing).
  *
  */
class SMTKPOLYGONSESSION_EXPORT SweepEvent
{
public:
  // NB: Do not modify the order of enums:
  // END must be first so that active edges can be removed before the sweepline position changes.
  // And CROSS must be between the START of segments and their END so that RemoveCrossing can terminate early.
  enum SweepEventType
  {
    SEGMENT_END,
    SEGMENT_START,
    SEGMENT_CROSS
  };

  SweepEventType m_type;
  internal::Point m_posn;
  smtk::model::Edge m_edge; // only used by SEGMENT_START
  int m_indx;               // only used by SEGMENT_START
  RegionIdSet::value_type
    m_frag[2]; // used by SEGMENT_END and SEGMENT_CROSS as frag ID, SEGMENT_START as sense (-1/+1)

  SweepEventType type() const { return this->m_type; }
  const internal::Point& point() const { return this->m_posn; }

  bool operator<(const SweepEvent& other) const;

  static SweepEvent SegmentStart(
    const internal::Point& p0, const internal::Point& p1, const smtk::model::Edge& edge, int segId);

  static SweepEvent SegmentEnd(const internal::Point& posn, RegionIdSet::value_type fragId);

  static SweepEvent SegmentCross(const internal::Point& crossPos, RegionIdSet::value_type fragId0,
    RegionIdSet::value_type fragId1);

  static bool RemoveCrossing(SweepEventSet& queue, FragmentId fragId0, FragmentId fragId1);
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_internal_SweepEvent_h
