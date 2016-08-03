//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
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

    incident_edge_data(Id edgeId, bool isEdgeOut)
      : m_edgeId(edgeId), m_edgeOut(isEdgeOut) { }
    incident_edge_data(Id faceId)
      : m_adjacentFace(faceId) { }
    incident_edge_data(Id edgeId, bool isEdgeOut, Id faceId)
      : m_edgeId(edgeId), m_edgeOut(isEdgeOut), m_adjacentFace(faceId) { }
    incident_edge_data(const incident_edge_data& other)
      : m_edgeId(other.edgeId()), m_edgeOut(other.isEdgeOutgoing()), m_adjacentFace(other.clockwiseFaceId()) { }
    incident_edge_data()
      : m_edgeOut(false) { }

    Id edgeId() const { return this->m_edgeId; }
    Id clockwiseFaceId() const { return this->m_adjacentFace; }
    bool isEdgeOutgoing() const { return this->m_edgeOut; }
    };
  typedef std::list<incident_edge_data> incident_edges;

  const Point& point() const { return this->m_coords; }
  Point point() { return this->m_coords; }

  bool canInsertEdge(const Point& neighborhood, incident_edges::iterator* where);
  void insertEdgeAt(incident_edges::iterator where, const Id& edgeId, bool edgeOutwards);
  void removeEdgeAt(incident_edges::iterator where);

  incident_edges::const_iterator edgesBegin() const { return this->m_edges.begin(); }
  incident_edges::const_iterator edgesEnd() const { return this->m_edges.end(); }

  incident_edges::iterator edgesBegin() { return this->m_edges.begin(); }
  incident_edges::iterator edgesEnd() { return this->m_edges.end(); }

  incident_edges::const_reverse_iterator edgesRBegin() const { return this->m_edges.rbegin(); }
  incident_edges::const_reverse_iterator edgesREnd() const { return this->m_edges.rend(); }

  incident_edges::reverse_iterator edgesRBegin() { return this->m_edges.rbegin(); }
  incident_edges::reverse_iterator edgesREnd() { return this->m_edges.rend(); }

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
