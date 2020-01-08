//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_edge_h
#define __smtk_session_polygon_internal_edge_h

#include "smtk/SharedPtr.h"
#include "smtk/session/polygon/internal/Entity.h"

#ifndef _WIN32
#include <sys/types.h> // for ssize_t
#else
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#if defined(_MSC_VER) && _MSC_VER < 1900
typedef SIZE_T size_t;
#endif
#endif // _WIN32

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{

class pmodel;

class SMTKPOLYGONSESSION_EXPORT edge : public entity
{
public:
  smtkTypeMacro(edge);
  smtkCreateMacro(edge);
  smtkSharedFromThisMacro(entity);
  virtual ~edge() {}

  std::size_t pointsSize() const { return m_points.size(); }

  PointSeq::const_iterator pointsBegin() const { return m_points.begin(); }
  PointSeq::iterator pointsBegin() { return m_points.begin(); }

  PointSeq::const_iterator pointsEnd() const { return m_points.end(); }
  PointSeq::iterator pointsEnd() { return m_points.end(); }

  PointSeq::const_reverse_iterator pointsRBegin() const { return m_points.rbegin(); }
  PointSeq::reverse_iterator pointsRBegin() { return m_points.rbegin(); }

  PointSeq::const_reverse_iterator pointsREnd() const { return m_points.rend(); }
  PointSeq::reverse_iterator pointsREnd() { return m_points.rend(); }

  bool pointsOfSegment(ssize_t idx, Point& lo, Point& hi) const
  {
    if (idx < 0 || idx >= static_cast<ssize_t>(m_points.size()))
      return false;

    PointSeq::const_iterator it = this->pointsBegin();
    for (ssize_t i = 0; i <= idx && it != this->pointsEnd(); ++i, ++it, lo = hi)
      hi = *it;
    hi = *it;
    return true;
  }

  PointSeq& points() { return m_points; }
  const PointSeq& points() const { return m_points; }

protected:
  edge() {}

  friend class pmodel;

  PointSeq m_points;
};

} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk

#endif // __smtk_session_polygon_internal_edge_h
