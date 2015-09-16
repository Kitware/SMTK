#ifndef __smtk_bridge_polygon_internal_edge_h
#define __smtk_bridge_polygon_internal_edge_h

#include "smtk/bridge/polygon/internal/Entity.h"
#include "smtk/SharedPtr.h"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

class model;

class edge : public entity
{
public:
  smtkTypeMacro(edge);
  smtkCreateMacro(edge);
  smtkSharedFromThisMacro(entity);
  virtual ~edge();

protected:
  edge();

  friend class model;

  idT m_id;
  idT m_uses[2];
  pointSeqT m_points;
};

      } // namespace internal
    } // namespace polygon
  }  // namespace bridge
} // namespace smtk
#endif // __smtk_bridge_polygon_internal_edge_h
