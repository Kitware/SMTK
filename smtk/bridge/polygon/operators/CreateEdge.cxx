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
    sess->findStorage<internal::pmodel>(
      parentModel.entity());
  bool ok = true;
  int numEdges = offsetsItem->numberOfValues();
  int numCoordsPerPt = coordinatesItem->value(0);
  // numPts is the number of points total (across all edges)
  long long numPts =
    (method == 0 ?
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
    if (numSegments < 1 || edgeEnd >= numPts)
      {
      smtkWarningMacro(this->log(),
        "Ignoring input " << ei << " (offset " << edgeOffset << " to " << edgeEnd << ")" <<
        " with not enough points or offset past end of points.");
      continue; // skip "edges" with only 0 or 1 vertices for their entire path.
      }

    // Fill in a list of segments for the edge so we can
    // check for self-intersections.
    std::list<internal::Segment> edgeSegs;
    switch (method)
      {
    case 0: // points, coordinates, offsets
        {
        std::vector<double> pt(numCoordsPerPt, 0.);
        std::vector<internal::Point> ppts;
        internal::Point curr;
        internal::Point prev;
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
            }
          }
        }
      break;
    case 1: // edges, offsets
        {
        internal::Point curr;
        internal::Point prev;
        bool first = true;
        for (; edgeOffset < edgeEnd; ++edgeOffset, prev = curr)
          {
          internal::vertex::Ptr vert =
            sess->findStorage<internal::vertex>(modelItem->value(edgeOffset).entity());
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

    std::list<internal::Segment>::const_iterator edgeit;
    if (ok)
      {
      // DBG
      std::cout << "Edge segments are:\n";
      for (edgeit = edgeSegs.begin(); edgeit != edgeSegs.end(); ++edgeit)
        printSegment(storage, "", *edgeit); // DBG
      // DBG

      SegmentSplitsT result;
      boost::polygon::intersect_segments(result, edgeSegs.begin(), edgeSegs.end());
      std::cout
        << "Intersected " << numSegments
        << " segments of edge " << ei << "."
        << " Result has " << result.size()
        << " segments:\n";
      if (result.empty())
        {
        smtkErrorMacro(this->log(), "Self-intersection of edge segments was empty set.");
        return this->createResult(smtk::model::OPERATION_FAILED);
        }

      // Now we must reorder the intersected results so they match the
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

      edgeit = edgeSegs.begin(); // Keep an iterator pointing to the source segment of the intersected results.
      SegmentSplitsT::iterator segStart = result.begin();
      printSegment(storage, "seg start", segStart->second); // DBG
      SegmentSplitsT::iterator segEnd;
      for (SegmentSplitsT::iterator sit = result.begin(); sit != result.end(); ++edgeit)
        {
        printSegment(storage, "Seg ", sit->second);
        internal::Point deltaSrc =
          internal::Point(
            edgeit->high().x() - edgeit->low().x(),
            edgeit->high().y() - edgeit->low().y());
        internal::Point deltaDst =
          internal::Point(
            sit->second.high().x() - sit->second.low().x(),
            sit->second.high().y() - sit->second.low().y());
        if (deltaDst.x() * deltaSrc.x() < 0 || deltaDst.y() * deltaSrc.y() < 0)
          {
          segStart = sit;
          for (segEnd = sit; segEnd != result.end() && segEnd->first == segStart->first; ++segEnd)
            {
            internal::Segment flipped(segEnd->second.high(), segEnd->second.low());
            segEnd->second = flipped;
            }
          std::reverse(segStart, segEnd);
          sit = segEnd;
          }
        else
          { // Advance sit to end of entries for the segment.
          segStart = sit;
          for (; sit != result.end() && sit->first == segStart->first; ++sit)
            /* do nothing */;
          }
        }
      // xxx
      // TODO:
      //   If first point and last point are not identical
      //   and method == 0 (points), then create model vertices
      //   for first and last vertex.
      //
      //   If result.size() > numSegments, create a model vertex
      //   for each intersection point.
      //
      //   Create an edge from first point to the next model vertex
      //   in the list of segments returned in result.
      //
      //   All of this done by assigning result.begin()->second.low()
      //   to be the "start" vertex.
      //   If the segment's end vertex is equal to "start", then
      //   terminate the edge, assign "start" to be the segment end
      //   vertex, and continue processing.
      //   If the segment's end vertex is an intersection point,
      //   add it as a model vertex, terminate edge, do ... as above.
      //   At the end, terminate the edge. If the end vertex is not
      //   equal to vertex 0 of the edge ** OR ** if the model edge
      //   was split, make both of them model vertices.

      //internal::Point modelEdgeStart = result.begin()->second.low();
      size_t lastOriginalSegment = -1;
      //SegmentSplitsT::iterator segStart = result.begin();
      //printSegment(storage, "seg start", segStart->second); // DBG
      //SegmentSplitsT::iterator segEnd;
      segStart = result.begin();
      for (SegmentSplitsT::iterator sit = result.begin(); sit != result.end(); lastOriginalSegment = (sit++)->first)
        {
        /*
        storage->liftPoint(sit->second.low(), lo.begin());
        storage->liftPoint(sit->second.high(), hi.begin());
        std::cout
          << "  " << sit->first
          << ": (" << lo[0] << ", " << lo[1] << ", " << lo[2]
          << ") -> (" << hi[0] << ", " << hi[1] << ", " << hi[2]
          << ")";
          */
        bool generateModelEdgeBefore = false;
        bool generateModelEdgeAfter = false;
        if (lastOriginalSegment == sit->first)
          { // repeated source segment means high() point must be a model vertex.
          std::cout << " *";
          generateModelEdgeBefore = true;
          }
        // Now edgeit points to the same segment as sit. Note that edgeit will
        // always be a superset of sit. However, sit may point the opposite
        // direction of edgeit (which is **WRONG** IMNSHO). Reverse sit in this
        // case so we don't process pieces of edges backwards.
        if (!!storage->pointId(sit->second.low()) && sit != segStart)
          { // An intermediate point is a model-vertex. We must generate a model edge.
          std::cout << " *";
          generateModelEdgeBefore = true;
          }
        if (!!storage->pointId(sit->second.high()) && sit != segStart)
          { // An intermediate point is a model-vertex. We must generate a model edge.
          std::cout << " +";
          generateModelEdgeAfter = true;
          }
        if (generateModelEdgeBefore) // && (segStart->second.low() != sit->second.low()))
          {
          // Model edges generated in this loop must always have model vertices at each end:
          smtk::model::Vertex vs = storage->findOrAddModelVertex(mgr, segStart->second.low());
          smtk::model::Vertex ve = storage->findOrAddModelVertex(mgr, sit->second.low());
          smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, sit);
          created.push_back(edge);
          /*
          // DBG
          storage->liftPoint(segStart->second.low(), lo.begin());
          storage->liftPoint(sit->second.low(), hi.begin());
          std::cout
            << "\n  Generate " << lo[0] << " " << lo[1] << " " << lo[2]
            << " -> " << hi[0] << " " << hi[1] << " " << hi[2] << "\n";
          // DBG
          */
          segStart = sit;
          }
        segEnd = sit;
        if (generateModelEdgeAfter)
          {
          // Model edges generated in this loop must always have model vertices at each end:
          smtk::model::Vertex vs = storage->findOrAddModelVertex(mgr, segStart->second.low());
          smtk::model::Vertex ve = storage->findOrAddModelVertex(mgr, sit->second.high());
          ++segEnd; // go one past the current segment so it is included in the model edge.
          smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, segEnd);
          created.push_back(edge);
          /*
          // DBG
          storage->liftPoint(segStart->second.low(), lo.begin());
          storage->liftPoint(sit->second.high(), hi.begin());
          std::cout
            << "\n  Generate " << lo[0] << " " << lo[1] << " " << lo[2]
            << " -> " << hi[0] << " " << hi[1] << " " << hi[2] << "\n";
          // DBG
          */
          segStart = segEnd;
          }
        std::cout << "\n";
        }
      // We've reached the end of the user-specified edge.
      // If we have any segments left, generate a model edge.
      if (segStart != result.end())
        { // This edge may be a model-vertex-free loop if the start and end vertices match.
        std::cout << " Generate terminal edge\n";
        smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, result.end());
      /*
        smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, result.back());
      */
        }
      }
    }

  smtk::model::OperatorResult opResult;
  if (ok)
    {
    opResult = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(opResult, created, CREATED);
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
