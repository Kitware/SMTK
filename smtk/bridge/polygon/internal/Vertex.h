#ifndef __smtk_bridge_polygon_internal_Vertex_h
#define __smtk_bridge_polygon_internal_Vertex_h

#include "smtk/SharedFromThis.h"
#include "smtk/bridge/polygon/internal/Entity.h"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

/**\brief A base class for all internal vertex storage.
  *
  * Every vertex stores a pointer to its parent and its UUID.
  * This class uses smtkEnableSharedPtr so that all entities may be
  * managed via one pool of shared pointers.
  */
class vertex : public entity
{
public:
  smtkTypeMacro(vertex);
  smtkCreateMacro(vertex);
  smtkSharedFromThisMacro(entity);
  virtual ~vertex() { }

  struct incident_edge_data
    {
    Id m_edgeId; // Should never be NULL
    Id m_adjacentFace; // Face immediately CW of m_edgeId. May be NULL.
    bool m_edgeOut; // True when edge points outward from vertex (i.e., edge oriented so beginning vertex is this vertex)
    };
  typedef std::list<incident_edge_data> incident_edges;

  const Point& point() const { return this->m_coords; }
  Point point() { return this->m_coords; }

  bool canInsertEdge(const Point& neighborhood, incident_edges::iterator* where);
  void insertEdgeAt(incident_edges::iterator where, const Id& edgeId, bool edgeOutwards);

  incident_edges::const_iterator beginEdges() const { return this->m_edges.begin(); }
  incident_edges::const_iterator endEdges() const { return this->m_edges.end(); }

  incident_edges::iterator beginEdges() { return this->m_edges.begin(); }
  incident_edges::iterator endEdges() { return this->m_edges.end(); }

protected:
  friend class pmodel;

  vertex() { }

  Point m_coords; // position in the plane.
  incident_edges m_edges; // CCW list of incident edges

  // NB: One extension to this structure would be:
  // Ptr m_prev; // Previous model vertex located at this exact point in the plane
  // Ptr m_next; // Next model vertex located at this exact point in the plane
  // These would allow modeling polygons that were "topologically independent"
  // while still allowing quick lookup of vertices by point location.

  // NB: Another extension to this structure would be:
  // Id m_containingFace; // Face that contains this model vertex as a hard point in its interior.
};

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_polygon_internal_Vertex_h
