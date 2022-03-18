//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_polygon_internal_Vertex_h
#define smtk_session_polygon_internal_Vertex_h

#include "smtk/SharedFromThis.h"
#include "smtk/session/polygon/internal/Entity.h"

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{

/**\brief A base class for all internal vertex storage.
  *
  * Every vertex stores a pointer to its parent and its UUID.
  * This class uses smtkEnableSharedPtr so that all entities may be
  * managed via one pool of shared pointers.
  */
class SMTKPOLYGONSESSION_EXPORT vertex : public entity
{
public:
  smtkTypeMacro(vertex);
  smtkCreateMacro(vertex);
  smtkSharedFromThisMacro(entity);
  ~vertex() override = default;

  struct incident_edge_data
  {
    Id m_edgeId;       // Should never be nullptr
    Id m_adjacentFace; // Face immediately CW of m_edgeId. May be nullptr.
    // True when edge points outward from vertex (i.e., edge oriented so beginning vertex is this vertex)
    bool m_edgeOut{ false };

    incident_edge_data(Id edgeId, bool isEdgeOut)
      : m_edgeId(edgeId)
      , m_edgeOut(isEdgeOut)
    {
    }
    incident_edge_data(Id faceId)
      : m_adjacentFace(faceId)
    {
    }
    incident_edge_data(Id edgeId, bool isEdgeOut, Id faceId)
      : m_edgeId(edgeId)
      , m_adjacentFace(faceId)
      , m_edgeOut(isEdgeOut)
    {
    }
    incident_edge_data(const incident_edge_data& other)
      : m_edgeId(other.edgeId())
      , m_adjacentFace(other.clockwiseFaceId())
      , m_edgeOut(other.isEdgeOutgoing())
    {
    }
    incident_edge_data() = default;

    Id edgeId() const { return m_edgeId; }
    Id clockwiseFaceId() const { return m_adjacentFace; }
    bool isEdgeOutgoing() const { return m_edgeOut; }
  };
  typedef std::list<incident_edge_data> incident_edges;

  const Point& point() const { return m_coords; }
  Point& point() { return m_coords; }

  bool canInsertEdge(const Point& neighborhood, incident_edges::iterator* where);
  incident_edges::iterator
  insertEdgeAt(incident_edges::iterator where, const Id& edgeId, bool edgeOutwards);
  incident_edges::iterator insertEdgeAt(
    incident_edges::iterator where,
    const Id& edgeId,
    bool edgeOutwards,
    const Id& faceId);
  void removeEdgeAt(incident_edges::iterator where);

  incident_edges::size_type numberOfEdgeIncidences() const { return m_edges.size(); }
  incident_edges::const_iterator edgesBegin() const { return m_edges.begin(); }
  incident_edges::const_iterator edgesEnd() const { return m_edges.end(); }

  incident_edges::iterator edgesBegin() { return m_edges.begin(); }
  incident_edges::iterator edgesEnd() { return m_edges.end(); }

  const incident_edge_data& edgesFront() const { return m_edges.front(); }
  const incident_edge_data& edgesBack() const { return m_edges.back(); }

  incident_edge_data& edgesFront() { return m_edges.front(); }
  incident_edge_data& edgesBack() { return m_edges.back(); }

  incident_edges::const_reverse_iterator edgesRBegin() const { return m_edges.rbegin(); }
  incident_edges::const_reverse_iterator edgesREnd() const { return m_edges.rend(); }

  incident_edges::reverse_iterator edgesRBegin() { return m_edges.rbegin(); }
  incident_edges::reverse_iterator edgesREnd() { return m_edges.rend(); }

  bool setFaceAdjacency(
    const Id& incidentEdge,
    const Id& adjacentFace,
    bool isCCW = true,
    int edgeDir = 0);
  int removeFaceAdjacencies(const Id& face);

  int removeIncidentEdge(const Id& edge);

  void dump();

  /// To be used by SessionIOJSON only (for deserializing from storage):
  void dangerousAppendEdge(const incident_edge_data& rec) { m_edges.insert(m_edges.end(), rec); }

  /// For use only by pmodel::splitEdge
  void setInsideSplit(bool duringSplit) { m_insideSplit = duringSplit; }

protected:
  friend class pmodel;

  vertex() = default;

  Point m_coords;         // position in the plane.
  incident_edges m_edges; // CCW list of incident edges
  bool m_insideSplit{
    false
  }; // Is an edge attached here being split? If so, canInsertEdge will behave differently.

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
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_internal_Vertex_h
