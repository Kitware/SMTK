//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/CreateEdge.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateEdge_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

typedef std::vector<std::pair<size_t, internal::Segment> > SegmentSplitsT;

/*
template<typename T>
void printSegment(internal::pmodel::Ptr storage, const std::string& msg, T& seg)
{
  std::vector<double> lo(3);
  std::vector<double> hi(3);
  storage->liftPoint(seg.low(), lo.begin());
  storage->liftPoint(seg.high(), hi.begin());
  std::cout
    << msg
    << "    " << lo[0] << " " << lo[1] << " " << lo[2]
    << " -> " << hi[0] << " " << hi[1] << " " << hi[2]
    << "\n";
}
*/

smtk::model::OperatorResult CreateEdge::operateInternal()
{
  smtk::bridge::polygon::Session* sess = this->polygonSession();
  smtk::model::Manager::Ptr mgr;
  if (!sess)
    return this->createResult(smtk::model::OPERATION_FAILED);

  mgr = sess->manager();
  // Discover how the user wants to specify scaling.
  smtk::attribute::IntItem::Ptr constructionMethodItem = this->findInt("construction method");
  // This value matches CreateEdge.sbt index (and enum value):
  int method = constructionMethodItem->discreteIndex(0);

  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");
  smtk::attribute::IntItem::Ptr offsetsItem = this->findInt("offsets");

  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  // Either modelItem contains a single Model or 2+ Vertex entities.
  // If method == 0 (points), use the owningModel of any Vertex or complain
  // If method == 1 (vertices), complain if the entities are not vertices or there are too few.
  // If method == 2 (interactive widget), after widget interaction, same requirements are needed
  //    as method == 0 (points) because widget representation points will be set to "points", etc.
  smtk::model::Model parentModel(modelItem->value(0));
  if (!parentModel.isValid())
    {
    parentModel = modelItem->value(0).owningModel();
    if (!parentModel.isValid() || (method == 1 && modelItem->numberOfValues() < 2))
      {
      smtkErrorMacro(this->log(),
        "A model (or vertices with a valid parent model) must be associated with the operator.");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    }
  if (method == 1 && (!modelItem->value(0).isVertex() || modelItem->numberOfValues() < 2))
    {
    smtkErrorMacro(this->log(),
      "When constructing an edge from vertices,"
      " all associated model entities must be vertices"
      " and there must be at least 2 vertices");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  internal::pmodel::Ptr storage =
    this->findStorage<internal::pmodel>(
      parentModel.entity());
  bool ok = true;
  int numEdges = offsetsItem->numberOfValues();
  int numCoordsPerPt = coordinatesItem->value(0);
  if ((method == 0 || method == 2) && numCoordsPerPt == 0)
    {
    smtkErrorMacro(this->log(),
      "When constructing an edge from points or interactive widget,"
      "the number of coordinates per point must be specified!");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // numPts is the number of points total (across all edges)
  long long numPts =
    ((method == 0 || method == 2) ?
     pointsItem->numberOfValues() / numCoordsPerPt : // == #pts / #coordsPerPt
     modelItem->numberOfValues());
  int ei;
  smtk::model::Edges created;
  // Process each edge individually:
  for (ei = 0; ei < numEdges; ++ei)
    {
    long long edgeOffset = offsetsItem->value(ei);
    long long edgeEnd = (ei < numEdges - 1 ? offsetsItem->value(ei + 1) : numPts);
    long long numSegments = edgeEnd - edgeOffset - 1;
    if (numSegments < 1 || edgeEnd > numPts)
      {
      smtkWarningMacro(this->log(),
        "Ignoring input " << ei << " (offset " << edgeOffset << " to " << edgeEnd << ")" <<
        " with not enough points or offset past end of points.");
      continue; // skip "edges" with only 0 or 1 vertices for their entire path.
      }

    // Fill in a list of segments for the edge so we can
    // check for self-intersections.
    std::list<internal::Segment> edgeSegs;
    bool edgeIsPeriodic = true;
    switch (method)
      {
    case 0: // points, coordinates, offsets
    case 2: // interactive widget, representation points will be set to points, coords, offsets
        {
        std::vector<double> pt(numCoordsPerPt, 0.);
        internal::Point curr;
        internal::Point prev;
        internal::Point orig;
        bool first = true;
        for (; edgeOffset < edgeEnd; ++edgeOffset, prev = curr)
          {
          for (int i = 0; i < numCoordsPerPt; ++i)
            pt[i] = pointsItem->value(edgeOffset * numCoordsPerPt + i);
          curr = storage->projectPoint(pt.begin(), pt.end());
          if (!first)
            {
            edgeSegs.push_back(internal::Segment(prev, curr));
            }
          else
            {
            first = false;
            orig = curr;
            }
          }
        if (!first && orig != curr)
          { // non-periodic edge forces endpts to be model vertices
          smtk::model::Vertex vs = storage->findOrAddModelVertex(mgr, orig);
          smtk::model::Vertex ve = storage->findOrAddModelVertex(mgr, curr);
          edgeIsPeriodic = false;
          }
        }
      break;
    case 1: // vertices, offsets
        {
        internal::Point curr;
        internal::Point prev;
        bool first = true;
        edgeIsPeriodic = (modelItem->value(edgeOffset) == modelItem->value(edgeEnd));
        for (; edgeOffset < edgeEnd; ++edgeOffset, prev = curr)
          {
          internal::vertex::Ptr vert =
            this->findStorage<internal::vertex>(modelItem->value(edgeOffset).entity());
          if (!vert)
            {
            ok = false;
            smtkErrorMacro(
              sess->log(),
              "vertices item " << edgeOffset << " not a valid vertex.");
            }
          curr = vert->point();
          if (!first)
            {
            edgeSegs.push_back(internal::Segment(prev, curr));
            }
          else
            {
            first = false;
            }
          }
        }
      break;
    default:
      ok = false;
      smtkInfoMacro(log(), "Unhandled construction method " << method << ".");
      break;
      }

    std::list<internal::Segment>::const_iterator edgeIt;
    if (ok)
      { // Inside here, we know we got a list of segments for this edge.

      // Intersect the segments with each other.
      // Intersection points become model vertices and cause the edge
      // to be split into possibly-multiple actual edges.
      SegmentSplitsT result;
      boost::polygon::intersect_segments(result, edgeSegs.begin(), edgeSegs.end());
      /*
      std::cout
        << "Intersected " << numSegments
        << " segments of edge " << ei << "."
        << " Result has " << result.size()
        << " segments:\n";
        */
      if (result.empty())
        {
        smtkErrorMacro(this->log(), "Self-intersection of edge segments was empty set.");
        return this->createResult(smtk::model::OPERATION_FAILED);
        }

      // I. Pre-process the intersected segments
      //
      // We perform two tasks to prepare the intersection results for
      // edge creation:
      //
      // A. Reordering segments of periodic edges with model vertices
      // When an edge is periodic (i.e., its first and last points are
      // identical), it may get split into multiple periodic loops (if
      // it self-intersects) or it might contain one or more pre-existing
      // model vertices that split the edge and must serve as endpoints.
      // In either of these circumstances, if the initial point is not a
      // model vertex, we would rather not force it to become one; so,
      // we move the first points that are not model vertices to the end
      // of the intersection results.
      //
      // B. Reorienting inverted segments head-to-tail.
      // We must reorder the intersected results so they match the
      // original direction of the input edges. This is tricky because
      // where an intersection occurs, if any one segment's record is
      // pointing the wrong direction, all the records for the segment
      // will be in reverse order. For example if we have edgeSegs =
      // {{a,b}, {b,c}, {c,d}, {d,e}} we can end up with result =
      // {0:{a,b}, 1:{c,f}, 1:{f,b}, 2:{d,c}, 3:{d,g}, 3:{g,e}}
      // depending on the arrangement of the points a--e.
      //
      // What we must do is both swap endpoints of some segments
      // reverse the order of swapped segments that share the same
      // source (only reversing those that have been swapped) to
      // obtain result =
      // {0:{a,b}, 1:{b,f}, 1:{f,c}, 2:{c,d}, 3:{d,g}, 3:{g,e}}.

      edgeIt = edgeSegs.begin(); // Keep an iterator pointing to the source segment of the intersected results.
      SegmentSplitsT::iterator segStart = result.begin();
      //printSegment(storage, "seg start", segStart->second); // DBG
      SegmentSplitsT::iterator segEnd;
      SegmentSplitsT::iterator firstModelVertex;
      bool haveFirstModelVertex = false;
      // Loop over all intersection-output segments by their input segment (edgeIt):
      for (SegmentSplitsT::iterator sit = result.begin(); sit != result.end(); ++edgeIt)
        {
        std::size_t numSegsPerSrc = 0; // Number of result segs per input edge in edgeSegs
        // Determine whether segments are reversed from the input edge:
        //printSegment(storage, "Seg ", sit->second);
        internal::HighPrecisionPoint deltaSrc =
          internal::HighPrecisionPoint(
            edgeIt->high().x() - edgeIt->low().x(),
            edgeIt->high().y() - edgeIt->low().y());
        internal::HighPrecisionPoint deltaDst =
          internal::HighPrecisionPoint(
            sit->second.high().x() - sit->second.low().x(),
            sit->second.high().y() - sit->second.low().y());
        // Whether the segments are reversed or not, determine which
        // output segments correspond to a single input segment:
        if (deltaDst.x() * deltaSrc.x() < 0 || deltaDst.y() * deltaSrc.y() < 0)
          {
          segStart = sit;
          for (segEnd = sit; segEnd != result.end() && segEnd->first == segStart->first; ++segEnd)
            {
            internal::Segment flipped(segEnd->second.high(), segEnd->second.low());
            segEnd->second = flipped;
            ++numSegsPerSrc;
            }
          // NB: after reverse(), segStart still points to beginning...
          // its contents are swapped with (segEnd-1):
          std::reverse(segStart, segEnd);
          }
        else
          { // Advance sit to end of entries for the input segment.
          segStart = sit;
          segEnd = sit;
          for (segEnd = segStart; segEnd != result.end() && segEnd->first == segStart->first; ++segEnd)
            ++numSegsPerSrc;
          }
        // Process all the output segments (i.e., [segStart,segEnd)) for the current input segment:
        sit = segStart;
        // If the first point in the first ouput segment for any input-segment is
        // a model vertex, make a note of it for periodic edges:
        if (edgeIsPeriodic && storage->pointId(sit->second.low()) && !haveFirstModelVertex)
          {
          haveFirstModelVertex = true;
          firstModelVertex = sit;
          }
        // If numSegsPerSrc > 1, we have interior model vertices where intersections occur.
        // Promote each of those points to model vertices.
        for (std::size_t i = 1; i < numSegsPerSrc; ++i)
          {
          smtk::model::Vertex vs = storage->findOrAddModelVertex(mgr, sit->second.high());
          ++sit;
          if (edgeIsPeriodic && !haveFirstModelVertex)
            {
            haveFirstModelVertex = true;
            firstModelVertex = sit;
            }
          }
        sit = segEnd;
        }
      // Move any non-model-vertex points on periodic edges that contain at least
      // one model vertex to the end of the edge list.
      if (edgeIsPeriodic && haveFirstModelVertex)
        {
        //result.splice(result, result.end(), result.begin(), firstModelVertex);
        SegmentSplitsT tmp(result.begin(), firstModelVertex);
        result.erase(result.begin(), firstModelVertex);
        result.insert(result.end(), tmp.begin(), tmp.end());
        }

      // II. Generate edge(s) as required.
      //
      // All intersection points have been marked as model edges
      // and all segments are now in proper head-to-tail order.
      // If the edge is periodic and has any model vertices, the
      // first segment is now guaranteed to start with a model
      // vertex (and thus the last segment will end with one).

      segStart = result.begin();
      for (SegmentSplitsT::iterator sit = result.begin(); sit != result.end(); )
        {
        bool generateEdge = (storage->pointId(sit->second.high()) ? true : false);
        ++sit;
        // Does the current segment end with a model vertex?
        if (generateEdge)
          { // Generate an edge. segStart->second.low() is guaranteed to be a model vertex.
          smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, sit);
          if (edge.isValid())
            created.push_back(edge);
          segStart = sit;
          }
        }
      // Handle the case when there are no model vertices:
      if (segStart != result.end())
        {
        smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, result.end());
        created.push_back(edge);
        }
      }
    }

  // Remove all non-free vertices from the model and mark the model as modified
  // if any were removed.
  smtk::model::VertexSet freeVerts = parentModel.cellsAs<smtk::model::VertexSet>();
  smtk::model::Edges::iterator crit;
  smtk::model::Vertices dead;
  for (crit = created.begin(); crit != created.end(); ++crit)
    {
    smtk::model::Vertices endpts = crit->vertices();
    for (smtk::model::Vertices::iterator evit = endpts.begin(); evit != endpts.end(); ++evit)
      {
      if (freeVerts.find(*evit) != freeVerts.end())
        {
        dead.push_back(*evit);
        }
      }
    }
  smtk::model::EntityRefArray modified;
  if (!freeVerts.empty())
    {
    parentModel.removeCells(dead);
    modified.push_back(parentModel);
    }

  smtk::model::OperatorResult opResult;
  if (ok)
    {
    opResult = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(opResult, created, CREATED);
    this->addEntitiesToResult(opResult, modified, MODIFIED);
    }
  else
    {
    opResult = this->createResult(smtk::model::OPERATION_FAILED);
    }

  return opResult;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::CreateEdge,
  polygon_create_edge,
  "create edge",
  CreateEdge_xml,
  smtk::bridge::polygon::Session);
