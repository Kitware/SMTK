#include "smtk/bridge/polygon/internal/Vertex.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/Session.h"

#include "smtk/io/Logger.h"

#include "boost/polygon/polygon.hpp"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

/**\brief Return true if the edge can be inserted.
  *
  * FIXME: This will have precision issues with small angles because
  *        sqrt() is not calculated with intervals and overflow may
  *        occur when edge neighbor-points are far from each other.
  */
bool vertex::canInsertEdge(const Point& neighborhood, incident_edges::iterator* where)
{
  // Early termination... 0 or 1 existing vertices are always in CCW order no
  // matter where we insert
  if (this->m_edges.size() < 2)
    {
    // A vertex with 1 incident edge that is part of a face has face completely
    // surrounding the vertex; it will never be valid to insert another edge
    // there without removing the face first.
    if (!this->m_edges.empty() && this->m_edges.front().m_adjacentFace)
      return false;
    // Otherwise, it is always valid to insert a new edge anywhere.
    if (where)
      *where = this->m_edges.begin();
    return true;
    }

  pmodel* model = this->parentAs<pmodel>();
  Point pt(
    neighborhood.x() - this->m_coords.x(),
    neighborhood.y() - this->m_coords.y()
  );

  Point prevPt(
    model->edgeTestPoint(
      this->m_edges.back().m_edgeId, !this->m_edges.back().m_edgeOut));
  Point pa(
    prevPt.x() - this->m_coords.x(),
    prevPt.y() - this->m_coords.y()
  );
  incident_edges::iterator it;
  for (it = this->m_edges.begin(); it != this->m_edges.end(); ++it)
    {
    Point currPt = model->edgeTestPoint(it->m_edgeId, !it->m_edgeOut);
    Point pb(
      currPt.x() - this->m_coords.x(),
      currPt.y() - this->m_coords.y()
    );

    bool inside;
    Coord axb = pa.x() * pb.y() - pb.x() * pa.y();
    if (axb < 0)
      { // CCW angle between pa and pb is < 180 degrees
      Coord axt = pa.x() * pt.y() - pt.x() * pa.y();
      if (axt < 0)
        inside = false; // vectors pa->pb don't bracket pt CCW
      long double mb = sqrt(pb.x() * pb.x() + pb.y() * pb.y());
      long double mt = sqrt(pt.x() * pt.x() + pt.y() * pt.y());
      if (axt * mb < axb * mt)
        { // pa->pb brackets pt CCW.
        inside = true;
        }
      }
    else
      { // CCW angle between pa and pb is >= 180 degrees
      Coord bxt = pb.x() * pt.y() - pt.x() * pb.y();
      inside = (bxt < 0);
      if (!inside)
        {
        Coord bxa = -axb;
        long double ma = sqrt(pa.x() * pa.x() + pa.y() * pa.y());
        long double mt = sqrt(pt.x() * pt.x() + pt.y() * pt.y());
        inside = (bxt * ma > bxa * mt);
        }
      }
    if (inside)
      {
      if (!it->m_adjacentFace)
        { // There is no face; it's OK to add the edge here.
        if (where)
          *where = it;
        return true;
        }
      else
        {
        smtkErrorMacro(model->session()->log(),
          "Edge would split face " << it->m_adjacentFace);
        return false;
        }
      }
    pa = pb;
    }
  smtkErrorMacro(model->session()->log(), "Collinear edges");
  return false;
}

/**\brief Insert the edge \a where told (by canInsertEdge).
  *
  * \a edgeOutwards indicates whether the forward-direction edge
  * is outward or inward-pointing (from/to this vertex).
  */
void vertex::insertEdgeAt(incident_edges::iterator where, const Id& edgeId, bool edgeOutwards)
{
  incident_edge_data edgeData;
  edgeData.m_edgeId = edgeId;
  edgeData.m_edgeOut = edgeOutwards;
  // NB: Should never insert edge where a face exists, so edgeData.m_adjacentFace should be left NULL.
  this->m_edges.insert(where, edgeData);
}

/**\brief Remove the edge incidence record at the given position.
  *
  * This does not perform any updates to other incidence records.
  */
void vertex::removeEdgeAt(incident_edges::iterator where)
{
  this->m_edges.erase(where);
}

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
