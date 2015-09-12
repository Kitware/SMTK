#ifndef __smtk_bridge_polygon_internal_edge_h
#define __smtk_bridge_polygon_internal_edge_h

#include "smtk/bridge/polygon/internal/config.h"
#include "smtk/SharedPtr.h"

class model;

class edge : smtkEnableSharedPtr<edge>
{
public:
  smtkTypeMacro(edge);
  smtkCreateMacro(edge);

protected:
  edge();
  ~edge();

  friend class model;

  model* m_model;
  idT m_id;
  pointSeqT m_points;
};

#endif // __smtk_bridge_polygon_internal_edge_h
