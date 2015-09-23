#ifndef __smtk_bridge_polygon_internal_edge_h
#define __smtk_bridge_polygon_internal_edge_h

#include "smtk/bridge/polygon/internal/Entity.h"
#include "smtk/SharedPtr.h"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

class pmodel;

class edge : public entity
{
public:
  smtkTypeMacro(edge);
  smtkCreateMacro(edge);
  smtkSharedFromThisMacro(entity);
  virtual ~edge() { }

  std::size_t pointsSize() const { return this->m_points.size(); }

  PointSeq::const_iterator pointsBegin() const { return this->m_points.begin(); }
  PointSeq::iterator pointsBegin() { return this->m_points.begin(); }

  PointSeq::const_iterator pointsEnd() const { return this->m_points.end(); }
  PointSeq::iterator pointsEnd() { return this->m_points.end(); }

  PointSeq::const_reverse_iterator pointsRBegin() const { return this->m_points.rbegin(); }
  PointSeq::reverse_iterator pointsRBegin() { return this->m_points.rbegin(); }

  PointSeq::const_reverse_iterator pointsREnd() const { return this->m_points.rend(); }
  PointSeq::reverse_iterator pointsREnd() { return this->m_points.rend(); }

protected:
  edge() { }

  friend class pmodel;

  PointSeq m_points;
};

      } // namespace internal
    } // namespace polygon
  }  // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_polygon_internal_edge_h
