//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Edge.h"

#include "smtk/model/Session.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Tessellation.h"
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
  this->m_scale = static_cast<long long>(231000 / featureSize);
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
  this->m_scale = static_cast<long long>(231000 / featureSize);
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

/**\brief Add a vertex to the model manager.
  *
  * This creates a vertex record in the model manager and adds its tessellation.
  * It also adds the integer coordinates of the point to
  * the internal model's data (this instance).
  * This does **not** relate the vertex record in the model manager to a parent model
  * or owning geometric entity (such as an edge, face, or volume).
  */
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

  // Add vertex to model as a free cell (which it is until it bounds something).
  smtk::model::Model self(mgr, this->id());
  self.embedEntity(v);

  v.assignDefaultName();
  return v;
}

/**\brief Split the model edge with the given \a edgeId at the given \a coords.
  *
  * The point coordinates need not lie precisely on the edge, but are assumed not
  * to cause the split edge to intersect (or miss) any faces or edges the original
  * edge did not.
  *
  * If the point coordinates already specify a model vertex, this method returns false.
  *
  * FIXME: Return new edge Ids and new vertex Id.
  */
bool pmodel::splitModelEdgeAtPoint(smtk::model::ManagerPtr mgr, const Id& edgeId, const std::vector<double>& coords)
{
  Point pt = this->projectPoint(coords.begin(), coords.end());
  if (this->pointId(pt))
    return false; // Point is already a model vertex.
  // TODO: Find point on edge closest to pt? Need to find where to insert model vertex?
  smtk::model::Vertex v = this->findOrAddModelVertex(mgr, pt);
  return this->splitModelEdgeAtModelVertex(mgr, edgeId, v.entity());
}

/** Split the model edge at one of its points that has been promoted to a model vertex.
  *
  * Since model edges are not allowed to have model vertices along their interior,
  * this method should not be exposed as a public operator but may be used internally
  * when performing other operations.
  */
bool pmodel::splitModelEdgeAtModelVertex(smtk::model::ManagerPtr mgr, const Id& edgeId, const Id& vertexId)
{
  // Look up edge
  edge::Ptr edg = this->session()->findStorage<edge>(edgeId);
  // Look up vertex
  vertex::Ptr vrt = this->session()->findStorage<vertex>(vertexId);
  if (!edg || !vrt)
    return false;
  PointSeq::const_iterator split;
  for (split = edg->pointsBegin(); split != edg->pointsEnd(); ++split)
    {
    if (vrt->point() == *split)
      { // Split the edge at this location by creating 2 new edges and deleting this edge.
      return this->splitModelEdgeAtModelVertex(mgr, edg, vrt, split);
      }
    }
  // Edge did not contain model vertex in its sequence.
  return false;
}

typedef std::vector<std::pair<size_t, Segment> > SegmentSplitsT;

#if 0
static void DumpSegSplits(
  const char* msg,
  SegmentSplitsT::iterator a,
  SegmentSplitsT::iterator b)
{
  if (msg)
    std::cout << msg << "\n";

  SegmentSplitsT::iterator ii;
  for (ii = a; ii != b; ++ii)
    {
    std::cout << "  " << ii->first
      << " : " << ii->second.low().x() << " " << ii->second.low().y()
      << " -- " << ii->second.high().x() << " " << ii->second.high().y()
      << "\n";
    }
}

static void DumpPointSeq(
  const char* msg,
  PointSeq::const_iterator a,
  PointSeq::const_iterator b,
  PointSeq::const_iterator loc)
{
  if (msg)
    std::cout << msg << "\n";

  PointSeq::const_iterator ii;
  for (ii = a; ii != b; ++ii)
    {
    std::cout
      << "  " << ii->x() << " " << ii->y()
      << (ii == loc ? " *\n" : "\n");
    }
}
#endif // 0

bool pmodel::splitModelEdgeAtModelVertex(
  smtk::model::ManagerPtr mgr, edge::Ptr edgeToSplit, vertex::Ptr splitPoint, PointSeq::const_iterator location)
{
  size_t npts;
  if (
    !edgeToSplit ||
    !splitPoint ||
    (npts = edgeToSplit->pointsSize()) < 3 ||
    location == edgeToSplit->pointsBegin() ||
    *location == *edgeToSplit->pointsRBegin())
    return false;

  //DumpPointSeq("Split Edge", edgeToSplit->pointsBegin(), edgeToSplit->pointsEnd(), location);
  PointSeq::const_iterator it;
  size_t n = 0;
  if (
    *edgeToSplit->pointsBegin() == *edgeToSplit->pointsRBegin() && // edge is periodic
    this->m_vertices.find(*edgeToSplit->pointsBegin()) == this->m_vertices.end()) // edge has no model vertices (those must be at start+end)
    {
    // Edge had no model vertices and we are being asked to split it at a
    // point interior to its sequence; reorder the sequence so the split
    // point is at the beginning+end of the sequence.
    //
    // Note that this is kinda futzy becase periodic edges repeat one point
    // at their beginning and end... we have to remove the duplicate before
    // splicing and then add a duplicate of the new start point to the end
    // of the list.
    std::cout << "Edge is periodic, split is interior!!!!\n";
    it = edgeToSplit->pointsEnd();
    --it;
    edgeToSplit->m_points.erase(it);
    it = edgeToSplit->pointsBegin();
    edgeToSplit->m_points.splice(it, edgeToSplit->m_points, location, edgeToSplit->pointsEnd());
    edgeToSplit->m_points.insert(edgeToSplit->pointsEnd(), *location);

    // Regenerate the tessellation for the edge with the new point order:
    //mgr->erase(edgeToSplit->id(), smtk::model::SESSION_TESSELLATION);
    smtk::model::Edge modelEdge(mgr, edgeToSplit->id());
    this->addEdgeTessellation(modelEdge, edgeToSplit);
    return true;
    }
  SegmentSplitsT segs;
  SegmentSplitsT::iterator segSplit;
  PointSeq::const_iterator prev = edgeToSplit->pointsBegin();
  segs.reserve(npts - 1); // Preallocation to prevent vector from reallocating and invalidating segSplit iterator.
  it = prev;
  for (++it; it != edgeToSplit->pointsEnd(); ++it, ++n)
    {
    segs.push_back(std::pair<size_t,Segment>(n, Segment(*prev, *it)));
    if (prev == location)
      segSplit = segs.begin() + n;
    prev = it;
    }
  //DumpSegSplits("Pre-split: ", segs.begin(), segs.end());

  // Remove edgeToSplit from its endpoint vertices so that creation
  // of new edges can succeed (otherwise it will fail when trying
  // to insert a coincident edge at the existing edge endpoints).
  std::pair<Id,Id> adjacentFaces = this->removeModelEdgeFromEndpoints(mgr, edgeToSplit);
  (void)adjacentFaces;

  //DumpSegSplits("Split A: ", segs.begin(), segSplit);
  //DumpSegSplits("Split B: ", segSplit, segs.end());
  mgr->erase(edgeToSplit->id());
  // Now we can create the new model edges.
  smtk::model::Edge eA = this->createModelEdgeFromSegments(mgr, segs.begin(), segSplit);
  smtk::model::Edge eB = this->createModelEdgeFromSegments(mgr, segSplit, segs.end());
  //std::cout << "Split into " << eA.name() << " " << eB.name() << "\n";

  // TODO: Fix face adjacency information (face relations and at all 3 vertices)
  //       Fix face loops by replacing old edge with new edges.
  //       Create/fix vertex use at model vertex? No, this should be done wherever the vertex is created.
  return true;
}

// TODO: Remove edgeToSplit so that creation can succeed (otherwise
//       it will fail when trying to insert a coincident edge at the
//       existing edge endpoints.
std::pair<Id,Id> pmodel::removeModelEdgeFromEndpoints(smtk::model::ManagerPtr mgr, EdgePtr edg)
{
  std::pair<Id,Id> result;
  if (!edg || !mgr)
    return result;

  Id epids[2];
  epids[0] = this->pointId(*edg->pointsBegin());
  epids[1] = this->pointId(*edg->pointsRBegin());
  if (!epids[0])
    return result; // edge is periodic and has no model vertices

  // Iterate over both endpoints (which may be the same, but in that
  // case we still need to remove both edge-incidence records).
  for (int i = 0; i < 2; ++i)
    {
    vertex::Ptr endpt = this->session()->findStorage<vertex>(epids[i]);
    vertex::incident_edges::iterator where;
    for (where = endpt->edgesBegin(); where != endpt->edgesEnd(); ++where)
      {
      if (where->edgeId() == edg->id())
        { // found the incident edge.
        if (!result.first)
          {
          vertex::incident_edges::iterator tmp = where;
          result.first =
            (where == endpt->edgesBegin() ?
             endpt->edgesRBegin()->clockwiseFaceId() :
             (--tmp)->clockwiseFaceId());
          result.second = where->clockwiseFaceId();
          }
        endpt->removeEdgeAt(where);
        }
      }
    }
  return result;
}

/**\brief Return the point closest to one of an edge's endpoints.
  *
  * Returns the point nearest but not at the tail end of the edge
  * (using the edge's forward ordering of points)
  * when \a edgeEndPt is true.
  * Returns the point nearest but not at the front end of the edge
  * when \a edgeEndPt is false.
  *
  * This method is used to order edges in the immediate neighborhood
  * of model vertices (which may only be endpoints, not interior points).
  */
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

void pmodel::addEdgeTessellation(smtk::model::Edge& edgeRec, internal::edge::Ptr edgeData)
{
  if (!edgeRec.isValid() || !edgeData)
    return;

  smtk::model::Manager::Ptr mgr = edgeRec.manager();
  smtk::model::Tessellation empty;
  UUIDsToTessellations::iterator tessIt =
    mgr->setTessellation(edgeRec.entity(), empty);

  // Now populate the tessellation in place.
  PointSeq::const_iterator ptIt;
  std::vector<double> coords(3);
  std::size_t numPts = edgeData->pointsSize();
  std::vector<int> conn;
  conn.reserve(numPts + 2);
  conn.push_back(smtk::model::TESS_POLYLINE);
  conn.push_back(static_cast<int>(numPts));
  for (ptIt = edgeData->pointsBegin(); ptIt != edgeData->pointsEnd(); ++ptIt)
    {
    this->liftPoint(*ptIt, coords.begin());
    conn.push_back(tessIt->second.addCoords(&coords[0]));
    }
  tessIt->second.insertCell(0, conn);
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
