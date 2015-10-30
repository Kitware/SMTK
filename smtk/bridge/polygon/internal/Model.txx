#ifndef __smtk_bridge_polygon_internal_model_txx
#define __smtk_bridge_polygon_internal_model_txx

#include "smtk/model/Edge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Vertex.h"
#include "smtk/bridge/polygon/internal/Edge.h"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

/**\brief Create a model edge from ordered segments with preconditions.
  *
  * + Segments must be ordered head-to-tail.
  * + Only the initial and final points may have model vertices associated
  *   with their locations.
  * + If the initial and final points are not identical, then model vertices
  *   *must* be associated with their locations and those vertices must not
  *   have faces that overlap the edge. (This is verified before creation
  *   but only the immediate neighborhood of the endpoints is checked by
  *   examining face adjacencies at the location where the new edge would
  *   be inserted into the vertex connectivity.)
  *
  * If these preconditions do not hold, either an invalid (empty) edge will be
  * returned or the model will become inconsistent.
  */
template<typename T>
model::Edge pmodel::createModelEdgeFromSegments(model::ManagerPtr mgr, T begin, T end)
{
  if (!mgr || begin == end)
    return smtk::model::Edge();

  Id vInit = this->pointId(begin->second.low());
  Id vFini = this->pointId((begin + (end - begin - 1))->second.high());

  vertex::Ptr vInitStorage = this->m_session->findStorage<vertex>(vInit);
  vertex::Ptr vFiniStorage = this->m_session->findStorage<vertex>(vFini);

  if (!vInitStorage ^ !vFiniStorage)
    { // one but not both of the points are model vertices. Error.
    smtkErrorMacro(this->session()->log(),
      "Zero or both endpoints must be model vertices to create edge.");
    return smtk::model::Edge();
    }

  internal::vertex::incident_edges::iterator whereBegin;
  internal::vertex::incident_edges::iterator whereEnd;
  if (vInitStorage)
    { // Ensure edge can be inserted without splitting a face.
    if (!vInitStorage->canInsertEdge(begin->second.high(), &whereBegin))
      {
      smtkErrorMacro(this->m_session->log(),
        "Edge would overlap face in neighborhood of first vertex");
      return smtk::model::Edge();
      }
    }

  if (vFiniStorage)
    { // Ensure edge can be inserted without splitting a face.
    if (!vFiniStorage->canInsertEdge((begin + (end - begin - 1))->second.low(), &whereEnd))
      {
      smtkErrorMacro(this->m_session->log(),
        "Edge would overlap face in neighborhood of last vertex");
      return smtk::model::Edge();
      }
    }

  // We can safely create the edge now
  smtk::model::Edge created = mgr->addEdge();
  internal::edge::Ptr storage = internal::edge::create();
  storage->setParent(this);
  storage->setId(created.entity());
  this->m_session->addStorage(created.entity(), storage);
  storage->m_points.clear();
  storage->m_points.insert(storage->m_points.end(), begin->second.low());
  for (T segIt = begin; segIt != end; ++segIt)
    storage->m_points.insert(storage->m_points.end(), segIt->second.high());

  smtk::model::Model parentModel(mgr, this->id());
  // Insert edge at proper place in model vertex edge-lists
  if (vInitStorage)
    {
    vInitStorage->insertEdgeAt(whereBegin, created.entity(), /* edge is outwards: */ true);
    smtk::model::Vertex vert(mgr, vInit);
    if (parentModel.isEmbedded(vert))
      parentModel.unembedEntity(vert);
    created.findOrAddRawRelation(vert);
    }
  if (vFiniStorage)
    {
    vFiniStorage->insertEdgeAt(whereEnd, created.entity(), /* edge is outwards: */ false);
    smtk::model::Vertex vert(mgr, vFini);
    if (parentModel.isEmbedded(vert))
      parentModel.unembedEntity(vert);
    created.findOrAddRawRelation(vert);
    }
  // Add tesselation to created edge using storage to lift point coordinates:
  this->addEdgeTessellation(created, storage);

  parentModel.embedEntity(created);
  created.assignDefaultName(); // Do not move above parentModel.embedEntity() or name will suck.

  return created;
}

template<typename T>
Point pmodel::projectPoint(T coordBegin, T coordEnd)
{
  double xyz[3] = {0, 0, 0};
  int i = 0;
  // Translate to origin
  for (T c = coordBegin; c != coordEnd && i < 3; ++i, ++c)
    {
    xyz[i] = *c - this->m_origin[i];
    }
  // Assume any unspecified coordinates are 0 and finish translating to origin
  for (; i < 3; ++i)
    {
    xyz[i] = - this->m_origin[i];
    }
  // Project translated point to x and y axes
  double px = 0, py = 0;
  for (i = 0; i < 3; ++i)
    {
    px += xyz[i] * this->m_xAxis[i];
    py += xyz[i] * this->m_yAxis[i];
    }
  // Scale point and round to integer
  Point result(px * this->m_scale, py * this->m_scale);
  return result;
}

template<typename T>
void pmodel::liftPoint(const Point& ix, T coordBegin)
{
  T coord = coordBegin;
  for (int i = 0; i < 3; ++i, ++coord)
    {
    *coord =
      this->m_origin[i] +
      ix.x() * this->m_iAxis[i] +
      ix.y() * this->m_jAxis[i];
    }
}

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
#endif // __smtk_bridge_polygon_internal_model_txx
