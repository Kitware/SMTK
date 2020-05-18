//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_model_h
#define __smtk_session_polygon_internal_model_h

#include "smtk/session/polygon/Exports.h"

#include "smtk/session/polygon/internal/Entity.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Vertex.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <array>

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{

class SMTKPOLYGONSESSION_EXPORT pmodel : public entity
{
public:
  smtkTypeMacro(pmodel);
  smtkCreateMacro(pmodel);
  smtkSharedFromThisMacro(entity);
  pmodel();
  ~pmodel();

  bool computeModelScaleAndNormal(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& y_axis,
    double featureSize,
    smtk::io::Logger& log);

  bool computeModelScaleAndYAxis(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& z_axis,
    double featureSize,
    smtk::io::Logger& log);

  bool computeFeatureSizeAndNormal(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& y_axis,
    long long modelScale,
    smtk::io::Logger& log);

  bool restoreModel(
    std::vector<double>& origin,
    std::vector<double>& x_axis,
    std::vector<double>& y_axis,
    std::vector<double>& z_axis,
    std::vector<double>& i_axis,
    std::vector<double>& j_axis,
    double featureSize,
    long long modelScale);

  smtk::model::Vertices findOrAddModelVertices(
    smtk::model::ResourcePtr resource,
    const std::vector<double>& points,
    int numCoordsPerPt);

  smtk::model::Vertex
  findOrAddModelVertex(smtk::model::ResourcePtr resource, const Point& pt, bool addToModel = true);

  smtk::model::Vertex
  addModelVertex(smtk::model::ResourcePtr resource, const Point& pt, bool addToModel = true);

  bool demoteModelVertex(
    smtk::model::ResourcePtr resource,
    internal::VertexPtr vert,
    smtk::model::EntityRefs& created,
    smtk::model::EntityRefs& modified,
    smtk::model::EntityRefs& expunged,
    int debugLevel = 0);

  model::Edge createModelEdgeFromVertices(
    smtk::model::ResourcePtr resource,
    internal::VertexPtr v0,
    internal::VertexPtr v1);

  template<typename T, typename U>
  model::Edge createModelEdgeFromSegments(
    smtk::model::ResourcePtr resource,
    T begin,
    T end,
    bool addToModel,
    const U& splitEdgeFaces,
    bool headIsNewVertex,
    smtk::model::VertexSet& newVerts);

  template<typename T>
  model::Edge
  createModelEdgeFromPoints(smtk::model::ResourcePtr resource, T begin, T end, bool isFreeCell);

  template<typename T>
  std::set<Id> createModelEdgesFromPoints(T begin, T end);

  bool splitModelEdgeAtPoint(
    smtk::model::ResourcePtr resource,
    const Id& edgeId,
    const std::vector<double>& point,
    smtk::model::EntityRefArray& created,
    int debugLevel = 0);
  bool splitModelEdgeAtIndex(
    smtk::model::ResourcePtr resource,
    const Id& edgeId,
    int splitPointIndex,
    smtk::model::EntityRefArray& created,
    int debugLevel = 0);
  bool splitModelEdgeAtModelVertex(
    smtk::model::ResourcePtr resource,
    const Id& edgeId,
    const Id& vertexId,
    smtk::model::EntityRefArray& created,
    int debugLevel = 0);
  bool splitModelEdgeAtModelVertex(
    smtk::model::ResourcePtr resource,
    EdgePtr edgeToSplit,
    VertexPtr splitPoint,
    PointSeq::const_iterator location,
    smtk::model::EntityRefArray& created,
    int debugLevel = 0);
  bool splitModelEdgeAtModelVertices(
    smtk::model::ResourcePtr resource,
    EdgePtr edgeToSplit,
    std::vector<VertexPtr>& splitPoints,
    std::vector<PointSeq::const_iterator>& locations,
    smtk::model::EntityRefArray& created,
    int debugLevel = 0);

  std::pair<Id, Id> removeModelEdgeFromEndpoints(smtk::model::ResourcePtr resource, EdgePtr edg);
  bool removeVertexLookup(const internal::Point& location, const Id& vid);

  Point edgeTestPoint(const Id& edgeId, bool edgeEndPt) const;

  void pointsInLoopOrder(std::vector<Point>& pts, const smtk::model::Loop& loop);

  void addFaceTessellation(smtk::model::Face& faceRec);
  void addEdgeTessellation(smtk::model::Edge& edgeRec, internal::EdgePtr edgeData);
  void addVertTessellation(smtk::model::Vertex& vertRec, internal::VertexPtr vertData);

  void addFaceMeshTessellation(smtk::model::Face& faceRec);
  void addEdgeMeshTessellation(smtk::model::Edge& edgeRec, internal::EdgePtr edgeData);
  void addVertMeshTessellation(smtk::model::Vertex& vertRec, internal::VertexPtr vertData);

  double* origin() { return &(m_origin[0]); }
  const double* origin() const { return &(m_origin[0]); }

  double* xAxis() { return &(m_xAxis[0]); }
  const double* xAxis() const { return &(m_xAxis[0]); }

  double* yAxis() { return &(m_yAxis[0]); }
  const double* yAxis() const { return &(m_yAxis[0]); }

  double* zAxis() { return &(m_zAxis[0]); }
  const double* zAxis() const { return &(m_zAxis[0]); }

  double* iAxis() { return &(m_iAxis[0]); }
  const double* iAxis() const { return &(m_iAxis[0]); }

  double* jAxis() { return &(m_jAxis[0]); }
  const double* jAxis() const { return &(m_jAxis[0]); }

  double featureSize() const { return m_featureSize; }
  double modelScale() const { return static_cast<double>(m_scale); }

  //ConstSessionPtr session() const { return m_session; }
  SessionPtr session() { return m_session; }
  void setSession(SessionPtr s) { m_session = s; }

  Id pointId(const Point& p) const;

  /**\brief A convenience method to get the model vertex at \a p.
    *
    * This method will never create a model vertex; if one is not
    * present at \a p, an invalid model::Vertex will be returned.
    */
  smtk::model::Vertex vertexAtPoint(smtk::model::ResourcePtr resource, const Point& p) const
  {
    return smtk::model::Vertex(resource, this->pointId(p));
  }

  template<typename T>
  Point projectPoint(T coordBegin, T coordEnd);

  template<typename T>
  void liftPoint(const Point& ix, T coordBegin);

  template<typename T>
  bool tweakEdge(
    smtk::model::Edge edge,
    int numCoordsPerPt,
    T coordBegin,
    T coordEnd,
    smtk::model::EntityRefArray& modified);
  bool tweakEdge(
    smtk::model::Edge edge,
    internal::PointSeq& replacement,
    smtk::model::EntityRefArray& modified);

  bool tweakVertex(
    smtk::model::Vertex vertRec,
    const Point& vertPosn,
    smtk::model::EntityRefs& modifiedEdgesAndFaces);

  void addVertexIndex(VertexPtr vert);

protected:
  SessionPtr m_session; // Parent session of this pmodel.
  long long
    m_scale; // Recommend this be a large composite number w/ factors 2, 3, 5 (e.g., 15360, 231000, or 1182720)
  double m_featureSize;

  double m_origin[3]; // Base point of plane for pmodel

  double m_xAxis[3]; // Unit length vector in world space (m_scale model-integers long)
  double m_yAxis[3]; // In-plane vector orthogonal to m_xAxis (also with unit length).
  double m_zAxis[3]; // Normal vector orthogonal to m_xAxis and m_yAxis with the same length.

  double
    m_iAxis[3]; // Vector whose length should be equal to one integer "unit" (e.g., 1 integer long)
  double m_jAxis[3]; // In-plane vector orthogonal to m_xAxis with the same length.

  PointToVertexId m_vertices;
  //pointsToEdgeIdT m_edges;
};

} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk

#endif // __smtk_session_polygon_internal_model_h
