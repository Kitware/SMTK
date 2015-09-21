#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/model/Session.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/io/Logger.h"

#include "smtk/bridge/polygon/internal/Model.txx"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

pmodel::pmodel()
  : m_session(NULL), m_featureSize(1e-8)
{
  for (int i = 0; i < 3; ++i)
    {
    this->m_origin[i] = 0.; // Base point of plane for model
    this->m_xAxis[i] = 0.; // Vector whose length should be equal to one "unit" (e.g., m_scale integers long)
    this->m_yAxis[i] = 0.; // In-plane vector orthogonal to m_xAxis with the same length.
    this->m_zAxis[i] = 0.; // Normal vector orthogonal to m_xAxis and m_yAxis with the same length.
    this->m_iAxis[i] = 0.; // Vector whose length should be equal to one "unit" (e.g., 1 integer delta long)
    this->m_jAxis[i] = 0.; // In-plane vector orthogonal to m_iAxis with the same length.
    }
  this->m_xAxis[0] = this->m_yAxis[1] = this->m_zAxis[2] = 1.;
  this->m_iAxis[0] = this->m_jAxis[1] = 1.;
}

pmodel::~pmodel()
{
  // Tis better to have dereferenced and crashed than never to have crashed at all:
  this->m_session = NULL;
}

bool pmodel::computeModelScaleAndNormal(
  std::vector<double>& origin,
  std::vector<double>& x_axis,
  std::vector<double>& y_axis,
  double featureSize,
  smtk::io::Logger& log)
{
  if (featureSize <= 0.)
    {
    smtkErrorMacro(log,
      "Feature size must be positive (not " << featureSize << ").");
    return false;
    }
  this->m_featureSize = featureSize;

  if (origin.size() != 3 || x_axis.size() != 3 || y_axis.size() != 3)
    {
    smtkErrorMacro(log,
      "Vector of length 3 expected for"
      << " origin (" << origin.size() << "),"
      << " x axis (" << x_axis.size() << "), and"
      << " y axis (" << y_axis.size() << ").");
    return false;
    }
  double xl2 = 0., yl2 = 0., zl2 = 0.;
  for (int i = 0; i < 3; ++i)
    {
    this->m_origin[i] = origin[i];
    this->m_xAxis[i] = x_axis[i];
    this->m_yAxis[i] = y_axis[i];
    xl2 += x_axis[i] * x_axis[i];
    yl2 += y_axis[i] * y_axis[i];
    this->m_zAxis[i] =
      x_axis[(i+1) % 3] * y_axis[(i+2) % 3] -
      x_axis[(i+2) % 3] * y_axis[(i+1) % 3];
    zl2 += this->m_zAxis[i] * this->m_zAxis[i];
    }
  if (xl2 < 1e-16 || yl2 < 1e-16 || zl2 < 1e-16)
    {
    smtkErrorMacro(log,
      "Vectors of non-zero L2 norm required for "
      << " x (" << xl2 << "),"
      << " y (" << yl2 << "), and"
      << " z (" << zl2 << ") axes.");
    return false;
    }
  xl2 = sqrt(xl2);
  yl2 = sqrt(yl2);
  zl2 = sqrt(zl2);
  // Scale each feature size to be 231000 integer quanta.
  this->m_scale = 231000 / featureSize;
  // Make the world (x,y) axes unit length and the (i,j) axes one quantum in length:
  for (int i = 0; i < 3; ++i)
    {
    this->m_xAxis[i] /= xl2;
    this->m_yAxis[i] /= yl2;
    this->m_zAxis[i] /= zl2;
    this->m_iAxis[i] = this->m_xAxis[i] / this->m_scale;
    this->m_jAxis[i] = this->m_yAxis[i] / this->m_scale;
    }
  return true;
}

bool pmodel::computeModelScaleAndYAxis(
  std::vector<double>& origin,
  std::vector<double>& x_axis,
  std::vector<double>& z_axis,
  double featureSize,
  smtk::io::Logger& log)
{
  if (featureSize <= 0.)
    {
    smtkErrorMacro(log,
      "Feature size must be positive (not " << featureSize << ").");
    return false;
    }
  this->m_featureSize = featureSize;

  if (origin.size() != 3 || x_axis.size() != 3 || z_axis.size() != 3)
    {
    smtkErrorMacro(log,
      "Vector of length 3 expected for"
      << " origin (" << origin.size() << "),"
      << " x axis (" << x_axis.size() << "), and"
      << " z axis (" << z_axis.size() << ").");
    return false;
    }
  double xl2 = 0., yl2 = 0., zl2 = 0.;
  for (int i = 0; i < 3; ++i)
    {
    this->m_origin[i] = origin[i];
    this->m_xAxis[i] = x_axis[i];
    this->m_zAxis[i] = z_axis[i];
    xl2 += x_axis[i] * x_axis[i];
    zl2 += z_axis[i] * z_axis[i];
    this->m_yAxis[i] =
      z_axis[(i+1) % 3] * x_axis[(i+2) % 3] -
      z_axis[(i+2) % 3] * x_axis[(i+1) % 3];
    yl2 += this->m_yAxis[i] * this->m_yAxis[i];
    }
  if (xl2 < 1e-16 || yl2 < 1e-16 || zl2 < 1e-16)
    {
    smtkErrorMacro(log,
      "Vectors of non-zero L2 norm required for "
      << " x (" << xl2 << "),"
      << " y (" << yl2 << "), and"
      << " z (" << zl2 << ") axes.");
    return false;
    }
  xl2 = sqrt(xl2);
  yl2 = sqrt(yl2);
  zl2 = sqrt(zl2);
  // Make the axes one feature-size in length:
  this->m_scale = 231000 / featureSize;
  // Make the world (x,y) axes unit length and the (i,j) axes one quantum in length:
  for (int i = 0; i < 3; ++i)
    {
    this->m_xAxis[i] /= xl2;
    this->m_yAxis[i] /= yl2;
    this->m_zAxis[i] /= zl2;
    this->m_iAxis[i] = this->m_xAxis[i] / this->m_scale;
    this->m_jAxis[i] = this->m_yAxis[i] / this->m_scale;
    }
  return true;
}

bool pmodel::computeFeatureSizeAndNormal(
  std::vector<double>& origin,
  std::vector<double>& x_axis,
  std::vector<double>& y_axis,
  long long modelScale,
  smtk::io::Logger& log)
{
  if (origin.size() != 3 || x_axis.size() != 3 || y_axis.size() != 3)
    {
    smtkErrorMacro(log,
      "Vector of length 3 expected for"
      << " origin (" << origin.size() << "),"
      << " x axis (" << x_axis.size() << "), and"
      << " y axis (" << y_axis.size() << ").");
    return false;
    }
  double xl2 = 0., yl2 = 0., zl2 = 0.;
  for (int i = 0; i < 3; ++i)
    {
    this->m_origin[i] = origin[i];
    this->m_xAxis[i] = x_axis[i];
    this->m_yAxis[i] = y_axis[i];
    xl2 += x_axis[i] * x_axis[i];
    yl2 += y_axis[i] * y_axis[i];
    this->m_zAxis[i] =
      x_axis[(i+1) % 3] * y_axis[(i+2) % 3] -
      x_axis[(i+2) % 3] * y_axis[(i+1) % 3];
    zl2 += this->m_zAxis[i] * this->m_zAxis[i];
    }
  if (xl2 < 1e-16 || yl2 < 1e-16 || zl2 < 1e-16)
    {
    smtkErrorMacro(log,
      "Vectors of non-zero L2 norm required for "
      << " x (" << xl2 << "),"
      << " y (" << yl2 << "), and"
      << " z (" << zl2 << ") axes.");
    return false;
    }
  xl2 = sqrt(xl2);
  yl2 = sqrt(yl2);
  zl2 = sqrt(zl2);
  this->m_scale = modelScale;
  this->m_featureSize = 1.0;
  for (int i = 0; i < 3; ++i)
    {
    this->m_xAxis[i] /= xl2;
    this->m_yAxis[i] /= yl2;
    this->m_zAxis[i] /= zl2;
    this->m_iAxis[i] = this->m_xAxis[i] / this->m_scale;
    this->m_jAxis[i] = this->m_yAxis[i] / this->m_scale;
    }
  return true;
}

smtk::model::Vertices pmodel::findOrAddModelVertices(
  smtk::model::ManagerPtr mgr,
  const std::vector<double>& points,
  int numCoordsPerPt)
{
  smtk::model::Vertices vertices;
  std::vector<double>::const_iterator it = points.begin();
  long long i = 0;
  for (i = 0; it != points.end(); it += numCoordsPerPt, i += numCoordsPerPt)
    {
    Point projected = this->projectPoint(it, it + numCoordsPerPt);
    vertices.push_back(this->findOrAddModelVertex(mgr, projected));
    }
  return vertices;
}

smtk::model::Vertex pmodel::findOrAddModelVertex(
  smtk::model::ManagerPtr mgr,
  const Point& pt)
{
  PointToVertexId::const_iterator pit = this->m_vertices.find(pt);
  if (pit != this->m_vertices.end())
    return smtk::model::Vertex(mgr, pit->second);

  // Add a model vertex to the manager
  smtk::model::Vertex v = mgr->addVertex();
  // Add a coordinate-map lookup to local storage:
  this->m_vertices[pt] = v.entity();
  vertex::Ptr vi = vertex::create();
  vi->m_coords = pt;
  vi->setParent(this);
  this->m_session->addStorage(v.entity(), vi);
  // Figure out the floating-point approximation for our discretized coordinate
  // and add it to the tessellation for the new model vertex:
  double snappedPt[3];
  this->liftPoint(pt, snappedPt);
  smtk::model::Tessellation tess;
  tess.addPoint(snappedPt);
  v.setTessellation(&tess);
  // TODO: Add the vertex to the model as a free cell?
  return v;
}

Point pmodel::edgeTestPoint(const Id& edgeId, bool edgeEndPt) const
{
  edge::Ptr e = this->m_session->findStorage<edge>(edgeId);
  if (e)
    {
    if (edgeEndPt == true)
      { // Return test point near *last* vertex of forwards edge.
      PointSeq::const_reverse_iterator it = e->pointsRBegin();
      ++it; // Advance from endpoint by 1 so we are not coincident to the endpoint.
      return *it;
      }
    else
      { // Return test point near *first* vertex of forwards edge.
      PointSeq::const_iterator it = e->pointsBegin();
      ++it;
      return *it;
      }
    }
  return Point(); // FIXME: Do something better? detectable?
}

Id pmodel::pointId(const Point& p) const
{
  PointToVertexId::const_iterator it = this->m_vertices.find(p);
  if (it == this->m_vertices.end())
    return Id();
  return it->second;
}

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
