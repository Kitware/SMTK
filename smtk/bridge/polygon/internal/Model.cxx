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

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

#include "smtk/io/Logger.h"
//#include "smtk/io/ExportJSON.h"

#include "smtk/bridge/polygon/internal/Model.txx"

using namespace smtk::model;

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

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
  // Scale each feature size to be 231 integer quanta. (NB: 231 = 3 * 7 * 11, some nice primes)
  this->m_scale = static_cast<long long>(231.0 / featureSize);
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
  this->m_scale = static_cast<long long>(231.0 / featureSize);
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

bool pmodel::restoreModel(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& y_axis,
    std::vector<double>& z_axis,
    std::vector<double>& i_axis,
    std::vector<double>& j_axis,
    double featureSize,
    long long modelScale)
{
  if (
    origin.size() != 3 ||
    x_axis.size() != 3 ||
    y_axis.size() != 3 ||
    z_axis.size() != 3 ||
    i_axis.size() != 3 ||
    j_axis.size() != 3)
    {
    std::cerr
      << "Vector of length 3 expected for"
      << " origin (" << origin.size() << "),"
      << " x axis (" << x_axis.size() << "),"
      << " y axis (" << y_axis.size() << "),"
      << " z axis (" << z_axis.size() << "),"
      << " i axis (" << i_axis.size() << "),"
      << " and"
      << " j axis (" << j_axis.size() << ").";
    return false;
    }
  if (featureSize <= 0.)
    {
    std::cerr << "Specified feature size (" << featureSize << ") is not positive.";
    return false;
    }
  if (modelScale <= 0)
    {
    std::cerr << "Specified model scale (" << modelScale << ") is not positive.";
    return false;
    }

  for (int i = 0; i < 3; ++i)
    {
    this->m_origin[i] = origin[i];
    this->m_xAxis[i] = x_axis[i];
    this->m_yAxis[i] = y_axis[i];
    this->m_zAxis[i] = z_axis[i];
    this->m_iAxis[i] = i_axis[i];
    this->m_jAxis[i] = j_axis[i];
    }
  this->m_scale = modelScale;
  this->m_featureSize = featureSize;
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
  vi->setParent(this);
  vi->setId(v.entity());
  vi->m_coords = pt;
  this->m_session->addStorage(v.entity(), vi);
  // Figure out the floating-point approximation for our discretized coordinate
  // and add it to the tessellation for the new model vertex:
  this->addVertTessellation(v, vi);

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
bool pmodel::splitModelEdgeAtPoint(
  smtk::model::ManagerPtr mgr,
  const Id& edgeId,
  const std::vector<double>& coords,
  smtk::model::EntityRefs& created,
  smtk::model::EntityRefs& modified)
{
  Point pt = this->projectPoint(coords.begin(), coords.end());
  if (this->pointId(pt))
    {
    smtkWarningMacro(this->session()->log(), "Point is already a model vertex.");
    return false; // Point is already a model vertex.
    }
  // TODO: Find point on edge closest to pt? Need to find where to insert model vertex?
  smtk::model::Vertex v = this->findOrAddModelVertex(mgr, pt);
  return this->splitModelEdgeAtModelVertex(mgr, edgeId, v.entity(), created, modified);
}

/** Split the model edge at one of its points that has been promoted to a model vertex.
  *
  * Since model edges are not allowed to have model vertices along their interior,
  * this method should not be exposed as a public operator but may be used internally
  * when performing other operations.
  */
bool pmodel::splitModelEdgeAtModelVertex(
  smtk::model::ManagerPtr mgr,
  const Id& edgeId,
  const Id& vertexId,
  smtk::model::EntityRefs& created,
  smtk::model::EntityRefs& modified)
{
  // Look up edge
  edge::Ptr edg = this->session()->findStorage<edge>(edgeId);
  // Look up vertex
  vertex::Ptr vrt = this->session()->findStorage<vertex>(vertexId);
  if (!edg || !vrt)
    return false;
  PointSeq::iterator split;
  Coord maxDelta = static_cast<Coord>(this->m_featureSize * this->m_scale);
  for (split = edg->pointsBegin(); split != edg->pointsEnd(); ++split)
    {
    if (
      std::abs(vrt->point().x() - split->x()) < maxDelta &&
      std::abs(vrt->point().y() - split->y()) < maxDelta)
      { // Split the edge at this location by creating 2 new edges and deleting this edge.
      if (split == edg->pointsBegin() && *split == *edg->pointsRBegin())
        { // User wants to split periodic edge... overwrite both front and back with vertex point
        *split = vrt->point();
        *(edg->pointsRBegin()) = vrt->point();
        }
      else
        { // Overwrite split point with vertex point.
        *split = vrt->point();
        }
      return this->splitModelEdgeAtModelVertex(mgr, edg, vrt, split, created, modified);
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

/**\brief An internal edge split operation.
  *
  * This variant requires the point along the model vertex to already have been promoted.
  * It takes an iterator into the original model edge's sequence of points and creates
  * new edges.
  */
bool pmodel::splitModelEdgeAtModelVertex(
  smtk::model::ManagerPtr mgr,
  edge::Ptr edgeToSplit,
  vertex::Ptr splitPoint,
  PointSeq::const_iterator location,
  smtk::model::EntityRefs& created,
  smtk::model::EntityRefs& modified)
{
  std::vector<vertex::Ptr> splitPoints;
  std::vector<PointSeq::const_iterator> locations;
  splitPoints.push_back(splitPoint);
  locations.push_back(location);
  return this->splitModelEdgeAtModelVertices(
    mgr, edgeToSplit, splitPoints, locations, created, modified);
}

bool pmodel::splitModelEdgeAtModelVertices(
  smtk::model::ManagerPtr mgr,
  edge::Ptr edgeToSplit,
  std::vector<vertex::Ptr>& splitPointsInEdgeOrder,
  std::vector<PointSeq::const_iterator>& locationsInEdgeOrder,
  smtk::model::EntityRefs& created,
  smtk::model::EntityRefs& modified)
{
  (void)modified;
  size_t npts;
  if (
    !edgeToSplit ||
    (npts = edgeToSplit->pointsSize()) < 2 ||
    locationsInEdgeOrder.empty() ||
    splitPointsInEdgeOrder.size() != locationsInEdgeOrder.size())
    return false;

  //DumpPointSeq("Split Edge", edgeToSplit->pointsBegin(), edgeToSplit->pointsEnd(), location);
  size_t n = 0;
  smtk::model::Edge modelEdge(mgr, edgeToSplit->id());
  smtk::model::Vertices allVertices;
  smtk::model::Vertex finalModelVert;
  bool isPeriodic = (*edgeToSplit->pointsBegin() == *edgeToSplit->pointsRBegin());
  //bool noModelVertices = (this->m_vertices.find(*edgeToSplit->pointsBegin()) == this->m_vertices.end());
  bool noModelVertices = modelEdge.vertices().empty();
  if (isPeriodic && noModelVertices)
    {
    // Edge has no model vertices because it's periodic.
    // Are we being asked to split only at interior points?
    // Or is one of the split locations the start/end point
    // of the edge's sequence. If the former, then we reorder
    // edge points so a split occurs at the beginning/end:
    if (
      **locationsInEdgeOrder.begin() != *edgeToSplit->pointsBegin() &&
      **locationsInEdgeOrder.rbegin() != *edgeToSplit->pointsRBegin())
      {
      // Note that this is kinda futzy becase periodic edges repeat one point
      // at their beginning and end... we have to remove the duplicate before
      // splicing and then add a duplicate of the new start point to the end
      // of the list.
      smtkDebugMacro(this->session()->log(), "Edge is periodic, split is interior!");

#if defined(GCC_STDLIBCXX_SUPPORT_BROKEN)
      // GCC 4.9.2 does not support iterator conversions for
      // std::list<>::iterator and its ::splice method only takes mutable
      // iterators. With C++11, they are supposed to be const_iterators, but
      // without the conversion, things. Compiler detection is done by CMake
      // because so many non-GNU compilers define __GNUC__.
      PointSeq::iterator it = edgeToSplit->pointsEnd();
      --it;
      edgeToSplit->m_points.erase(it);
      PointSeq::const_iterator cBegin = edgeToSplit->pointsBegin();
      size_t dist = std::distance(cBegin, *locationsInEdgeOrder.begin());
      PointSeq::iterator loc2 = edgeToSplit->pointsBegin();
      std::advance(loc2, dist);
      PointSeq::iterator it2 = edgeToSplit->pointsEnd();
      edgeToSplit->m_points.splice(it2, edgeToSplit->m_points, loc2, edgeToSplit->pointsEnd());
#else
      PointSeq::const_iterator it = edgeToSplit->pointsEnd();
      --it;
      edgeToSplit->m_points.erase(it);
      it = edgeToSplit->pointsBegin();
      edgeToSplit->m_points.splice(it, edgeToSplit->m_points, *locationsInEdgeOrder.begin(), edgeToSplit->pointsEnd());
#endif
      edgeToSplit->m_points.insert(edgeToSplit->pointsEnd(), **locationsInEdgeOrder.begin());

      // Now the edge's points have been reordered so that the first (and last) point
      // will be promoted to a model vertex. Because PointSeq is a list, none of the
      // iterators in locationsInEdgeOrder are invalid.

      allVertices.reserve(locationsInEdgeOrder.size() + 1);
      finalModelVert = smtk::model::Vertex(mgr, (*splitPointsInEdgeOrder.rbegin())->id());
      }
    }
  else if (!noModelVertices) // i.e., we have model vertices at our endpoints.
    {
    // Is the first/last point a split location? If so (and because we already
    // have model vertices), then we should discard those splits.

    if (**locationsInEdgeOrder.begin() == (*splitPointsInEdgeOrder.begin())->point())
      { // First point is already a model vert... discard the first split
      locationsInEdgeOrder.erase(locationsInEdgeOrder.begin());
      splitPointsInEdgeOrder.erase(splitPointsInEdgeOrder.begin());
      }

    if (!locationsInEdgeOrder.empty() && **locationsInEdgeOrder.rbegin() == (*splitPointsInEdgeOrder.rbegin())->point())
      { // Last point is already a model vert... discard the last split
      locationsInEdgeOrder.erase((++locationsInEdgeOrder.rbegin()).base());
      splitPointsInEdgeOrder.erase((++splitPointsInEdgeOrder.rbegin()).base());
      }

    allVertices.reserve(locationsInEdgeOrder.size() + 2);
    // Add model vertex at head:
    allVertices.push_back(modelEdge.vertices()[0]);
    // Remember the vertex at the tail for later:
    finalModelVert = modelEdge.vertices()[1];
    }
  else
    { // We don't have model vertices, but we aren't periodic? This is an error.
    smtkErrorMacro(this->session()->log(),
      "Asked to split a non-periodic edge with no model vertices. Not possible.");
    return false;
    }
  SegmentSplitsT segs;
  std::vector<SegmentSplitsT::iterator> segSplits;
  std::vector<vertex::Ptr>::iterator mvertit = splitPointsInEdgeOrder.begin();
  PointSeq::const_iterator prev = edgeToSplit->pointsBegin();
  segs.reserve(npts - 1); // Preallocation to prevent vector from reallocating and invalidating segSplit iterator.
  PointSeq::const_iterator it = prev;
  std::vector<PointSeq::const_iterator>::const_iterator lit = locationsInEdgeOrder.begin();
  for (++it; it != edgeToSplit->pointsEnd(); ++it, ++n)
    {
    segs.push_back(std::pair<size_t,Segment>(n, Segment(*prev, *it)));
    if (prev == *lit)
      {
      // We've happened upon a split point.
      allVertices.push_back(smtk::model::Vertex(mgr, (*mvertit)->id())); // Add the model vertex here to the list.
      ++mvertit;
      segSplits.push_back(segs.begin() + n); // Remember where to start the next edge.
      ++lit; // Start looking for the next split point.
      if (lit == locationsInEdgeOrder.end())
        { // Prevent dereferencing the end iterator:
        lit = locationsInEdgeOrder.begin();
        }
      }
    prev = it;
    }
  allVertices.push_back(finalModelVert); // Now we have an array of N+1 model vertices bounding N edges.
  //DumpSegSplits("Pre-split: ", segs.begin(), segs.end());

  // Remove edgeToSplit from its endpoint vertices so that creation
  // of new edges can succeed (otherwise it will fail when trying
  // to insert a coincident edge at the existing edge endpoints).
  std::pair<Id,Id> adjacentFaces = this->removeModelEdgeFromEndpoints(mgr, edgeToSplit);
  bool isFreeCell = (!adjacentFaces.first && !adjacentFaces.second);

  // Now we can create the new model edges.
  SegmentSplitsT::iterator last = segs.begin();
  smtk::model::Vertices::iterator avit = allVertices.begin();
  smtk::model::Edge eout;
  smtk::model::Model model(mgr, this->id());
  smtk::model::EntityRefArray cre;
  for (std::vector<SegmentSplitsT::iterator>::iterator sgit = segSplits.begin(); sgit != segSplits.end(); ++sgit)
    {
    eout = this->createModelEdgeFromSegments(mgr, last, *sgit);
    // Tie vertices to parent edge:
    eout.findOrAddRawRelation(*avit);
    ++avit;
    eout.findOrAddRawRelation(*avit);
    // Tie edge to model (if edge is not "owned" by a face).
    if (isFreeCell)
      {
      model.addCell(eout);
      }
    cre.push_back(eout);
    created.insert(eout);
    last = *sgit;
    }
  if (last != segs.end())
    {
    eout = this->createModelEdgeFromSegments(mgr, last, segs.end());
    if (isFreeCell)
      {
      model.addCell(eout);
      }
    cre.push_back(eout);
    created.insert(eout);
    }

  // Update loops of face(s) attached to original edge.
  // Each use-record of the old (original) edge should be replaced with one
  // use-record for each of the new edges. Traverse the list of modelEdge uses:
  smtk::model::EdgeUses oldEdgeUses = modelEdge.uses<smtk::model::EdgeUses>();
  for (smtk::model::EdgeUses::iterator oeus = oldEdgeUses.begin(); oeus != oldEdgeUses.end(); ++oeus)
    {
    // Find the loop the use participates in:
    smtk::model::Loop modelLoop =
      oeus->boundingShellEntity().as<smtk::model::Loop>();
    // Create uses of replacement edge(s) in order of modelEdgeUse
    // Since we use modelEdge's point sequence, we know that our new uses
    // must have the same sense and orientation. If the orientation of the
    // old-edge-use is NEGATIVE, then we should reverse the order of
    // the replacement uses.
    smtk::model::EdgeUses replacements;
    int origSense = oeus->sense();
    smtk::model::Orientation origOrientation = oeus->orientation();
    if (origOrientation == smtk::model::POSITIVE)
      {
      smtk::model::EntityRefArray::iterator creit;
      for (creit = cre.begin(); creit != cre.end(); ++creit)
        {
        replacements.push_back(
          mgr->addEdgeUse(*creit, origSense, origOrientation));
        }
      }
    else
      {
      smtk::model::EntityRefArray::reverse_iterator creit;
      for (creit = cre.rbegin(); creit != cre.rend(); ++creit)
        {
        replacements.push_back(
          mgr->addEdgeUse(*creit, origSense, origOrientation));
        }
      }
    modelLoop.replaceEdgeUseWithUses(*oeus, replacements);
    }
  //smtk::io::ExportJSON::fromModelManagerToFile(mgr, "/tmp/inanity.json");

  // Handle property assignments to output edges:
  this->session()->splitProperties(modelEdge, created);

  model.removeCell(modelEdge);
  //DumpSegSplits("Split A: ", segs.begin(), segSplit);
  //DumpSegSplits("Split B: ", segSplit, segs.end());
  mgr->erase(edgeToSplit->id());
  //std::cout << "Split into " << eA.name() << " " << eB.name() << "\n";

  // TODO: Fix face adjacency information (face relations and at all 3 vertices)
  //       Create new edge uses if old ones were present.
  //       Fix face loops by replacing old edge with new edges.
  //       Create/fix vertex use at model vertex? No, this should be done wherever the vertex is created.
  //       Add first+last model vertices to "modified" if the edge had any vertices originally.
  return true;
}

/**\brief Create a model edge from 2 model vertices.
  *
  * The model vertices should be different.
  *
  * If these preconditions do not hold, either an invalid (empty) edge will be
  * returned or the model will become inconsistent.
  */
model::Edge pmodel::createModelEdgeFromVertices(model::ManagerPtr mgr,
						internal::VertexPtr v0,
						internal::VertexPtr v1)
{
  if (!mgr || !v0 || !v1)
    {
    smtkErrorMacro(this->m_session->log(),
		   "Detected either invalid Model Manager or at "
		   "least one of the vertices was NULL");
    return smtk::model::Edge();
    }
      
  if (v0 == v1)
    {
    smtkErrorMacro(this->m_session->log(),
		   "Vertices must be unique");
    return smtk::model::Edge();
    }

  internal::vertex::incident_edges::iterator whereBegin;
  internal::vertex::incident_edges::iterator whereEnd;
  // Ensure edge can be inserted without splitting a face.
  if (!v0->canInsertEdge(v1->point(), &whereBegin))
    {
    smtkErrorMacro(this->m_session->log(),
		   "Edge would overlap face in neighborhood of first vertex (" << smtk::model::Vertex(mgr, v0->id()).name() << ")A.");
    return smtk::model::Edge();
    }
  
 // Ensure edge can be inserted without splitting a face.
  if (!v1->canInsertEdge(v0->point(), &whereEnd))
    {
    smtkErrorMacro(this->m_session->log(),
		   "Edge would overlap face in neighborhood of last vertex (" << smtk::model::Vertex(mgr, v1->id()).name() << ")B.");
    return smtk::model::Edge();
    }


  // We can safely create the edge now
  smtk::model::Edge created = mgr->addEdge();
  internal::edge::Ptr storage = internal::edge::create();
  storage->setParent(this);
  storage->setId(created.entity());
  this->m_session->addStorage(created.entity(), storage);
  storage->m_points.clear();
  storage->m_points.push_back(v0->point());
  storage->m_points.push_back(v1->point());

  smtk::model::Model parentModel(mgr, this->id());
  // Insert edge at proper place in model vertex edge-lists
  v0->insertEdgeAt(whereBegin, created.entity(), /* edge is outwards: */ true);
  smtk::model::Vertex vert0(mgr, v0->id());
  if (parentModel.isEmbedded(vert0))
    {
    parentModel.removeCell(vert0);
    }
  created.findOrAddRawRelation(vert0);
  vert0.findOrAddRawRelation(created);
 
  v1->insertEdgeAt(whereEnd, created.entity(), /* edge is outwards: */ false);
  smtk::model::Vertex vert1(mgr, v1->id());
  if (parentModel.isEmbedded(vert1))
    {
    parentModel.removeCell(vert1);
    }
  created.findOrAddRawRelation(vert1);
  vert1.findOrAddRawRelation(created);
 
  // Add tesselation to created edge using storage to lift point coordinates:
  this->addEdgeTessellation(created, storage);

  parentModel.embedEntity(created);
  created.assignDefaultName(); // Do not move above parentModel.embedEntity() or name will suck.
  return created;
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

/// Remove a reverse lookup (from coordinates to vertex ID) from the model's search structure.
bool pmodel::removeVertexLookup(const Point& location, const Id& vid)
{
  PointToVertexId::const_iterator pit = this->m_vertices.find(location);
  if (pit == this->m_vertices.end() || pit->second != vid)
    {
    return false;
    }
  this->m_vertices.erase(pit);
  return true;
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

void pmodel::pointsInLoopOrder(std::vector<Point>& pts, const smtk::model::Loop& loop)
{
  EdgeUses eu = loop.edgeUses();
  for (EdgeUses::iterator it = eu.begin(); it != eu.end(); ++it)
    {
    //std::cout << "      " << it->name() << " " << (it->orientation() == POSITIVE ? "+" : "-") << "  " << it->edge().name() << " " << it->edge().entity() << "\n";
    edge::Ptr erec = this->session()->findStorage<edge>(it->edge().entity());
    if (erec)
      {
      if (it->orientation() == POSITIVE)
        {
        pts.insert(pts.end(), erec->pointsBegin(), erec->pointsEnd());
        /*
        for (PointSeq::const_iterator pit = erec->pointsBegin(); pit != erec->pointsEnd(); ++pit)
          {
          std::cout << "        " << pit->x() << " " << pit->y() << "\n";
          }
          */
        }
      else
        {
        pts.insert(pts.end(), erec->pointsRBegin(), erec->pointsREnd());
        /*
        for (PointSeq::const_reverse_iterator pit = erec->pointsRBegin(); pit != erec->pointsREnd(); ++pit)
          {
          std::cout << "        " << pit->x() << " " << pit->y() << "\n";
          }
          */
        }
      }
    }
}

template<typename T>
void preparePointsForBoost(T& ppts, internal::Coord& denx, internal::Coord& deny, bool useExistingDenominators)
{
  // If we aren't given denx + deny, loop through ppts to find them:
  if (!useExistingDenominators)
    {
    if (ppts.empty())
      {
      denx = 1;
      deny = 1;
      }
    else
      {
      internal::Coord xblo, xbhi;
      internal::Coord yblo, ybhi;
      auto pit = ppts.begin();
      xbhi = pit->x(); xblo = xbhi;
      ybhi = pit->y(); yblo = ybhi;
      for (++pit; pit != ppts.end(); ++pit)
        {
        if (xbhi < pit->x()) { xbhi = pit->x(); } else if (xblo > pit->x()) { xblo = pit->x(); }
        if (ybhi < pit->y()) { ybhi = pit->y(); } else if (yblo > pit->y()) { yblo = pit->y(); }
        }
      internal::Point bdsLo = internal::Point(xblo, yblo);
      internal::Point bdsHi = internal::Point(xbhi, ybhi);
      internal::Coord dx = xbhi;
      if (xblo < 0 && -xblo > dx)
        {
        dx = -xblo;
        }
      internal::Coord dy = ybhi;
      if (yblo < 0 && -yblo > dy)
        {
        dy = -yblo;
        }
      double lx = dx > 0 ? (std::log(dx) / std::log(2.0)) : 1.0;
      double ly = dy > 0 ? (std::log(dy) / std::log(2.0)) : 1.0;
      denx = lx > 31 ? (1 << static_cast<int>(std::ceil(lx - 31))) : 1;
      deny = ly > 31 ? (1 << static_cast<int>(std::ceil(ly - 31))) : 1;
      }
    }
  bool denom = denx > 1 || deny > 1;
  // If we need to truncate points, loop through them and do it:
  if (denom)
    {
    for (auto fpit = ppts.begin(); fpit != ppts.end(); ++fpit)
      {
      fpit->x(fpit->x() / denx);
      fpit->y(fpit->y() / deny);
      }
    }
}

void pmodel::addFaceTessellation(smtk::model::Face& faceRec)
{
  smtk::model::Model model = faceRec.owningModel();
  poly::polygon_set_data<internal::Coord> polys;
  poly::polygon_data<internal::Coord> pface;
  smtk::model::Loops outerLoops = faceRec.positiveUse().loops();
  smtk::model::Tessellation blank;
  smtk::model::UUIDsToTessellations::iterator smtkTess =
    faceRec.manager()->setTessellation(faceRec.entity(), blank);
  //std::cout << "Tessellate " << faceRec.name() << "\n";
  for (smtk::model::Loops::iterator lit = outerLoops.begin(); lit != outerLoops.end(); ++lit)
    {
    smtk::model::Loops innerLoops = lit->containedLoops();
    int npp = 1 + innerLoops.size();
    std::vector<std::vector<internal::Point> > pp2(npp);
    int ll = 0;
    //std::cout << "  Loop " << lit->name() << "\n";
    this->pointsInLoopOrder(pp2[ll], *lit);
    internal::Coord denx, deny;
    preparePointsForBoost(pp2[ll], denx, deny, false);
    bool denom = denx > 1 || deny > 1;
    pface.set(pp2[ll].rbegin(), pp2[ll].rend()); // boost likes its loops backwards
    poly::assign(polys, pface);
    ++ll;
    for (smtk::model::Loops::iterator ilit = innerLoops.begin(); ilit != innerLoops.end(); ++ilit, ++ll)
      {
      //std::cout << "    Inner Loop " << ilit->name() << "\n";
      this->pointsInLoopOrder(pp2[ll], *ilit);
      preparePointsForBoost(pp2[ll], denx, deny, true);
      poly::polygon_data<internal::Coord> loop;
      loop.set(pp2[ll].rbegin(), pp2[ll].rend());
      polys -= loop;
      }

    // Add the component to the face tessellation:
    std::vector<poly::polygon_data<internal::Coord> > tess;
    polys.get_trapezoids(tess);
    std::vector<poly::polygon_data<internal::Coord> >::const_iterator pit;
    double smtkPt[3];
    internal::Point ipt;
    for (pit = tess.begin(); pit != tess.end(); ++pit)
      {
      poly::polygon_data<internal::Coord>::iterator_type pcit;
      pcit = poly::begin_points(*pit);
      std::vector<int> triConn;
      triConn.resize(4);
      triConn[0] = smtk::model::TESS_TRIANGLE;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      this->liftPoint(ipt, &smtkPt[0]);
      triConn[1] = smtkTess->second.addCoords(&smtkPt[0]);
      //std::cout << "  " << triConn[1] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      ++pcit;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      this->liftPoint(ipt, &smtkPt[0]);
      triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
      ++pcit;
      //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      for (; pcit != poly::end_points(*pit); ++pcit)
        {
        triConn[2] = triConn[3];
        ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
        this->liftPoint(ipt, &smtkPt[0]);
        triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
        //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
        smtkTess->second.insertNextCell(triConn);
        }
      //std::cout << "\n";
      //faceRec.setColor(1., 1., 1., 1.);
      }
    }
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

void pmodel::addVertTessellation(smtk::model::Vertex& vertRec, internal::vertex::Ptr vertData)
{
  if (!vertRec.isValid() || !vertData)
    return;

  double snappedPt[3];
  this->liftPoint(vertData->point(), snappedPt);
  smtk::model::Tessellation tess;
  tess.addPoint(snappedPt);
  vertRec.setTessellation(&tess);
}

Id pmodel::pointId(const Point& p) const
{
  PointToVertexId::const_iterator it = this->m_vertices.find(p);
  if (it == this->m_vertices.end())
    return Id();
  return it->second;
}

/**\brief Move the model vertex \a vertRec from its current location to \a vertPosn.
  *
  * Note that this should not generally be allowed without a lot of checks to
  * verify that the tweak results in a valid model.
  * No checks are performed here.
  */
bool pmodel::tweakVertex(smtk::model::Vertex vertRec, const Point& vertPosn, smtk::model::EntityRefs& modifiedEdgesAndFaces)
{
  bool didChange = false;
  vertex::Ptr vv = this->session()->findStorage<vertex>(vertRec.entity());
  if (!vv)
    {
    return didChange;
    }
  didChange = (vv->point() != vertPosn);
  /*
  smtkDebugMacro(this->session()->log(),
    "  Vert from " << vv->point().x() << " " << vv->point().y() <<
    " to " << vertPosn.x() << " " << vertPosn.y() << "\n" <<
    "  Did tweak? " << (didChange ? "Y" : "N") <<
    );
    */
  if (!didChange)
    {
    return didChange;
    }

  // Erase old reverse lookup, update vertex, and add new reverse lookup:
  this->m_vertices.erase(vv->point());
  vv->m_coords = vertPosn;
  this->m_vertices[vertPosn] = vertRec.entity();

  vertex::incident_edges::iterator eit;
  for (eit = vv->edgesBegin(); eit != vv->edgesEnd(); ++eit)
    {
    smtk::model::Edge edgeRec(vertRec.manager(), eit->edgeId());
    PointSeq::iterator pit;
    edge::Ptr ee = this->session()->findStorage<edge>(eit->edgeId());
    if (eit->isEdgeOutgoing())
      { // Update the first point along the edge
      pit = ee->pointsBegin();
      }
    else
      { // Update the last point along the edge
      pit = (++ee->pointsRBegin()).base();
      }
    *pit = vertPosn;
    this->addEdgeTessellation(edgeRec, ee);
    modifiedEdgesAndFaces.insert(edgeRec);

    // If any faces are attached to the vertex, they must be retessellated.
    smtk::model::Faces facesOnEdge = edgeRec.faces();
    for (smtk::model::Faces::iterator fit = facesOnEdge.begin(); fit != facesOnEdge.end(); ++fit)
      {
      // If we have a face attached, re-tessellate it and add to modifiedEdgesAndFaces
      if (modifiedEdgesAndFaces.find(*fit) == modifiedEdgesAndFaces.end())
        {
        didChange = true;
        this->addFaceTessellation(*fit);
        modifiedEdgesAndFaces.insert(*fit);
        }
      }
    }

  // Update the SMTK tessellation in world coordinates:
  this->addVertTessellation(vertRec, vv);

  return didChange;
}

void pmodel::addVertexIndex(vertex::Ptr vert)
{
  this->m_vertices[vert->point()] = vert->id();
}

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
