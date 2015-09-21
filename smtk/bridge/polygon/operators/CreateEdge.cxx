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
        // FIXME
        }
      break;
    default:
      ok = false;
      smtkInfoMacro(log(), "Unhandled construction method " << method << ".");
      break;
      }

    // DBG
    std::cout << "Edge segments are:\n";
    std::list<internal::Segment>::const_iterator edgeit;
    for (edgeit = edgeSegs.begin(); edgeit != edgeSegs.end(); ++edgeit)
      {
      std::vector<double> lo(3);
      std::vector<double> hi(3);
      storage->liftPoint(edgeit->low(), lo.begin());
      storage->liftPoint(edgeit->high(), hi.begin());
      std::cout
        << "  (" << lo[0] << ", " << lo[1] << ", " << lo[2]
        << ") -> (" << hi[0] << ", " << hi[1] << ", " << hi[2]
        << ")\n";
      }
    // DBG

    SegmentSplitsT result;
    boost::polygon::intersect_segments(result, edgeSegs.begin(), edgeSegs.end());
    std::cout
      << "Intersected " << numSegments
      << " segments of edge " << ei << "."
      << " Result has " << result.size()
      << " segments:\n";
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
    if (result.empty())
      {
      smtkErrorMacro(this->log(), "Self-intersection of edge segments was empty set.");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    internal::Point modelEdgeStart = result.begin()->second.low();
    size_t lastOriginalSegment = -1;
    SegmentSplitsT::const_iterator segStart = result.begin();
    for (SegmentSplitsT::const_iterator sit = result.begin(); sit != result.end(); lastOriginalSegment = (sit++)->first)
      {
      std::vector<double> lo(3);
      std::vector<double> hi(3);
      storage->liftPoint(sit->second.low(), lo.begin());
      storage->liftPoint(sit->second.high(), hi.begin());
      std::cout
        << "  " << sit->first
        << ": (" << lo[0] << ", " << lo[1] << ", " << lo[2]
        << ") -> (" << hi[0] << ", " << hi[1] << ", " << hi[2]
        << ")";
      bool generateModelEdge = false;
      if (lastOriginalSegment == sit->first)
        { // repeated source segment means high() point must be a model vertex.
        std::cout << " *";
        smtk::model::Vertex v = storage->findOrAddModelVertex(mgr, sit->second.high());
        generateModelEdge = true;
        }
      else if (!!storage->pointId(sit->second.high()))
        {
        std::cout << " +";
        generateModelEdge = true;
        }
      if (generateModelEdge)
        {
        SegmentSplitsT::const_iterator segEnd = sit;
        ++segEnd; // go one past the current segment so it is included in the model edge.
        smtk::model::Edge edge = storage->createModelEdgeFromSegments(mgr, segStart, segEnd);
        segStart = segEnd;
        }
      std::cout << "\n";
      }
    }

  smtk::model::OperatorResult result;
  /*
  if (ok)
    {
    smtk::bridge::polygon::Session* sess = this->polygonSession();
    smtk::model::Manager::Ptr mgr;
    if (sess)
      {
      mgr = sess->manager();
      smtk::model::Model model = mgr->addModel(/ * par. dim. * / 2, / * emb. dim. * / 3, "model");
      storage->setId(model.entity());
      result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
      this->addEntityToResult(result, model, CREATED);
      model.setFloatProperty("x axis", smtk::model::FloatList(storage->xAxis(), storage->xAxis() + 3));
      model.setFloatProperty("y axis", smtk::model::FloatList(storage->yAxis(), storage->yAxis() + 3));
      model.setFloatProperty("normal", smtk::model::FloatList(storage->zAxis(), storage->zAxis() + 3));
      model.setFloatProperty("origin", smtk::model::FloatList(storage->origin(), storage->origin() + 3));
      model.setFloatProperty("feature size", storage->featureSize());
      model.setIntegerProperty("model scale", storage->modelScale());
      }
    }
  */
  if (!result)
    {
    result = this->createResult(smtk::model::OPERATION_FAILED);
    }

  return result;
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
