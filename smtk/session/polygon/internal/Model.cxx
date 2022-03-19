//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Edge.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

#include "smtk/io/Logger.h"
//#include "smtk/io/SaveJSON.h"

#include "smtk/session/polygon/Session.h"

#include "smtk/model/Resource.txx"
#include "smtk/session/polygon/Session.txx"
#include "smtk/session/polygon/internal/Model.txx"

using namespace smtk::model;

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{

typedef std::vector<std::pair<size_t, Segment>> SegmentSplitsT;

pmodel::pmodel()
  : m_session(nullptr)
{
  for (int i = 0; i < 3; ++i)
  {
    m_origin[i] = 0.; // Base point of plane for model
    m_xAxis[i] =
      0.; // Vector whose length should be equal to one "unit" (e.g., m_scale integers long)
    m_yAxis[i] = 0.; // In-plane vector orthogonal to m_xAxis with the same length.
    m_zAxis[i] = 0.; // Normal vector orthogonal to m_xAxis and m_yAxis with the same length.
    m_iAxis[i] =
      0.; // Vector whose length should be equal to one "unit" (e.g., 1 integer delta long)
    m_jAxis[i] = 0.; // In-plane vector orthogonal to m_iAxis with the same length.
  }
  m_xAxis[0] = m_yAxis[1] = m_zAxis[2] = 1.;
  m_iAxis[0] = m_jAxis[1] = 1.;
}

pmodel::~pmodel()
{
  // Tis better to have dereferenced and crashed than never to have crashed at all:
  m_session = nullptr;
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
    smtkErrorMacro(log, "Feature size must be positive (not " << featureSize << ").");
    return false;
  }
  m_featureSize = featureSize;

  if (origin.size() != 3 || x_axis.size() != 3 || y_axis.size() != 3)
  {
    smtkErrorMacro(
      log,
      "Vector of length 3 expected for"
        << " origin (" << origin.size() << "),"
        << " x axis (" << x_axis.size() << "), and"
        << " y axis (" << y_axis.size() << ").");
    return false;
  }
  double xl2 = 0., yl2 = 0., zl2 = 0.;
  for (int i = 0; i < 3; ++i)
  {
    m_origin[i] = origin[i];
    m_xAxis[i] = x_axis[i];
    m_yAxis[i] = y_axis[i];
    xl2 += x_axis[i] * x_axis[i];
    yl2 += y_axis[i] * y_axis[i];
    m_zAxis[i] =
      x_axis[(i + 1) % 3] * y_axis[(i + 2) % 3] - x_axis[(i + 2) % 3] * y_axis[(i + 1) % 3];
    zl2 += m_zAxis[i] * m_zAxis[i];
  }
  if (xl2 < 1e-16 || yl2 < 1e-16 || zl2 < 1e-16)
  {
    smtkErrorMacro(
      log,
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
  m_scale = static_cast<long long>(231.0 / featureSize);
  // Make the world (x,y) axes unit length and the (i,j) axes one quantum in length:
  for (int i = 0; i < 3; ++i)
  {
    m_xAxis[i] /= xl2;
    m_yAxis[i] /= yl2;
    m_zAxis[i] /= zl2;
    m_iAxis[i] = m_xAxis[i] / m_scale;
    m_jAxis[i] = m_yAxis[i] / m_scale;
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
    smtkErrorMacro(log, "Feature size must be positive (not " << featureSize << ").");
    return false;
  }
  m_featureSize = featureSize;

  if (origin.size() != 3 || x_axis.size() != 3 || z_axis.size() != 3)
  {
    smtkErrorMacro(
      log,
      "Vector of length 3 expected for"
        << " origin (" << origin.size() << "),"
        << " x axis (" << x_axis.size() << "), and"
        << " z axis (" << z_axis.size() << ").");
    return false;
  }
  double xl2 = 0., yl2 = 0., zl2 = 0.;
  for (int i = 0; i < 3; ++i)
  {
    m_origin[i] = origin[i];
    m_xAxis[i] = x_axis[i];
    m_zAxis[i] = z_axis[i];
    xl2 += x_axis[i] * x_axis[i];
    zl2 += z_axis[i] * z_axis[i];
    m_yAxis[i] =
      z_axis[(i + 1) % 3] * x_axis[(i + 2) % 3] - z_axis[(i + 2) % 3] * x_axis[(i + 1) % 3];
    yl2 += m_yAxis[i] * m_yAxis[i];
  }
  if (xl2 < 1e-16 || yl2 < 1e-16 || zl2 < 1e-16)
  {
    smtkErrorMacro(
      log,
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
  m_scale = static_cast<long long>(231.0 / featureSize);
  // Make the world (x,y) axes unit length and the (i,j) axes one quantum in length:
  for (int i = 0; i < 3; ++i)
  {
    m_xAxis[i] /= xl2;
    m_yAxis[i] /= yl2;
    m_zAxis[i] /= zl2;
    m_iAxis[i] = m_xAxis[i] / m_scale;
    m_jAxis[i] = m_yAxis[i] / m_scale;
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
    smtkErrorMacro(
      log,
      "Vector of length 3 expected for"
        << " origin (" << origin.size() << "),"
        << " x axis (" << x_axis.size() << "), and"
        << " y axis (" << y_axis.size() << ").");
    return false;
  }
  double xl2 = 0., yl2 = 0., zl2 = 0.;
  for (int i = 0; i < 3; ++i)
  {
    m_origin[i] = origin[i];
    m_xAxis[i] = x_axis[i];
    m_yAxis[i] = y_axis[i];
    xl2 += x_axis[i] * x_axis[i];
    yl2 += y_axis[i] * y_axis[i];
    m_zAxis[i] =
      x_axis[(i + 1) % 3] * y_axis[(i + 2) % 3] - x_axis[(i + 2) % 3] * y_axis[(i + 1) % 3];
    zl2 += m_zAxis[i] * m_zAxis[i];
  }
  if (xl2 < 1e-16 || yl2 < 1e-16 || zl2 < 1e-16)
  {
    smtkErrorMacro(
      log,
      "Vectors of non-zero L2 norm required for "
        << " x (" << xl2 << "),"
        << " y (" << yl2 << "), and"
        << " z (" << zl2 << ") axes.");
    return false;
  }
  xl2 = sqrt(xl2);
  yl2 = sqrt(yl2);
  zl2 = sqrt(zl2);
  m_scale = modelScale;
  m_featureSize = 1.0;
  for (int i = 0; i < 3; ++i)
  {
    m_xAxis[i] /= xl2;
    m_yAxis[i] /= yl2;
    m_zAxis[i] /= zl2;
    m_iAxis[i] = m_xAxis[i] / m_scale;
    m_jAxis[i] = m_yAxis[i] / m_scale;
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
    origin.size() != 3 || x_axis.size() != 3 || y_axis.size() != 3 || z_axis.size() != 3 ||
    i_axis.size() != 3 || j_axis.size() != 3)
  {
    std::cerr << "Vector of length 3 expected for"
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
    m_origin[i] = origin[i];
    m_xAxis[i] = x_axis[i];
    m_yAxis[i] = y_axis[i];
    m_zAxis[i] = z_axis[i];
    m_iAxis[i] = i_axis[i];
    m_jAxis[i] = j_axis[i];
  }
  m_scale = modelScale;
  m_featureSize = featureSize;
  return true;
}

smtk::model::Vertices pmodel::findOrAddModelVertices(
  smtk::model::ResourcePtr resource,
  const std::vector<double>& points,
  int numCoordsPerPt)
{
  smtk::model::Vertices vertices;
  std::vector<double>::const_iterator it = points.begin();
  long long i = 0;
  for (i = 0; it != points.end(); it += numCoordsPerPt, i += numCoordsPerPt)
  {
    Point projected = this->projectPoint(it, it + numCoordsPerPt);
    vertices.push_back(this->findOrAddModelVertex(resource, projected));
  }
  return vertices;
}

/**\brief Add a vertex to the model resource.
  *
  * This creates a vertex record in the model resource and adds its tessellation.
  * It also adds the integer coordinates of the point to
  * the internal model's data (this instance).
  * This does **not** create a default name or relate the vertex record in
  * the model resource to a parent model or owning geometric entity
  * (such as an edge, face, or volume) unless \a addToModel is true,
  * in which case the vertex is added as a free cell to the SMTK model.
  */
smtk::model::Vertex
pmodel::findOrAddModelVertex(smtk::model::ResourcePtr resource, const Point& pt, bool addToModel)
{
  PointToVertexId::const_iterator pit = m_vertices.find(pt);
  if (pit != m_vertices.end())
    return smtk::model::Vertex(resource, pit->second);

  return this->addModelVertex(resource, pt, addToModel);
}

smtk::model::Vertex
pmodel::addModelVertex(smtk::model::ResourcePtr resource, const Point& pt, bool addToModel)
{
  // Add a model vertex to the resource
  smtk::model::Vertex v = resource->addVertex();
  // Add a coordinate-map lookup to local storage:
  m_vertices[pt] = v.entity();
  // Create internal storage for the neighborhood of the vertex:
  vertex::Ptr vi = vertex::create();
  vi->setParent(this);
  vi->setId(v.entity());
  vi->m_coords = pt;
  m_session->addStorage(v.entity(), vi);
  // Figure out the floating-point approximation for our discretized coordinate
  // and add it to the tessellation for the new model vertex:
  this->addVertTessellation(v, vi);

  // Add vertex to model as a free cell (which it is until it bounds something).
  smtk::model::Model self(resource, this->id());
  if (addToModel)
  {
    self.embedEntity(v);
    v.assignDefaultName();
  }

  return v;
}

/// Add segments to \a segs for the given \a edge with either forward (\a reverse false) or backward (\a reverse true) orientation.
template<typename T>
void addSegmentsForEdge(T& segs, edge::Ptr edge, bool reverse)
{
  size_t n = segs.size();
  if (!reverse)
  {
    PointSeq::const_iterator prev = edge->pointsBegin();
    PointSeq::const_iterator it = prev;
    for (++it; it != edge->pointsEnd(); ++it, ++n)
    {
      segs.push_back(std::pair<size_t, Segment>(n, Segment(*prev, *it)));
    }
  }
  else
  {
    PointSeq::const_reverse_iterator prev = edge->pointsRBegin();
    PointSeq::const_reverse_iterator it = prev;
    for (++it; it != edge->pointsREnd(); ++it, ++n)
    {
      segs.push_back(std::pair<size_t, Segment>(n, Segment(*prev, *it)));
    }
  }
}

/**\brief Replace all edge-uses of \a modelEdge with nothing (if \a replacementEdge
  *       is invalid) or new edge-uses of \a replacementEdge that mirror the sense
  *       and orientation of \a modelEdge.
  *
  * This assumes \a modelEdge and \replacementEdge have the same underlying direction.
  *
  * TODO: Move to smtk/model? Is it generic enough?
  */
void edgeUseAndLoopRewrite(smtk::model::Edge& modelEdge, const smtk::model::Edge& replacementEdge)
{
  smtk::model::Resource::Ptr resource = modelEdge.resource();
  // Each use-record of the old (original) edge should be replaced with one or zero
  // use-records for each of the new edges. Traverse the list of modelEdge uses:
  smtk::model::EdgeUses oldEdgeUses = modelEdge.uses<smtk::model::EdgeUses>();
  for (smtk::model::EdgeUses::iterator oeus = oldEdgeUses.begin(); oeus != oldEdgeUses.end();
       ++oeus)
  {
    // Find the loop the use participates in:
    smtk::model::Loop modelLoop = oeus->boundingShellEntity().as<smtk::model::Loop>();
    // Create uses of replacement edge(s) in order of modelEdgeUse
    // Since we use modelEdge's point sequence, we know that our new uses
    // must have the same sense and orientation. If the orientation of the
    // old-edge-use is NEGATIVE, then we should reverse the order of
    // the replacement uses.
    smtk::model::EdgeUses replacements;
    int origSense = oeus->sense();
    smtk::model::Orientation origOrientation = oeus->orientation();
    if (replacementEdge.isValid())
    {
      replacements.push_back(resource->addEdgeUse(replacementEdge, origSense, origOrientation));
    }
    modelLoop.replaceEdgeUseWithUses(*oeus, replacements);
  }
}

/**\brief Demote a model vertex.
  *
  * Returns true when the vertex is deleted and false otherwise (because
  * exactly 1 or more than 2 edge-incidences were present).
  */
bool pmodel::demoteModelVertex(
  smtk::model::ResourcePtr resource,
  internal::VertexPtr vert,
  smtk::model::EntityRefs& created,
  smtk::model::EntityRefs& modified,
  smtk::model::EntityRefs& expunged,
  int debugLevel)
{
  if (!vert)
  {
    return false;
  }
  vertex::incident_edges::size_type nie = vert->numberOfEdgeIncidences();
  if (nie != 0 && nie != 2)
  {
    smtkErrorMacro(
      this->session()->log(),
      "Expected 0 or 2 edge incidences to " << smtk::model::Vertex(resource, vert->id()).name()
                                            << " but got " << nie);
    return false;
  }

  // Nothing beyond here could/should cause a failure; so it is safe to modify storage.

  std::pair<Id, Id> adjacentFaces1;
  std::pair<Id, Id> adjacentFaces2;
  bool isFreeCell = true;
  SegmentSplitsT segs;
  if (nie == 2)
  {
    // Look up the incident edges:
    vertex::incident_edges::iterator ierec1 = vert->edgesBegin();
    vertex::incident_edges::iterator ierec2 = ierec1;
    ++ierec2;
    smtk::model::Edge e1(resource, ierec1->edgeId());
    smtk::model::Edge e2(resource, ierec2->edgeId());
    smtk::model::Model model(e1.owningModel());
    edge::Ptr ie1 = this->session()->findStorage<edge>(ierec1->edgeId());
    edge::Ptr ie2 = this->session()->findStorage<edge>(ierec2->edgeId());

    // Find the "other" vertices attached to the incident edges:
    smtk::model::Vertices everts;
    everts = e1.vertices();
    smtk::model::Vertex ve1other =
      everts.front().entity() == vert->id() ? everts.back() : everts.front();
    everts = e2.vertices();
    smtk::model::Vertex ve2other =
      everts.front().entity() == vert->id() ? everts.back() : everts.front();

    // Perform surgery as needed:
    if (e1 == e2)
    { // The same model edge is incident twice.
      adjacentFaces1 = this->removeModelEdgeFromEndpoints(resource, ie1);
      isFreeCell = false; // well, it may be free, but we aren't changing it
    }
    else
    {
      // Capture the configuration of edges relative to the demoted vertex
      // before we remove them from the vertex's internal records:
      //      false: otherVert |--- e[12] ---> demotedVert or
      //       true: otherVert <--- e[12] ---| demotedVert?
      bool e1OutgoingFromDemotedVert = ierec1->isEdgeOutgoing();
      bool e2OutgoingFromDemotedVert = ierec2->isEdgeOutgoing();

      // Remove edges being merged from their endpoint vertices so that creation
      // of the new edge can succeed (otherwise it will fail when trying
      // to insert a coincident edge at the remaining edge endpoints).
      adjacentFaces1 = this->removeModelEdgeFromEndpoints(resource, ie1);
      adjacentFaces2 = this->removeModelEdgeFromEndpoints(resource, ie2);
      if (debugLevel > -5)
      {
        smtkDebugMacro(
          m_session->log(),
          "Demote adjacencies\n"
            << "  " << adjacentFaces1.first.toString() << " / " << adjacentFaces1.second.toString()
            << "  " << (e1OutgoingFromDemotedVert ? "o" : "i") << "\n"
            << "  " << adjacentFaces2.first.toString() << " / " << adjacentFaces2.second.toString()
            << "  " << (e2OutgoingFromDemotedVert ? "o" : "i"));
      }
      isFreeCell = (!adjacentFaces1.first && !adjacentFaces1.second);

      // Accumulate points from both edges as segments.
      size_t npts =
        ie1->pointsSize() + ie2->pointsSize() - 1; // -1 => don't duplicate model vertex location.
      segs.reserve(
        npts -
        1); // Preallocation to prevent vector from reallocating and invalidating segSplit iterator.

      // Create a new edge with the same orientation as edge 1.
      // Accumulate points from both edges
      if (e1OutgoingFromDemotedVert)
      {
        addSegmentsForEdge(segs, ie2, e2OutgoingFromDemotedVert);
        addSegmentsForEdge(segs, ie1, /*reverse?*/ false);
      }
      else
      {
        addSegmentsForEdge(segs, ie1, /*reverse?*/ false);
        addSegmentsForEdge(segs, ie2, !e2OutgoingFromDemotedVert);
      }
      smtk::model::VertexSet newVerts;
      // Now we can create the new model edge.
      smtk::model::Edge eout = this->createModelEdgeFromSegments(
        resource, segs.begin(), segs.end(), /*addToModel:*/ false, adjacentFaces1, false, newVerts);
      // FIXME: URHERE: Make sure ve1other and ve2other have face-adjacency recorded properly after eout is created.
      eout.findOrAddRawRelation(ve1other);
      if (ve2other != ve1other)
      { // If the new edge is a loop, only add the common vertex once:
        eout.findOrAddRawRelation(ve2other);
      }
      if (isFreeCell)
      {
        model.addCell(eout);
      }
      //created.push_back(eout);
      created.insert(eout);

      if (debugLevel > 0)
      {
        // Print eout's verts to see if demoted vertex somehow ends up there.
      }

      // Update loops of face(s) attached to edge 1 and 2.
      edgeUseAndLoopRewrite(e1, eout); // Replace uses of e1 with uses of eout
      edgeUseAndLoopRewrite(
        e2,
        smtk::model::Edge()); // Invalid edge as last arg => replace uses of e2 with an empty set

      // Handle property assignments to output edges:
      smtk::model::EntityRefs merged;
      merged.insert(e1);
      merged.insert(e2);
      this->session()->mergeEntities(merged, eout);

      model.removeCell(e1);
      model.removeCell(e2);
      //DumpSegSplits("Split A: ", segs.begin(), segSplit);
      //DumpSegSplits("Split B: ", segSplit, segs.end());
      resource->erase(ie1->id());
      resource->erase(ie2->id());
      expunged.insert(e1);
      expunged.insert(e2);
      //std::cout << "Split into " << eA.name() << " " << eB.name() << "\n";
    }
  }

  // If the number of incident edges is 0, just delete the vertex and return.
  smtk::model::VertexSet verts;
  smtk::model::Vertex mvert(resource, vert->id());
  verts.insert(mvert);
  smtk::model::EntityRefArray exptmp;
  this->session()->consistentInternalDelete(verts, modified, exptmp, debugLevel > 0);
  expunged.insert(exptmp.begin(), exptmp.end());

  return true;
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
  smtk::model::ResourcePtr resource,
  const Id& edgeId,
  const std::vector<double>& coords,
  smtk::model::EntityRefArray& created,
  int debugLevel)
{
  Point pt = this->projectPoint(coords.begin(), coords.end());
  if (this->pointId(pt))
  {
    smtkWarningMacro(this->session()->log(), "Point is already a model vertex.");
    return false; // Point is already a model vertex.
  }
  // TODO: Find point on edge closest to pt? Need to find where to insert model vertex?
  smtk::model::Vertex v = this->findOrAddModelVertex(resource, pt, /*add as free cell?*/ false);
  bool result =
    this->splitModelEdgeAtModelVertex(resource, edgeId, v.entity(), created, debugLevel);
  v.assignDefaultName(); // Assign the name after the vertex is added to the edge (and has a parent model).
  return result;
}

/**\brief Split the model edge with the given \a edgeId at the given \a pointIndex.
  *
  * If the point is already a model vertex, this method returns false.
  *
  * New edge Ids and new vertex Id are added to the \a created array.
  */
bool pmodel::splitModelEdgeAtIndex(
  smtk::model::ResourcePtr resource,
  const Id& edgeId,
  int pointIndex,
  smtk::model::EntityRefArray& created,
  int debugLevel)
{
  edge::Ptr storage = m_session->findStorage<internal::edge>(edgeId);
  if (!storage)
  {
    smtkErrorMacro(this->session()->log(), "Edge is not part of this model.");
    return false;
  }
  if (pointIndex < 0 || pointIndex >= static_cast<int>(storage->pointsSize()))
  {
    smtkErrorMacro(
      this->session()->log(),
      "Point index " << pointIndex << " is invalid (must be in [0, " << storage->pointsSize()
                     << "[.");
    return false;
  }
  auto pit = storage->pointsBegin();
  for (int pp = 0; pp < pointIndex && pit != storage->pointsEnd(); ++pp)
  {
    ++pit;
  }
  Point pt = *pit;
  if (this->pointId(pt))
  {
    smtkWarningMacro(this->session()->log(), "Point is already a model vertex.");
    return false; // Point is already a model vertex.
  }
  // TODO: Find point on edge closest to pt? Need to find where to insert model vertex?
  smtk::model::Vertex v = this->findOrAddModelVertex(resource, pt, /*add as free cell?*/ false);
  bool result =
    this->splitModelEdgeAtModelVertex(resource, edgeId, v.entity(), created, debugLevel);
  v.assignDefaultName(); // Assign the name after the vertex is added to the edge (and has a parent model).
  created.push_back(v);
  return result;
}

/** Split the model edge at one of its points that has been promoted to a model vertex.
  *
  * Since model edges are not allowed to have model vertices along their interior,
  * this method should not be exposed as a public operator but may be used internally
  * when performing other operations.
  */
bool pmodel::splitModelEdgeAtModelVertex(
  smtk::model::ResourcePtr resource,
  const Id& edgeId,
  const Id& vertexId,
  smtk::model::EntityRefArray& created,
  int debugLevel)
{
  // Look up edge
  edge::Ptr edg = this->session()->findStorage<edge>(edgeId);
  // Look up vertex
  vertex::Ptr vrt = this->session()->findStorage<vertex>(vertexId);
  if (!edg || !vrt)
    return false;
  PointSeq::iterator split;
  Coord maxDelta = static_cast<Coord>(m_featureSize * m_scale);
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
      return this->splitModelEdgeAtModelVertex(resource, edg, vrt, split, created, debugLevel);
    }
  }
  // Edge did not contain model vertex in its sequence.
  return false;
}

/**\brief An internal edge split operation.
  *
  * This variant requires the point along the model vertex to already have been promoted.
  * It takes an iterator into the original model edge's sequence of points and creates
  * new edges.
  */
bool pmodel::splitModelEdgeAtModelVertex(
  smtk::model::ResourcePtr resource,
  edge::Ptr edgeToSplit,
  vertex::Ptr splitPoint,
  PointSeq::const_iterator location,
  smtk::model::EntityRefArray& created,
  int debugLevel)
{
  std::vector<vertex::Ptr> splitPoints;
  std::vector<PointSeq::const_iterator> locations;
  splitPoints.push_back(splitPoint);
  locations.push_back(location);
  return this->splitModelEdgeAtModelVertices(
    resource, edgeToSplit, splitPoints, locations, created, debugLevel);
}

bool pmodel::splitModelEdgeAtModelVertices(
  smtk::model::ResourcePtr resource,
  edge::Ptr edgeToSplit,
  std::vector<vertex::Ptr>& splitPointsInEdgeOrder,
  std::vector<PointSeq::const_iterator>& locationsInEdgeOrder,
  smtk::model::EntityRefArray& created,
  int debugLevel)
{
  size_t npts;
  if (
    !edgeToSplit || (npts = edgeToSplit->pointsSize()) < 2 || locationsInEdgeOrder.empty() ||
    splitPointsInEdgeOrder.size() != locationsInEdgeOrder.size())
    return false;

  //DumpPointSeq("Split Edge", edgeToSplit->pointsBegin(), edgeToSplit->pointsEnd(), location);
  size_t n = 0;
  smtk::model::Edge modelEdge(resource, edgeToSplit->id());
  smtk::model::Vertices allVertices;
  smtk::model::Vertex finalModelVert;
  bool isPeriodic = (*edgeToSplit->pointsBegin() == *edgeToSplit->pointsRBegin());
  //bool noModelVertices = (m_vertices.find(*edgeToSplit->pointsBegin()) == m_vertices.end());
  smtk::model::Vertices currentVertices = modelEdge.vertices();
  bool noModelVertices = currentVertices.empty();
  if (isPeriodic && noModelVertices)
  {
    // Edge has no model vertices because it's periodic.
    // Are we being asked to split only at interior points?
    // Or is one of the split locations the start/end point
    // of the edge's sequence? If the former, then we reorder
    // edge points so a split occurs at the beginning/end:
    if (
      **locationsInEdgeOrder.begin() != *edgeToSplit->pointsBegin() &&
      **locationsInEdgeOrder.rbegin() != *edgeToSplit->pointsRBegin())
    {
      // Note that this is kinda futzy becase periodic edges repeat one point
      // at their beginning and end... we have to remove the duplicate before
      // splicing and then add a duplicate of the new start point to the end
      // of the list.
      if (debugLevel > 0)
      {
        smtkDebugMacro(this->session()->log(), "Edge is periodic, split is interior!");
      }

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
      edgeToSplit->m_points.splice(
        it, edgeToSplit->m_points, *locationsInEdgeOrder.begin(), edgeToSplit->pointsEnd());
#endif
      edgeToSplit->m_points.insert(edgeToSplit->pointsEnd(), **locationsInEdgeOrder.begin());

      // Now the edge's points have been reordered so that the first (and last) point
      // will be promoted to a model vertex. Because PointSeq is a list, none of the
      // iterators in locationsInEdgeOrder are invalid.

      allVertices.reserve(locationsInEdgeOrder.size() + 1);
    }
    finalModelVert = smtk::model::Vertex(resource, (*splitPointsInEdgeOrder.rbegin())->id());
  }
  else if (!noModelVertices) // i.e., we have model vertices at our endpoints.
  {
    // Is the first/last point a split location? If so (and because we already
    // have model vertices), then we should discard those splits.

    if (**locationsInEdgeOrder.begin() == *(edgeToSplit->pointsBegin()))
    { // First point is already a model vert... discard the first split
      locationsInEdgeOrder.erase(locationsInEdgeOrder.begin());
      splitPointsInEdgeOrder.erase(splitPointsInEdgeOrder.begin());
    }

    if (
      !locationsInEdgeOrder.empty() &&
      **locationsInEdgeOrder.rbegin() == *(edgeToSplit->pointsRBegin()))
    { // Last point is already a model vert... discard the last split
      locationsInEdgeOrder.erase((++locationsInEdgeOrder.rbegin()).base());
      splitPointsInEdgeOrder.erase((++splitPointsInEdgeOrder.rbegin()).base());
    }

    allVertices.reserve(locationsInEdgeOrder.size() + 2);
    // Add model vertex at head:
    allVertices.push_back(currentVertices.front());
    // Remember the vertex at the tail for later:
    finalModelVert = currentVertices.back();
  }
  else
  { // We don't have model vertices, but we aren't periodic? This is an error.
    smtkErrorMacro(
      this->session()->log(),
      "Asked to split a non-periodic edge with no model vertices. Not possible.");
    return false;
  }
  SegmentSplitsT segs;
  std::vector<SegmentSplitsT::iterator> segSplits;
  std::vector<vertex::Ptr>::iterator mvertit = splitPointsInEdgeOrder.begin();
  PointSeq::const_iterator prev = edgeToSplit->pointsBegin();
  segs.reserve(
    npts -
    1); // Preallocation to prevent vector from reallocating and invalidating segSplit iterator.
  PointSeq::const_iterator it = prev;
  std::vector<PointSeq::const_iterator>::const_iterator lit = locationsInEdgeOrder.begin();
  for (++it; it != edgeToSplit->pointsEnd(); ++it, ++n)
  {
    segs.push_back(std::pair<size_t, Segment>(n, Segment(*prev, *it)));
    if (prev == *lit)
    {
      // We've happened upon a split point.
      allVertices.push_back(
        smtk::model::Vertex(resource, (*mvertit)->id())); // Add the model vertex here to the list.
      ++mvertit;
      segSplits.push_back(segs.begin() + n); // Remember where to start the next edge.
      ++lit;                                 // Start looking for the next split point.
      if (lit == locationsInEdgeOrder.end())
      { // Prevent dereferencing the end iterator:
        lit = locationsInEdgeOrder.begin();
      }
    }
    prev = it;
  }
  allVertices.push_back(
    finalModelVert); // Now we have an array of N+1 model vertices bounding N edges.
  //DumpSegSplits("Pre-split: ", segs.begin(), segs.end());

  // Remove edgeToSplit from its endpoint vertices so that creation
  // of new edges can succeed (otherwise it will fail when trying
  // to insert a coincident edge at the existing edge endpoints).
  std::pair<Id, Id> adjacentFaces = this->removeModelEdgeFromEndpoints(resource, edgeToSplit);
  bool isFreeCell = (!adjacentFaces.first && !adjacentFaces.second);
  if (debugLevel > 0)
  {
    smtkDebugMacro(
      m_session->log(),
      "Split " << modelEdge.name() << "  faces " << adjacentFaces.first.toString() << " / "
               << adjacentFaces.second.toString());
  }

  // Now we can create the new model edges.
  SegmentSplitsT::iterator last = segs.begin();
  smtk::model::Edge eout;
  smtk::model::Model model(resource, this->id());
  std::size_t crepre = created.size();
  if (segSplits.empty() || segSplits.back() != segs.end())
  {
    segSplits.push_back(segs.end());
  }

  smtk::model::VertexSet newVerts;
  std::vector<SegmentSplitsT::iterator>::iterator sgit = segSplits.begin();
  do
  {
    eout = this->createModelEdgeFromSegments(
      resource, last, *sgit, /*addToModel:*/ false, adjacentFaces, *sgit != segs.end(), newVerts);
    // Tie edge to model (if edge is not "owned" by a face).
    if (eout.isValid())
    { // An invalid edge may be returned when splitting a loop that previously had no vertices.
      if (isFreeCell)
      {
        model.addCell(eout);
      }
      created.push_back(eout);
    }
    last = *sgit;
    if (last == segs.end())
    {
      break;
    }
    ++sgit;
  } while (true);

  if (debugLevel > 0)
  {
    std::ostringstream summ;
    summ << "Edge incidences at new interior vertices:\n";
    smtk::model::Vertices::iterator avit;
    for (avit = allVertices.begin(); avit != allVertices.end(); ++avit)
    {
      vertex::Ptr ivrt = this->session()->findStorage<vertex>(avit->entity());
      summ << "  " << smtk::model::Vertex(resource, ivrt->id()).name() << " ("
           << ivrt->id().toString() << ")\n";
      vertex::incident_edges::const_iterator veit;
      for (veit = ivrt->edgesBegin(); veit != ivrt->edgesEnd(); ++veit)
      {
        summ << "    " << smtk::model::Edge(resource, veit->edgeId()).name() << " ("
             << veit->edgeId().toString() << ")"
             << " cw face " << veit->clockwiseFaceId().toString() << " out? "
             << (veit->isEdgeOutgoing() ? "Y" : "N") << "\n";
      }
    }
    smtkDebugMacro(m_session->log(), summ.str());
  }

  // Update loops of face(s) attached to original edge.
  // Each use-record of the old (original) edge should be replaced with one
  // use-record for each of the new edges. Traverse the list of modelEdge uses:
  smtk::model::EdgeUses oldEdgeUses = modelEdge.uses<smtk::model::EdgeUses>();
  smtk::model::Loop modelLoop;
  for (smtk::model::EdgeUses::iterator oeus = oldEdgeUses.begin(); oeus != oldEdgeUses.end();
       ++oeus)
  {
    // Find the loop the use participates in:
    modelLoop = oeus->boundingShellEntity().as<smtk::model::Loop>();
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
      for (creit = created.begin() + crepre; creit != created.end(); ++creit)
      {
        replacements.push_back(resource->addEdgeUse(*creit, origSense, origOrientation));
      }
    }
    else
    {
      smtk::model::EntityRefArray::reverse_iterator creit;
      smtk::model::EntityRefArray::reverse_iterator crerend = created.rend() - crepre;
      for (creit = created.rbegin(); creit != crerend; ++creit)
      {
        replacements.push_back(resource->addEdgeUse(*creit, origSense, origOrientation));
      }
    }
    if (debugLevel > 0)
    {
      smtkDebugMacro(
        m_session->log(),
        "Replace " << oeus->name() << " (edge " << oeus->edge().name() << " "
                   << " sense " << origSense << " ornt "
                   << (origOrientation == smtk::model::POSITIVE ? "+" : "-") << ") with "
                   << replacements.size() << " uses.");
    }
    modelLoop.replaceEdgeUseWithUses(*oeus, replacements);
  }

  smtk::model::EntityRefs createdSet(created.begin() + crepre, created.end());
  // Handle property assignments to output edges:
  this->session()->splitEntity(modelEdge, createdSet);

  model.removeCell(modelEdge);
  //DumpSegSplits("Split A: ", segs.begin(), segSplit);
  //DumpSegSplits("Split B: ", segSplit, segs.end());
  resource->erase(edgeToSplit->id());
  //std::cout << "Split into " << eA.name() << " " << eB.name() << "\n";

  // Now, regardless of whether the new edge(s) are free cells or belong to a loop,
  // they have a parent model... assign names to those that didn't inherit one:
  smtk::model::EntityRefArray::iterator cit;
  for (cit = created.begin(); cit != created.end(); ++cit)
  {
    smtk::model::Edge credge(*cit);
    credge.assignDefaultName();
  }
  return true;
}

/**\brief Create a model edge from 2 model vertices.
  *
  * The model vertices should be different.
  *
  * If these preconditions do not hold, either an invalid (empty) edge will be
  * returned or the model will become inconsistent.
  */
model::Edge pmodel::createModelEdgeFromVertices(
  model::ResourcePtr resource,
  internal::VertexPtr v0,
  internal::VertexPtr v1)
{
  if (!resource || !v0 || !v1)
  {
    smtkErrorMacro(
      m_session->log(),
      "Detected either invalid Model Resource or at "
      "least one of the vertices was nullptr");
    return smtk::model::Edge();
  }

  if (v0 == v1)
  {
    smtkErrorMacro(m_session->log(), "Vertices must be unique");
    return smtk::model::Edge();
  }

  internal::vertex::incident_edges::iterator whereBegin;
  internal::vertex::incident_edges::iterator whereEnd;
  // Ensure edge can be inserted without splitting a face.
  if (!v0->canInsertEdge(v1->point(), &whereBegin))
  {
    smtkErrorMacro(
      m_session->log(),
      "Edge would overlap face in neighborhood of first vertex ("
        << smtk::model::Vertex(resource, v0->id()).name() << ")A.");
    return smtk::model::Edge();
  }

  // Ensure edge can be inserted without splitting a face.
  if (!v1->canInsertEdge(v0->point(), &whereEnd))
  {
    smtkErrorMacro(
      m_session->log(),
      "Edge would overlap face in neighborhood of last vertex ("
        << smtk::model::Vertex(resource, v1->id()).name() << ")B.");
    return smtk::model::Edge();
  }

  // We can safely create the edge now
  smtk::model::Edge created = resource->addEdge();
  internal::edge::Ptr storage = internal::edge::create();
  storage->setParent(this);
  storage->setId(created.entity());
  m_session->addStorage(created.entity(), storage);
  storage->m_points.clear();
  storage->m_points.push_back(v0->point());
  storage->m_points.push_back(v1->point());

  smtk::model::Model parentModel(resource, this->id());
  // Insert edge at proper place in model vertex edge-lists
  v0->insertEdgeAt(whereBegin, created.entity(), /* edge is outwards: */ true);
  smtk::model::Vertex vert0(resource, v0->id());
  if (parentModel.isEmbedded(vert0))
  {
    parentModel.removeCell(vert0);
  }
  created.findOrAddRawRelation(vert0);
  vert0.findOrAddRawRelation(created);

  v1->insertEdgeAt(whereEnd, created.entity(), /* edge is outwards: */ false);
  smtk::model::Vertex vert1(resource, v1->id());
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
std::pair<Id, Id> pmodel::removeModelEdgeFromEndpoints(
  smtk::model::ResourcePtr resource,
  EdgePtr edg)
{
  std::pair<Id, Id> result;
  if (!edg || !resource)
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
    vertex::incident_edges::iterator next = endpt->edgesEnd();
    for (where = endpt->edgesBegin(); where != endpt->edgesEnd(); where = next)
    {
      if (where->edgeId() == edg->id())
      { // found the incident edge.
        if (i == 0)
        {
          vertex::incident_edges::iterator tmp = where;
          ++tmp;
          result.first =
            (tmp == endpt->edgesEnd() ? endpt->edgesBegin()->clockwiseFaceId()
                                      : tmp->clockwiseFaceId());
          result.second = where->clockwiseFaceId();
        }
        next = where;
        --next;
        endpt->removeEdgeAt(where);
      }
      ++next;
    }
  }
  return result;
}

/// Remove a reverse lookup (from coordinates to vertex ID) from the model's search structure.
bool pmodel::removeVertexLookup(const Point& location, const Id& vid)
{
  PointToVertexId::const_iterator pit = m_vertices.find(location);
  if (pit == m_vertices.end() || pit->second != vid)
  {
    return false;
  }
  m_vertices.erase(pit);
  return true;
}

#include <typeinfo>

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
  edge::Ptr e = m_session->findStorage<edge>(edgeId);
  if (e)
  {
    if (edgeEndPt)
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
void preparePointsForBoost(
  T& ppts,
  internal::Coord& denx,
  internal::Coord& deny,
  bool useExistingDenominators)
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
      xbhi = pit->x();
      xblo = xbhi;
      ybhi = pit->y();
      yblo = ybhi;
      for (++pit; pit != ppts.end(); ++pit)
      {
        if (xbhi < pit->x())
        {
          xbhi = pit->x();
        }
        else if (xblo > pit->x())
        {
          xblo = pit->x();
        }
        if (ybhi < pit->y())
        {
          ybhi = pit->y();
        }
        else if (yblo > pit->y())
        {
          yblo = pit->y();
        }
      }
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
  smtk::model::Tessellation* smtkTess = faceRec.resetTessellation();
  //std::cout << "Tessellate " << faceRec.name() << "\n";
  for (smtk::model::Loops::iterator lit = outerLoops.begin(); lit != outerLoops.end(); ++lit)
  {
    smtk::model::Loops innerLoops = lit->containedLoops();
    int npp = 1 + static_cast<int>(innerLoops.size());
    std::vector<std::vector<internal::Point>> pp2(npp);
    int ll = 0;
    //std::cout << "  Loop " << lit->name() << "\n";
    this->pointsInLoopOrder(pp2[ll], *lit);
    internal::Coord denx, deny;
    preparePointsForBoost(pp2[ll], denx, deny, false);
    bool denom = denx > 1 || deny > 1;
    pface.set(pp2[ll].rbegin(), pp2[ll].rend()); // boost likes its loops backwards
    poly::assign(polys, pface);
    ++ll;
    for (smtk::model::Loops::iterator ilit = innerLoops.begin(); ilit != innerLoops.end();
         ++ilit, ++ll)
    {
      //std::cout << "    Inner Loop " << ilit->name() << "\n";
      this->pointsInLoopOrder(pp2[ll], *ilit);
      preparePointsForBoost(pp2[ll], denx, deny, true);
      poly::polygon_data<internal::Coord> loop;
      loop.set(pp2[ll].rbegin(), pp2[ll].rend());
      polys -= loop;
    }

    // Add the component to the face tessellation:
    std::vector<poly::polygon_data<internal::Coord>> tess;
    polys.get_trapezoids(tess);
    std::vector<poly::polygon_data<internal::Coord>>::const_iterator pit;
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
      triConn[1] = smtkTess->addCoords(&smtkPt[0]);
      //std::cout << "  " << triConn[1] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      ++pcit;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      this->liftPoint(ipt, &smtkPt[0]);
      triConn[3] = smtkTess->addCoords(&smtkPt[0]);
      ++pcit;
      //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      for (; pcit != poly::end_points(*pit); ++pcit)
      {
        triConn[2] = triConn[3];
        ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
        this->liftPoint(ipt, &smtkPt[0]);
        triConn[3] = smtkTess->addCoords(&smtkPt[0]);
        //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
        smtkTess->insertNextCell(triConn);
      }
      //std::cout << "\n";
      //faceRec.setColor(1., 1., 1., 1.);
    }
  }
  // Now update the bounding box:
  std::vector<double> bbox(6);
  smtk::model::Tessellation::invalidBoundingBox(&bbox[0]);
  smtkTess->getBoundingBox(&bbox[0]);
  faceRec.setBoundingBox(&bbox[0]);
}

void pmodel::addEdgeTessellation(smtk::model::Edge& edgeRec, internal::edge::Ptr edgeData)
{
  if (!edgeRec.isValid() || !edgeData)
    return;

  smtk::model::Resource::Ptr resource = edgeRec.resource();
  Tessellation* smtkTess = edgeRec.resetTessellation();

  // Now populate the tessellation in place.
  PointSeq::const_iterator ptIt;
  std::vector<double> coords(3);
  std::size_t numPts = edgeData->pointsSize();
  std::vector<int> conn;
  conn.reserve(numPts + 2);
  conn.push_back(smtk::model::TESS_POLYLINE);
  conn.push_back(static_cast<int>(numPts));
  bool isPeriodic = (*edgeData->pointsBegin() == *edgeData->pointsRBegin());
  std::size_t numUniquePts = isPeriodic ? numPts - 1 : numPts;
  std::size_t ii;
  for (ptIt = edgeData->pointsBegin(), ii = 0; ii < numUniquePts; ++ptIt, ++ii)
  {
    this->liftPoint(*ptIt, coords.begin());
    conn.push_back(smtkTess->addCoords(&coords[0]));
  }
  if (isPeriodic)
  {
    conn.push_back(0); // repeat initial point instead of adding a duplicate.
  }
  smtkTess->insertCell(0, conn);

  // Now update the bounding box:
  std::vector<double> bbox(6);
  smtk::model::Tessellation::invalidBoundingBox(&bbox[0]);
  smtkTess->getBoundingBox(&bbox[0]);
  edgeRec.setBoundingBox(&bbox[0]);
}

void pmodel::addVertTessellation(smtk::model::Vertex& vertRec, internal::vertex::Ptr vertData)
{
  if (!vertRec.isValid() || !vertData)
    return;

  double snappedPt[3];
  this->liftPoint(vertData->point(), snappedPt);
  smtk::model::Tessellation tess;
  tess.addPoint(snappedPt);
  vertRec.setTessellationAndBoundingBox(&tess);
}

Id pmodel::pointId(const Point& p) const
{
  PointToVertexId::const_iterator it = m_vertices.find(p);
  if (it == m_vertices.end())
    return Id();
  return it->second;
}

/**\brief Tweak an edge into a new shape, which you promise is valid.
  *
  * This variant accepts iterators for a list of *point* objects (not coordinates)
  * in the internal coordinate system of the polygon.
  * **WARNING**: The input \a replacement list is *spliced* into the edge, and thus
  * all entries are removed from \a replacement.
  * This is done so that iterators pointing into \a replacement are valid iterators
  * into the resulting edge; this property is used by the "clean geometry" operator
  * to efficiently reshape and split a model edge at intersection points.
  *
  * If the first and last points are not precisely coincident with the original
  * edge's, then any model vertices are tweaked as well.
  * Faces attached to the edge are retessellated.
  */
bool pmodel::tweakEdge(
  smtk::model::Edge edge,
  internal::PointSeq& replacement,
  smtk::model::EntityRefArray& modified)
{
  edge::Ptr storage = m_session->findStorage<internal::edge>(edge.entity());
  if (!storage)
  {
    return false;
  }

  // Determine whether the original edge was periodic
  internal::PointSeq& epts(storage->points());
  bool isPeriodic = (*epts.begin()) == (*(++epts.rbegin()).base());

  // See which model vertex (if any) matches the existing begin
  smtk::model::Vertices verts = edge.vertices();
  bool isFirstVertStart = true;
  if (!verts.empty())
  {
    internal::vertex::Ptr firstVert =
      m_session->findStorage<internal::vertex>(verts.begin()->entity());
    isFirstVertStart = (firstVert->point() == *epts.begin());
  }
  // Now erase the existing edge points and rewrite them:
  epts.clear();
  epts.splice(epts.end(), replacement);
  if (isPeriodic && (*epts.begin()) != (*(++epts.rbegin()).base()))
  { // It was periodic but isn't any more. Close the loop naively.
    smtkDebugMacro(m_session->log(), "Closing non-periodic tweak to preserve topology.");
    epts.insert(epts.end(), *epts.begin());
  }
  // Lift the integer points into world coordinates:
  this->addEdgeTessellation(edge, storage);
  modified.push_back(edge);

  smtk::model::EntityRefs modEdgesAndFaces;
  for (smtk::model::Vertices::iterator vit = verts.begin(); vit != verts.end(); ++vit)
  {
    internal::Point locn =
      ((vit == verts.begin()) != !isFirstVertStart) ? *epts.begin() : *(++epts.rbegin()).base();
    smtkDebugMacro(m_session->log(), "Tweaking vertex " << vit->name() << ".");
    if (this->tweakVertex(*vit, locn, modEdgesAndFaces))
    {
      modified.push_back(*vit);
    }
  }
  modified.insert(modified.end(), modEdgesAndFaces.begin(), modEdgesAndFaces.end());
  // We must check any attached faces and retessellate them if they haven't been redone already.
  // This can happen on edges with faces but no model vertices or when the model vertex
  // positions have not been changed by the tweak.
  smtk::model::Faces facesOnEdge = edge.faces();
  for (smtk::model::Faces::iterator fit = facesOnEdge.begin(); fit != facesOnEdge.end(); ++fit)
  {
    // If we have a face attached, re-tessellate it and add to modEdgesAndFaces
    if (modEdgesAndFaces.find(*fit) == modEdgesAndFaces.end())
    {
      smtkDebugMacro(m_session->log(), "Retessellating face " << fit->name() << ".");
      this->addFaceTessellation(*fit);
      modEdgesAndFaces.insert(*fit);
    }
  }
  return true;
}

/**\brief Move the model vertex \a vertRec from its current location to \a vertPosn.
  *
  * Note that this should not generally be allowed without a lot of checks to
  * verify that the tweak results in a valid model.
  * No checks are performed here.
  */
bool pmodel::tweakVertex(
  smtk::model::Vertex vertRec,
  const Point& vertPosn,
  smtk::model::EntityRefs& modifiedEdgesAndFaces)
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
  m_vertices.erase(vv->point());
  vv->m_coords = vertPosn;
  m_vertices[vertPosn] = vertRec.entity();

  vertex::incident_edges::iterator eit;
  for (eit = vv->edgesBegin(); eit != vv->edgesEnd(); ++eit)
  {
    smtk::model::Edge edgeRec(vertRec.resource(), eit->edgeId());
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
  m_vertices[vert->point()] = vert->id();
}

} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk
