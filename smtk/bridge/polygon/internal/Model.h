#ifndef __smtk_bridge_polygon_internal_model_h
#define __smtk_bridge_polygon_internal_model_h

#include "smtk/bridge/polygon/internal/config.h"
#include "smtk/SharedPtr.h"

class session;

class model : smtkEnableSharedPtr<model>
{
public:
  smtkTypeMacro(model);
  smtkCreateMacro(model);

  template<typename T>
  std::set<idT> createModelEdgesFromPoints(T begin, T end);

protected:
  model();
  ~model();

  friend class session;

  session* m_parent;
  idT m_id;
  double m_scale; // Recommend this be a large composite number w/ factors 2, 3, 5 (e.g., 15360 or 1182720)
  double m_origin[3]; // Base point of plane for model
  double m_xAxis[3]; // Vector whose length should be equal to one "unit" (e.g., m_scale integers long)
  double m_yAxis[3]; // In-plane vector orthogonal to m_xAxis with the same length.
  double m_zAxis[3]; // Normal vector orthogonal to m_xAxis and m_yAxis with the same length.
  pointToVertexPtr m_vertices;
  pointsToEdgeIdT m_edges;
};

#endif // __smtk_bridge_polygon_internal_model_h
