//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/CreateFaces.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/ActiveFragmentTree.h"
#include "smtk/bridge/polygon/internal/Config.h"
#include "smtk/bridge/polygon/internal/Edge.h"
#include "smtk/bridge/polygon/internal/Neighborhood.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Region.h"
#include "smtk/bridge/polygon/internal/SweepEvent.h"
#include "smtk/bridge/polygon/internal/Util.h"

#include "smtk/bridge/polygon/Operator.txx"
#include "smtk/bridge/polygon/internal/Neighborhood.txx"
#include "smtk/bridge/polygon/internal/Model.txx"

#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/common/UnionFind.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateFaces_xml.h"

#include <deque>
#include <map>
#include <set>
#include <vector>

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk {
  namespace bridge {
    namespace polygon {

/// An internal structure used when discovering edge loops.
struct ModelEdgeInfo
{
  ModelEdgeInfo()
    : m_allowedOrientations(0)
    {
    this->m_visited[0] = this->m_visited[1] = false;
    }
  ModelEdgeInfo(int allowedOrientations)
    {
    this->m_allowedOrientations = allowedOrientations > 0 ? +1 : allowedOrientations < 0 ? -1 : 0;
    this->m_visited[0] = this->m_visited[1] = false;
    }
  ModelEdgeInfo(const ModelEdgeInfo& other)
    : m_allowedOrientations(other.m_allowedOrientations)
    {
    for (int i = 0; i < 2; ++i)
      m_visited[i] = other.m_visited[i];
    }

  int m_allowedOrientations; // 0: all, -1: only negative, +1: only positive
  bool m_visited[2]; // has the [0]: negative, [1]: positive orientation of the edge been visited already?
};

/// An internal structure used to map model edges to information about the space between them.
typedef std::map<smtk::model::Edge, ModelEdgeInfo> ModelEdgeMap;

/// An internal structure used to hold a sequence of model edges which form a loop.
struct LoopInfo
{
  internal::Id m_id;
  internal::Rect m_bounds;
  smtk::model::Edges m_edges;
  std::set<internal::Id> m_children; // first-level holes
  bool operator < (const LoopInfo& other)
    {
    return ll(this->m_bounds) < ur(other.m_bounds);
    }
};

/// An internal structure that holds all the loops discovered, sorted by their lower-left bounding box coordinates.
typedef std::map<internal::Id,LoopInfo> LoopsById;

typedef std::vector<EdgeFragment> FragmentArray; // List of all output fragments forming loops.

typedef std::vector<std::pair<smtk::model::Edge,bool> > OrientedEdges;

#if 0
static void AddLoopsForEdge(
  CreateFaces* op,
  ModelEdgeMap& modelEdgeMap,
  ModelEdgeMap::iterator edgeInfo,
  LoopsById& loops,
  smtk::model::VertexSet& visitedVerts,
  std::map<internal::Point, int>& visitedPoints // number of times a point has been encountered (not counting periodic repeat at end of a single-edge loop); used to identify points that must be promoted to model vertices.
)
{
  if (!edgeInfo->first.isValid() || !op)
    {
    return; // garbage-in? garbage-out.
    }
  internal::EdgePtr edgeRec = op->findStorage<internal::edge>(edgeInfo->first.entity());

  smtk::model::Vertices endpts = edgeInfo->first.vertices();
  if (endpts.empty())
    { // Tessellation had better be a periodic loop. Traverse for bbox.
    //AddEdgePointsToBox(tess, box);
    }
  else
    { // Choose an endpoint and walk around the edge.
    }
}
#endif // 0

void DumpEventQueue(const char* msg, SweepEventSet& eventQueue)
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

smtk::model::OperatorResult CreateFaces::operateInternal()
{
  // Discover how the user wants to specify scaling.
  smtk::attribute::IntItem::Ptr constructionMethodItem = this->findInt("construction method");
  int method = constructionMethodItem->discreteIndex(0);

  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");
  smtk::attribute::IntItem::Ptr offsetsItem = this->findInt("offsets");

  smtk::attribute::ModelEntityItem::Ptr edgesItem = this->findModelEntity("edges");

  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  smtk::model::Model model;

  internal::pmodel::Ptr storage; // Look up from session = internal::pmodel::create();
  bool ok = true;

  Session* sess = this->polygonSession();
  smtk::model::ManagerPtr mgr;
  if (!sess || !(mgr = sess->manager()))
    {
    // error logging requires mgr...
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  // Keep a set of model edges marked by the directions in which they
  // should be used to form faces. This will constrain what faces
  // may be created without requiring users to pick a point interior
  // to the face.
  //
  // This way, when users specify oriented (CCW) point sequences or
  // a preferred set of edges as outer loop + inner loops, we don't
  // create faces that fill the holes.
  // But when users specify that all possible faces should be created,
  // they don't have to pick interior points.
  //
  // -1 = use only negative orientation
  //  0 = no preferred direction: use in either or both directions
  // +1 = use only positive orientation
  ModelEdgeMap modelEdgeMap;

  // First, collect or create edges to process:
  // These case values match CreateFaces.sbt indices (and enum values):
  switch (method)
    {
  case 0: // points, coordinates, offsets
      {
      // identify pre-existing model vertices from points
      // verify that existing edges/faces incident to model vertices
      //        do not impinge on proposed edge/face
      // run edge-creation pre-processing on each point-sequence?
      // determine sub-sequences of points that will
      //   (a) form new edges
      //   (b) make use of existing edges (with orientation)
      // determine loop nesting and edge splits required by intersecting loops
      // report point sequences, model vertices (existing, imposed by intersections, non-manifold), loops w/ nesting
      // ---
      // create new vertices as required
      // create edges on point sequences
      // modify/create vertex uses
      // create chains
      // create edge uses
      // create loops
      // create faces
      }
    break;
  case 1: // edges, points, coordinates
      {
      if (!model.isValid())
        {
        smtkErrorMacro(this->log(), "Invalid model (or non-model entity) specified when a model was expected.");
        return this->createResult(smtk::model::OPERATION_FAILED);
        }
      for (int i = 0; i < modelItem->numberOfValues(); ++i)
        {
        smtk::model::Edge edgeIn(modelItem->value(i));
        if (edgeIn.isValid())
          {
          modelEdgeMap[edgeIn] = 0;
          }
        }
      // for each edge
      //   for each model vertex
      //     walk loops where vertices have no face, aborting walk if an unselected edge is found.
      //     mark traversed regions and do not re-traverse
      //   OR IF NO MODEL VERTICES
      //     edge must be periodic and oriented properly... treat it as a loop to bound a hole+/^face-filling-the-hole.
      //     mark traversed regions and do not re-traverse
      //   determine loop nesting and edge splits required by intersecting loops
      //   report model vertices (imposed by intersections, non-manifold), loops w/ nesting
      // ---
      // create new vertices as required
      // modify vertex uses
      // create edge uses
      // create loops
      // create faces
      }
    break;
  case 2: // all non-overlapping
      {
      model = modelItem->value(0);
      if (!model.isValid())
        {
        smtkErrorMacro(this->log(), "Invalid model (or non-model entity) specified when a model was expected.");
        return this->createResult(smtk::model::OPERATION_FAILED);
        }
      smtk::model::Edges allEdges =
        model.cellsAs<smtk::model::Edges>();
      for (smtk::model::Edges::const_iterator it = allEdges.begin(); it != allEdges.end(); ++it)
        {
        modelEdgeMap[*it] = 0;
        }
      // Same as case 1 but with the set of all edges in model.
      //
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
      }
    break;
  default:
    ok = false;
    smtkInfoMacro(log(), "Unhandled construction method " << method << ".");
    break;
    }

  // Create an event queue and populate it with events
  // for each segment of each edge in modelEdgeMap.
  ModelEdgeMap::iterator modelEdgeIt;
  SweepEventSet eventQueue; // (QE) sorted into a queue by point-x, point-y, event-type, and then event-specific data.
  for (modelEdgeIt = modelEdgeMap.begin(); modelEdgeIt != modelEdgeMap.end(); ++modelEdgeIt)
    {
    if (this->m_debugLevel > 0)
      {
      std::cout << "Considering input edge: " << modelEdgeIt->first.name() << "\n";
      }
    internal::EdgePtr erec =
      this->findStorage<internal::edge>(
        modelEdgeIt->first.entity());

    if (erec->pointsSize() < 2)
      continue; // Do not handle edges with < 2 points.

    internal::PointSeq::const_iterator pit = erec->pointsBegin();
    int seg = 0;
    internal::Point last = *pit;
    for (++pit; pit != erec->pointsEnd(); ++pit, ++seg)
      {
      eventQueue.insert(SweepEvent::SegmentStart(last, *pit, modelEdgeIt->first, seg));
      //eventQueue.insert(SweepEvent::SegmentEnd(*pit, modelEdgeIt->first, seg - 1));
      last = *pit;
      }
    }
  if (this->m_debugLevel > 0)
    {
    DumpEventQueue( "Initial", eventQueue);
    }

  // The first event in eventQueue had better be a segment-start event.
  // So the first thing this event-loop should do is start processing edges.
  // As other edges are added, they must intersect all active edges
  // and add split events as required.
  std::vector<SweepEvent> edgesToInsertAfterAdvance; // FIXME. Needed?
  FragmentArray fragments; // (FR)
  fragments.reserve(static_cast<size_t>(eventQueue.size() * 1.125)); // pre-allocate some space for additional segments

  internal::Point startPoint = eventQueue.begin()->point();
  startPoint.x(startPoint.x() - 1);
  SweeplinePosition sweepPosn(startPoint);
  ActiveFragmentTree activeEdges(fragments, sweepPosn); // (IT)
  Neighborhood neighborhood(sweepPosn, fragments, eventQueue, activeEdges, this->polygonSession()); // N(x)
  neighborhood.setDebugLevel(this->m_debugLevel);

  neighborhood.sweep(); // Run through events in the queue
  if (this->m_debugLevel > 0)
    {
    neighborhood.dumpRegions();
    }

  // Now we have loops for each region; iterate over them and
  // create SMTK topology records:
  this->m_status = smtk::model::OPERATION_SUCCEEDED;
  this->m_result = this->createResult(this->m_status);
  this->m_model = model;
  neighborhood.getLoops(this);

  // Finally, tessellate each face using Boost::polygon
  // (although TODO: it would be better to triangulate while sweeping).
  this->addTessellations();

	smtk::attribute::IntItem::Ptr outcome =
		this->m_result->findInt("outcome");
  if (this->m_status != outcome->value())
    {
		outcome->setValue(0, this->m_status);
    }

  return this->m_result;
}

void CreateFaces::evaluateLoop(
  RegionId faceNumber,
  OrientedEdges& loop,
  std::set<RegionId>& borders)
{
	// Keep track of loops for tessellation
  // TODO: Handle tessellation as part of the Neighborhood sweep instead of re-sweeping with boost.polygon.
  this->m_regionLoops[faceNumber].push_back(loop);

  // Traverse fragments to build a list of oriented model edges.
  // TODO: Handle split edges here or during sweep?
  for (OrientedEdges::const_iterator oit = loop.begin(); oit != loop.end(); ++oit)
		{
    this->m_model.removeCell(oit->first);
		}

	smtk::model::Manager::Ptr mgr = this->manager();
  std::map<RegionId, smtk::model::Face>::iterator fit = this->m_regionFaces.find(faceNumber);
  // Now transcribe SMTK loop-use from edges referenced by fragments in loop
  // ** OR **
  // if loop edges have "faceNumber" material on both sides, add as included edge
  if (fit == this->m_regionFaces.end())
    { // this is the first (and thus outer) loop for the face
    smtk::common::UUID modelFaceId = mgr->unusedUUID();
    smtk::common::UUID modelFaceUseId = mgr->unusedUUID();
    smtk::common::UUID outerLoopId = mgr->unusedUUID();
    if (this->m_debugLevel > 0)
      {
      std::cout << "Face " << faceNumber << " " << modelFaceId << " with outer loop " << outerLoopId << "\n";
      }
    // Transcribe the outer loop, face use, and face:
    if (!mgr->insertModelFaceWithOrientedOuterLoop(modelFaceId, modelFaceUseId, outerLoopId, loop))
      {
      smtkErrorMacro(this->log(), "Could not create SMTK outer loop of face.");
			this->m_status = smtk::model::OPERATION_FAILED;
      return;
      }
    smtk::model::Face modelFace(mgr, modelFaceId);
    this->m_model.addCell(modelFace);
		this->addEntityToResult(this->m_result, modelFace, CREATED);
    if (this->m_debugLevel > 0)
      {
      std::cout
        << "Adding " << faceNumber << " as model face "
        << this->m_result->findModelEntity("created")->numberOfValues() << "\n";
      }
		this->m_regionFaces[faceNumber] = modelFace;
    }
  else
    {
    smtk::common::UUID innerLoopId = mgr->unusedUUID();
		smtk::common::UUID parentLoopId = fit->second.positiveUse().loops()[0].entity();
    if (this->m_debugLevel > 0)
      {
      std::cout
        << "Face " << faceNumber << " parent loop " << parentLoopId
        << " with inner loop " << innerLoopId << "\n";
      }
		if (!mgr->insertModelFaceOrientedInnerLoop(innerLoopId, parentLoopId, loop))
			{
      smtkErrorMacro(this->log(), "Could not create SMTK outer loop of face.");
			this->m_status = smtk::model::OPERATION_FAILED;
      return;
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

void CreateFaces::addTessellations()
{
  std::map<RegionId, std::vector<OrientedEdges> >::iterator rit; // Face iterator
  for (rit = this->m_regionLoops.begin(); rit != this->m_regionLoops.end(); ++rit)
    {
    if (this->m_debugLevel > 2)
      {
      std::cout << "Tessellate region " << rit->first << "\n";
      }
    // Look up SMTK face and owning model from region number:
    smtk::model::Face modelFace = this->m_regionFaces[rit->first];
    smtk::model::Model model = modelFace.owningModel();
    internal::pmodel::Ptr pmodel = this->findStorage<internal::pmodel>(model.entity());

    // Now traverse loops of face to create boost::poly tessellation:
    std::vector<OrientedEdges>::iterator lit; // Loops-of-face iterator
    poly::polygon_set_data<internal::Coord> polys;
#if 0
    poly::polygon_with_holes_data<internal::Coord> pface;
    std::vector<poly::polygon_data<internal::Coord> > holes;
#else
    poly::polygon_data<internal::Coord> pface;
#endif
    bool isOuter = true;
    size_t npp = rit->second.end() - rit->second.begin();
    std::vector<std::vector<internal::Point> > pp2(npp);
    size_t ppi = 0;
    for (lit = rit->second.begin(); lit != rit->second.end(); ++lit, ++ppi)
      {
      this->pointsInLoopOrderFromOrientedEdges(
        pp2[ppi], lit->begin(), lit->end(), pmodel);
      if (isOuter)
        {
        isOuter = false;
        pface.set(pp2[ppi].rbegin(), pp2[ppi].rend());
        if (this->m_debugLevel > 2)
          {
          printPts("  Outer", pp2[ppi].rbegin(), pp2[ppi].rend());
          }
        poly::assign(polys, pface);
        }
      else
        {
        poly::polygon_data<internal::Coord> loop;
        loop.set(pp2[ppi].rbegin(), pp2[ppi].rend());
        if (this->m_debugLevel > 2)
          {
          printPts("  Subtract Inner", pp2[ppi].rbegin(), pp2[ppi].rend());
          }
#if 0
        holes.push_back(loop);
#else
        polys -= loop;
#endif
        }
      }
#if 0
    pface.set_holes(holes.begin(), holes.end());
    polys.insert(pface);
#endif

    std::vector<poly::polygon_data<internal::Coord> > tess;
    polys.get_trapezoids(tess);
    //polys.get(tess);
    std::vector<poly::polygon_data<internal::Coord> >::const_iterator pit;
    smtk::model::Tessellation blank;
    smtk::model::UUIDsToTessellations::iterator smtkTess =
      this->manager()->setTessellation(modelFace.entity(), blank);
    double smtkPt[3];
    for (pit = tess.begin(); pit != tess.end(); ++pit)
      {
      //std::cout << "Fan\n";
      poly::polygon_data<internal::Coord>::iterator_type pcit;
      pcit = poly::begin_points(*pit);
      std::vector<int> triConn;
      triConn.resize(4);
      triConn[0] = smtk::model::TESS_TRIANGLE;
      pmodel->liftPoint(*pcit, &smtkPt[0]);
      triConn[1] = smtkTess->second.addCoords(&smtkPt[0]);
      //std::cout << "  " << triConn[1] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      ++pcit;
      pmodel->liftPoint(*pcit, &smtkPt[0]);
      triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
      ++pcit;
      //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      for (; pcit != poly::end_points(*pit); ++pcit)
        {
        triConn[2] = triConn[3];
        pmodel->liftPoint(*pcit, &smtkPt[0]);
        triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
        //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
        smtkTess->second.insertNextCell(triConn);
        }
      //std::cout << "\n";
      //modelFace.setColor(1., 1., 1., 1.);
      }
    }
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::CreateFaces,
  polygon_create_faces,
  "create faces",
  CreateFaces_xml,
  smtk::bridge::polygon::Session);
