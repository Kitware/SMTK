#ifndef __smtk_bridge_polygon_internal_model_h
#define __smtk_bridge_polygon_internal_model_h

#include "smtk/bridge/polygon/internal/Config.h"
#include "smtk/SharedFromThis.h"

namespace smtk {
  namespace io { class Logger; }
  namespace bridge {
    namespace polygon {
      namespace internal {

class session;

class model : smtkEnableSharedPtr(model)
{
public:
  smtkTypeMacro(model);
  smtkCreateMacro(model);
  model();
  ~model();

  bool computeModelScaleAndNormal(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& y_axis,
    double featureSize,
    smtk::io::Logger& log);

  bool computeModelScaleAndYAxis(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& z_axis,
    double featureSize,
    smtk::io::Logger& log);

  bool computeFeatureSizeAndNormal(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& y_axis,
    long long modelScale,
    smtk::io::Logger& log);

  template<typename T>
  std::set<Id> createModelEdgesFromPoints(T begin, T end);

  double* origin() { return this->m_origin; }
  const double* origin() const { return this->m_origin; }

  double* xAxis() { return this->m_xAxis; }
  const double* xAxis() const { return this->m_xAxis; }

  double* yAxis() { return this->m_yAxis; }
  const double* yAxis() const { return this->m_yAxis; }

  double* zAxis() { return this->m_zAxis; }
  const double* zAxis() const { return this->m_zAxis; }

  double featureSize() const { return this->m_featureSize; }
  double modelScale() const { return this->m_scale; }

  Id id() const { return this->m_id; }
  void setId(const Id& id) { this->m_id = id; }

protected:
  friend class session;

  session* m_parent;
  Id m_id;
  long long m_scale; // Recommend this be a large composite number w/ factors 2, 3, 5 (e.g., 15360, 231000, or 1182720)
  double m_featureSize;
  double m_origin[3]; // Base point of plane for model
  double m_xAxis[3]; // Vector whose length should be equal to one "unit" (e.g., m_scale integers long)
  double m_yAxis[3]; // In-plane vector orthogonal to m_xAxis with the same length.
  double m_zAxis[3]; // Normal vector orthogonal to m_xAxis and m_yAxis with the same length.
  //PointToVertexId m_vertices;
  //pointsToEdgeIdT m_edges;
};

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_polygon_internal_model_h
