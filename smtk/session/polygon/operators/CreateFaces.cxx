//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/CreateFaces.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/ActiveFragmentTree.h"
#include "smtk/session/polygon/internal/Config.h"
#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Neighborhood.h"
#include "smtk/session/polygon/internal/Region.h"
#include "smtk/session/polygon/internal/SweepEvent.h"
#include "smtk/session/polygon/internal/Util.h"

#include "smtk/session/polygon/Operation.txx"
#include "smtk/session/polygon/Session.txx"
#include "smtk/session/polygon/internal/Model.txx"
#include "smtk/session/polygon/internal/Neighborhood.txx"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/ShellEntity.txx"

#include "smtk/common/UnionFind.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/polygon/operators/CreateFaces_xml.h"

#include <cmath>
#include <deque>
#include <limits>
#include <map>
#include <set>
#include <vector>

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk
{
namespace session
{
namespace polygon
{

/// An internal structure used to hold a sequence of model edges which form a loop.
struct LoopInfo
{
  internal::Id m_id;
  internal::Rect m_bounds;
  smtk::model::Edges m_edges;
  std::set<internal::Id> m_children; // first-level holes
  bool operator<(const LoopInfo& other) { return ll(m_bounds) < ur(other.m_bounds); }
};

/// An internal structure that holds all the loops discovered, sorted by their lower-left bounding box coordinates.
typedef std::map<internal::Id, LoopInfo> LoopsById;

typedef std::vector<EdgeFragment> FragmentArray; // List of all output fragments forming loops.

typedef std::vector<std::pair<smtk::model::Edge, bool>> OrientedEdges;

static void DumpEventQueue(const char* msg, SweepEventSet& eventQueue)
{
  std::cout << ">>>>>   " << msg << "\n";
  std::cout << ">>>>>   Event Queue:\n";
  SweepEventSet::iterator it;
  for (it = eventQueue.begin(); it != eventQueue.end(); ++it)
  {
    std::cout << "  " << it->type() << ": " << it->point().x() << ", " << it->point().y() << "\n";
  }
  std::cout << "<<<<<   Event Queue\n";
}

/**\brief Populate the list of edges we should use to generate faces.
  *
  * Subclasses may override this method.
  *
  * We keep a set of model edges (m_edgeMap) marked by the
  * directions in which they should be used to form faces.
  * This will constrain what faces may be created without
  * requiring users to pick a point interior
  * to the face.
  *
  * This way, when users specify oriented (CCW) point sequences or
  * a preferred set of edges as outer loop + inner loops, we don't
  * create faces that fill the holes.
  * But when users specify that all possible faces should be created,
  * they don't have to pick interior points.
  *
  * -1 = use only negative orientation
  *  0 = no preferred direction: use in either or both directions
  * +1 = use only positive orientation
  */
bool CreateFaces::populateEdgeMap()
{
  auto modelItem = this->parameters()->associations();
  smtk::model::Model model = modelItem->valueAs<smtk::model::Entity>();
  if (!model.isValid())
  {
    smtkErrorMacro(
      this->log(), "Invalid model (or non-model entity) specified when a model was expected.");
    m_result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
    return false;
  }
  m_model = model;

  // Collect all the edges in this model, not just the free cells:
  smtk::model::Edges allEdges =
    m_resource->entitiesMatchingFlagsAs<smtk::model::Edges>(smtk::model::EDGE, true);
  for (smtk::model::Edges::const_iterator it = allEdges.begin(); it != allEdges.end(); ++it)
  {
    if (it->owningModel() == model)
    {
      m_edgeMap[*it] = 0;
    }
  }
  return true;
}

CreateFaces::Result CreateFaces::operateInternal()
{
  auto modelItem = this->parameters()->associations();
  smtk::model::Model model = modelItem->valueAs<smtk::model::Entity>();

  m_resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(model.component()->resource());

  if (!m_resource)
  {
    // error logging requires mgr...
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  internal::pmodel::Ptr storage; // Look up from session = internal::pmodel::create();

  // First, collect the edges to process:
  if (!this->populateEdgeMap())
  {
    return m_result ? m_result : this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  model = m_model; // should have been set by populateEdgeMap()

  if (m_edgeMap.empty())
  {
    smtkErrorMacro(this->log(), "No edges selected.");
    m_result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
    return m_result;
  }

  // Create a union-find struct
  // for each "model" vertex
  //   for each edge attached to each vertex
  //     add 2 union-find entries (UFEs), 1 per co-edge
  //     merge adjacent pairs of UFEs
  //     store UFEs on edges
  // For each loop, discover nestings and merge UFEs
  // For each edge
  //   For each unprocessed (nesting-wise) UFE
  //     Discover nesting via ray test
  //     Merge parent and child UFEs (if applicable)
  //     Add an (edge, coedge sign) tuple to a "face" identified by the given UFE
  // FIXME: Test for self-intersections?
  // FIXME: Deal w/ pre-existing faces?

  // Create an event queue and populate it with events
  // for each segment of each edge in m_edgeMap.
  ModelEdgeMap::iterator modelEdgeIt;
  internal::Coord xblo = std::numeric_limits<internal::Coord>::lowest();
  internal::Coord xbhi = std::numeric_limits<internal::Coord>::lowest();
  internal::Coord yblo = std::numeric_limits<internal::Coord>::lowest();
  internal::Coord ybhi = std::numeric_limits<internal::Coord>::lowest();
  bool xybinit = false;
  SweepEventSet
    eventQueue; // (QE) sorted into a queue by point-x, point-y, event-type, and then event-specific data.
  for (modelEdgeIt = m_edgeMap.begin(); modelEdgeIt != m_edgeMap.end(); ++modelEdgeIt)
  {
    if (m_debugLevel > 0)
    {
      std::cout << "Considering input edge: " << modelEdgeIt->first.name() << "\n";
    }
    internal::EdgePtr erec = m_resource->findStorage<internal::edge>(modelEdgeIt->first.entity());

    if (erec->pointsSize() < 2)
      continue; // Do not handle edges with < 2 points.

    internal::PointSeq::const_iterator pit = erec->pointsBegin();
    int seg = 0;
    internal::Point last = *pit;
    if (!xybinit)
    {
      xybinit = true;
      xbhi = pit->x();
      xblo = xbhi;
      ybhi = pit->y();
      yblo = ybhi;
    }
    else
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
    for (++pit; pit != erec->pointsEnd(); ++pit, ++seg)
    {
      eventQueue.insert(SweepEvent::SegmentStart(last, *pit, modelEdgeIt->first, seg));
      //eventQueue.insert(SweepEvent::SegmentEnd(*pit, modelEdgeIt->first, seg - 1));
      last = *pit;
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
  }
  m_bdsLo = internal::Point(xblo, yblo);
  m_bdsHi = internal::Point(xbhi, ybhi);
  if (m_debugLevel > 0)
  {
    DumpEventQueue("Initial", eventQueue);
    std::cout << "Bounds: " << xblo << " " << yblo << "    " << xbhi << " " << ybhi << "\n";
  }

  // The first event in eventQueue had better be a segment-start event.
  // So the first thing this event-loop should do is start processing edges.
  // As other edges are added, they must intersect all active edges
  // and add split events as required.
  std::vector<SweepEvent> edgesToInsertAfterAdvance; // FIXME. Needed?
  FragmentArray fragments;                           // (FR)
  fragments.reserve(static_cast<size_t>(
    eventQueue.size() * 1.125)); // pre-allocate some space for additional segments

  internal::Point startPoint = eventQueue.begin()->point();
  startPoint.x(startPoint.x() - 1);
  SweeplinePosition sweepPosn(startPoint);
  ActiveFragmentTree activeEdges(fragments, sweepPosn); // (IT)
  Neighborhood neighborhood(
    sweepPosn, fragments, eventQueue, activeEdges, m_resource->polygonSession()); // N(x)
  neighborhood.setDebugLevel(m_debugLevel);

  neighborhood.sweep(); // Run through events in the queue
  if (m_debugLevel > 0)
  {
    neighborhood.dumpRegions();
  }

  // Now we have loops for each region; iterate over them and
  // create SMTK topology records:
  m_status = smtk::operation::Operation::Outcome::SUCCEEDED;
  m_result = this->createResult(m_status);
  m_model = model;
  neighborhood.getLoops(this);

  {
    smtk::attribute::ComponentItem::Ptr modified = m_result->findComponent("modified");
    modified->setValue(model.component());
  }

  // Finally, tessellate each face using Boost::polygon
  // (although TODO: it would be better to triangulate while sweeping).
  this->addTessellations();

  smtk::attribute::IntItem::Ptr outcome = m_result->findInt("outcome");
  if (static_cast<int>(m_status) != outcome->value())
  {
    outcome->setValue(0, static_cast<int>(m_status));
  }

  return m_result;
}

void CreateFaces::evaluateLoop(
  RegionId faceNumber,
  OrientedEdges& loop,
  std::set<RegionId>& borders)
{
  (void)borders;

  std::map<RegionId, smtk::model::Face>::iterator fit = m_regionFaces.find(faceNumber);
  if (fit != m_regionFaces.end() && !fit->second.isValid())
  { // An invalid face in the region->face map indicates we should ignore all loops bounding the region
    return;
  }

  // Keep track of loops for tessellation
  // TODO: Handle tessellation as part of the Neighborhood sweep instead of re-sweeping with boost.polygon.
  m_regionLoops[faceNumber].push_back(loop);

  // Do not create a face if any edge is already marked as bounding a face on side indicated by the edge-use.
  for (OrientedEdges::const_iterator oit = loop.begin(); oit != loop.end(); ++oit)
  {
    smtk::model::EdgeUses eus = oit->first.edgeUses();
    for (smtk::model::EdgeUses::iterator euit = eus.begin(); euit != eus.end(); ++euit)
    {
      if (euit->orientation() == smtk::model::POSITIVE && oit->second)
      { // This use is co-oriented with the loop. Does it have a face?
        smtk::model::FaceUse fu = euit->faceUse();
        if (fu.isValid() && fu.face().isValid())
        {
          if (m_debugLevel > 0)
          {
            smtkDebugMacro(
              this->log(),
              "  Skipping loop because edge "
                << oit->first.name() << " (" << oit->first.entity() << ")"
                << " is already attached to face " << fu.face().name() << " (" << fu.face().entity()
                << ")");
          }
          m_regionFaces[faceNumber] = smtk::model::Face(); // Mark the region as invalid
          return;
        }
      }
    }
  }

  // Traverse fragments to build a list of oriented model edges.
  // TODO: Handle split edges here or during sweep?
  for (OrientedEdges::const_iterator oit = loop.begin(); oit != loop.end(); ++oit)
  {
    m_model.removeCell(oit->first);
  }

  // Now transcribe SMTK loop-use from edges referenced by fragments in loop
  // ** OR **
  // if loop edges have "faceNumber" material on both sides, add as included edge

  if (fit == m_regionFaces.end())
  { // this is the first (and thus outer) loop for the face
    smtk::common::UUID modelFaceId = m_resource->unusedUUID();
    smtk::common::UUID modelFaceUseId = m_resource->unusedUUID();
    smtk::common::UUID outerLoopId = m_resource->unusedUUID();
    if (m_debugLevel > 0)
    {
      std::cout << "Face " << faceNumber << " " << modelFaceId << " with outer loop " << outerLoopId
                << "\n";
    }
    // Transcribe the outer loop, face use, and face:
    if (!m_resource->insertModelFaceWithOrientedOuterLoop(
          modelFaceId, modelFaceUseId, outerLoopId, loop))
    {
      smtkErrorMacro(this->log(), "Could not create SMTK outer loop of face.");
      m_status = smtk::operation::Operation::Outcome::FAILED;
      return;
    }
    // Update vertex neighborhoods to include new face adjacency.
    smtk::model::Face modelFace(m_resource, modelFaceId);
    m_model.addCell(modelFace);
    modelFace.assignDefaultName();
    this->updateLoopVertices(
      smtk::model::Loop(m_resource, outerLoopId), modelFace, /* isCCW */ true);

    {
      smtk::attribute::ComponentItem::Ptr created = m_result->findComponent("created");
      created->appendValue(modelFace.component());
      operation::MarkGeometry(m_resource).markModified(created);
    }

    if (m_debugLevel > 0)
    {
      std::cout << "Adding " << faceNumber << " as model face "
                << m_result->findComponent("created")->numberOfValues() << "\n";
    }
    m_regionFaces[faceNumber] = modelFace;
  }
  else
  {
    smtk::common::UUID innerLoopId = m_resource->unusedUUID();
    smtk::common::UUID parentLoopId = fit->second.positiveUse().loops()[0].entity();
    if (m_debugLevel > 0)
    {
      std::cout << "Face " << faceNumber << " parent loop " << parentLoopId << " with inner loop "
                << innerLoopId << "\n";
    }
    if (!m_resource->insertModelFaceOrientedInnerLoop(innerLoopId, parentLoopId, loop))
    {
      smtkErrorMacro(this->log(), "Could not create SMTK inner loop of face.");
      m_status = smtk::operation::Operation::Outcome::FAILED;
      return;
    }
    smtk::model::Loop tmp(m_resource, innerLoopId);
    smtk::model::EdgeUses leus = tmp.edgeUses();
    this->updateLoopVertices(
      smtk::model::Loop(m_resource, innerLoopId), fit->second, /* isCCW */ false);
  }
}

/**\brief Visit each edge of the loop and update its vertices' polygon-storage.
  *
  * This method does 2 things:
  * (1) it ensures that the vertex has an incident-edge record for each edge of the loop
  * and (2) it marks the proper indicent-edge record as adjacent to the bordant face (\a brd).
  *
  * The question of which indicent-edge record should be marked is determined by the \a isCCW flag,
  * which indicates to which side of the loop the face lies.
  * When \a isCCW is true, \a brd lies to the left of the edges in the loop (looking from above the
  * face in the direction of the model's normal vector). Otherwise, it lies to the right.
  */
void CreateFaces::updateLoopVertices(
  const smtk::model::Loop& loop,
  const smtk::model::Face& brd,
  bool isCCW)
{
  smtk::model::EdgeUses edgesOfLoop = loop.uses<smtk::model::EdgeUses>();
  smtk::common::UUIDs done;
  if (m_debugLevel > 0)
  {
    std::cout << "Loop " << loop.name() << " " << (isCCW ? "outer" : "inner") << " brd "
              << brd.name() << "\n";
  }
  isCCW = true;
  for (smtk::model::EdgeUses::iterator eit = edgesOfLoop.begin(); eit != edgesOfLoop.end(); ++eit)
  {
    smtk::model::Edge edge = eit->edge();
    if (m_debugLevel > 0)
    {
      std::cout << "  " << edge.name() << "  "
                << (eit->orientation() == smtk::model::POSITIVE ? "+" : "-") << "  " << eit->sense()
                << "\n";
    }
    smtk::model::Vertices verts = edge.vertices();
    /*
    int vertOrder[2][2] = {
      { 0, 1 },
      { 1, 0 }
    };
    int* econn = (eit->sense() == smtk::model::NEGATIVE && verts.size() > 1) ? vertOrder[1] : vertOrder[0];
    */
    /*
    std::cout << "  VerticesFromEdge   ";
    if (verts.size() > 0) { std::cout << " " << verts[0].name(); }
    if (verts.size() > 1) { std::cout << " " << verts[1].name(); }
    */
    if (eit->orientation() == smtk::model::NEGATIVE && verts.size() > 1)
    {
      smtk::model::Vertex tmp = verts[0];
      verts[0] = verts[1];
      verts[1] = tmp;
    }
    /*
    std::cout << "\n";
    std::cout << "  LoopOrderedVertices";
    if (verts.size() > 0) { std::cout << " " << verts[0].name(); }
    if (verts.size() > 1) { std::cout << " " << verts[1].name(); }
    std::cout << "\n";
    */

    // FIXME: Handle case when the same edge is incident to the same model vertex at both ends.
    //        In this case, the face-adjacency could be accidentally reversed because the
    //        wrong incident_edge_data struct of the vertex might be found first and modified.
    for (smtk::model::Vertices::iterator vit = verts.begin(); vit != verts.end(); ++vit)
    {
      internal::vertex::Ptr vrec = m_resource->findStorage<internal::vertex>(vit->entity());
      if (!vrec)
      {
        smtkWarningMacro(
          this->log(), "Could not update vertex on edge " << edge.name() << " " << edge.entity());
        continue;
      }
      vrec->setFaceAdjacency(edge.entity(), brd.entity(), isCCW);
      isCCW = !isCCW;
      // Find incident edge corresponding to "edge"
      // if isCCW, mark its face.
      // if !isCCW, traverse the ring and mark the last entry before the incident edge
    }
  }
}

template<typename T>
void printPts(const std::string& msg, T begin, T end)
{
  std::cout << msg << "\n";
  T pit;
  for (pit = begin; pit != end; ++pit)
  {
    //std::cout << "      " << pit->x()/2.31e13 << " " << pit->y()/2.31e13 << "\n";
    std::cout << "      " << pit->x() << " " << pit->y() << "\n";
  }
}

void CreateFaces::removeFacesFromResult(const smtk::model::EntityRefs& faces)
{
  smtk::attribute::ComponentItem::Ptr cre = m_result->findComponent("created");
  // std::set<smtk::model::Entity::Ptr> cremod(cre->begin(), cre->end());
  std::set<smtk::model::Entity::Ptr> cremod;
  for (auto c = cre->begin(); c != cre->end(); ++c)
  {
    cremod.insert(std::dynamic_pointer_cast<smtk::model::Entity>(*c));
  }

  for (smtk::model::EntityRefs::const_iterator fit = faces.begin(); fit != faces.end(); ++fit)
  {
    cremod.erase(fit->entityRecord());
  }
  smtkErrorMacro(
    this->log(), faces.size() << " faces had an empty tessellation; removing them from the model.");

  smtk::model::EntityRefArray modified;
  smtk::model::EntityRefArray expunged;
  m_resource->polygonSession()->consistentInternalDelete(
    faces, modified, expunged, m_debugLevel > 0);

  // Rewrite result, excluding the delete faces.
  smtk::model::EntityRefArray crevec(cremod.begin(), cremod.end());
  cre->setNumberOfValues(crevec.size());
  int i = 0;
  for (auto c = crevec.begin(); c != crevec.end(); ++c, ++i)
  {
    cre->setValue(i, c->entityRecord());
  }
}

void CreateFaces::addTessellations()
{
  // See if we need to prevent boost from overflows and crashes
  // by truncating coordinates to 31 bits.
  internal::Coord dx = m_bdsHi.x();
  if (m_bdsLo.x() < 0 && -m_bdsLo.x() > dx)
  {
    dx = -m_bdsLo.x();
  }
  internal::Coord dy = m_bdsHi.y();
  if (m_bdsLo.y() < 0 && -m_bdsLo.y() > dy)
  {
    dy = -m_bdsLo.y();
  }
  double lx = dx > 0 ? (std::log(dx) / std::log(2.0)) : 1.0;
  double ly = dy > 0 ? (std::log(dy) / std::log(2.0)) : 1.0;
  internal::Coord denx = lx > 31 ? (1 << static_cast<int>(std::ceil(lx - 31))) : 1;
  internal::Coord deny = ly > 31 ? (1 << static_cast<int>(std::ceil(ly - 31))) : 1;
  bool denom = denx > 1 || deny > 1;

  smtk::model::EntityRefs emptyFaces; // Faces that should not have been created.
  std::map<RegionId, std::vector<OrientedEdges>>::iterator rit; // Face iterator
  for (rit = m_regionLoops.begin(); rit != m_regionLoops.end(); ++rit)
  {
    if (m_debugLevel > 2)
    {
      std::cout << "Tessellate region " << rit->first << "\n";
    }
    // Look up SMTK face and owning model from region number:
    smtk::model::Face modelFace = m_regionFaces[rit->first];
    if (!modelFace.isValid())
    { // Do not attempt to tessellate an invalid face (we expect the face to already exist).
      continue;
    }
    smtk::model::Model model = modelFace.owningModel();
    internal::pmodel::Ptr pmodel = m_resource->findStorage<internal::pmodel>(model.entity());

    // Now traverse loops of face to create boost::poly tessellation:
    std::vector<OrientedEdges>::iterator lit; // Loops-of-face iterator
    poly::polygon_set_data<internal::Coord> polys;
    poly::polygon_data<internal::Coord> pface;
    bool isOuter = true;
    size_t npp = rit->second.end() - rit->second.begin();
    std::vector<std::vector<internal::Point>> pp2(npp);
    size_t ppi = 0;
    for (lit = rit->second.begin(); lit != rit->second.end(); ++lit, ++ppi)
    {
      this->pointsInLoopOrderFromOrientedEdges(pp2[ppi], lit->begin(), lit->end(), pmodel);
      if (isOuter)
      {
        isOuter = false;
        if (denom)
        {
          for (auto fpit = pp2[ppi].rbegin(); fpit != pp2[ppi].rend(); ++fpit)
          {
            fpit->x(fpit->x() / denx);
            fpit->y(fpit->y() / deny);
          }
        }
        pface.set(pp2[ppi].rbegin(), pp2[ppi].rend());
        if (m_debugLevel > 2)
        {
          printPts("  Outer", pp2[ppi].rbegin(), pp2[ppi].rend());
        }
        poly::assign(polys, pface);
      }
      else
      {
        poly::polygon_data<internal::Coord> loop;
        if (denom)
        {
          for (auto fpit = pp2[ppi].rbegin(); fpit != pp2[ppi].rend(); ++fpit)
          {
            fpit->x(fpit->x() / denx);
            fpit->y(fpit->y() / deny);
          }
        }
        loop.set(pp2[ppi].rbegin(), pp2[ppi].rend());
        if (m_debugLevel > 2)
        {
          printPts("  Subtract Inner", pp2[ppi].rbegin(), pp2[ppi].rend());
        }
        polys -= loop;
      }
    }

    std::vector<poly::polygon_data<internal::Coord>> tess;
    polys.get_trapezoids(tess);
    //polys.get(tess);
    if (tess.empty())
    {
      emptyFaces.insert(modelFace);
      continue; // Skip to next face/region.
    }

    std::vector<poly::polygon_data<internal::Coord>>::const_iterator pit;
    smtk::model::Tessellation blank;
    smtk::model::UUIDsToTessellations::iterator smtkTess =
      m_resource->setTessellationAndBoundingBox(modelFace.entity(), blank);
    double smtkPt[3];
    for (pit = tess.begin(); pit != tess.end(); ++pit)
    {
      //std::cout << "Fan\n";
      poly::polygon_data<internal::Coord>::iterator_type pcit;
      internal::Point ipt;
      pcit = poly::begin_points(*pit);
      std::vector<int> triConn;
      triConn.resize(4);
      triConn[0] = smtk::model::TESS_TRIANGLE;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      pmodel->liftPoint(ipt, &smtkPt[0]);
      triConn[1] = smtkTess->second.addCoords(&smtkPt[0]);
      //std::cout << "  " << triConn[1] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      ++pcit;
      ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
      pmodel->liftPoint(ipt, &smtkPt[0]);
      triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
      ++pcit;
      //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      for (; pcit != poly::end_points(*pit); ++pcit)
      {
        triConn[2] = triConn[3];
        ipt = !denom ? *pcit : internal::Point(pcit->x() * denx, pcit->y() * deny);
        pmodel->liftPoint(ipt, &smtkPt[0]);
        triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
        //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
        smtkTess->second.insertNextCell(triConn);
      }
      //std::cout << "\n";
      //modelFace.setColor(1., 1., 1., 1.);
    }
    // Now update the bounding box:
    std::vector<double> bbox(6);
    smtk::model::Tessellation::invalidBoundingBox(bbox.data());
    smtkTess->second.getBoundingBox(bbox.data());
    modelFace.setBoundingBox(bbox.data());
  }

  if (!emptyFaces.empty())
  {
    this->removeFacesFromResult(emptyFaces);
  }
}

const char* CreateFaces::xmlDescription() const
{
  return CreateFaces_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk
