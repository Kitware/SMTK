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
  */
bool vertex::canInsertEdge(const Point& neighborhood, incident_edges::iterator* where)
{
  if (this->m_edges.empty())
    {
    if (where)
      *where = this->m_edges.begin();
    return true;
    }

  pmodel* model = this->parentAs<pmodel>();
  Segment testRay(this->m_coords, neighborhood);
  Point lastRadialPoint(
    model->edgeTestPoint(
      this->m_edges.back().m_edgeId, this->m_edges.back().m_edgeOut));
  int lastRadialOrientation = boost::polygon::orientation(testRay, lastRadialPoint);
  incident_edges::iterator it;
  for (it = this->m_edges.begin(); it != this->m_edges.end(); ++it)
    {
    Point x = model->edgeTestPoint(it->m_edgeId, it->m_edgeOut);
    int sense = boost::polygon::orientation(testRay, x);
    switch (sense)
      {
    case 1: // See if the previous edge brackets it.
      switch (lastRadialOrientation)
        {
      case 1: // Both edges to same side. Keep searching.
        break;
      case -1: // Found enclosing edge pair.
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
        break;
      case 0:
      default:
          {
          smtkErrorMacro(model->session()->log(),
            "Insertion would lead to coincident edges.");
          return false;
          }
        break;
        };
      break;
    case -1: // Keep moving CCW around edges.
      break;
    default:
    case 0: // Oops, this edge is coincident with another model edge. Fail.
        {
        smtkErrorMacro(model->session()->log(),
          "Insertion would lead to coincident edges.");
        return false;
        }
      break;
      }
    lastRadialPoint = x;
    lastRadialOrientation = sense;
    }
  smtkErrorMacro(model->session()->log(),
    "Could not find enclosing edge pair.");
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

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
