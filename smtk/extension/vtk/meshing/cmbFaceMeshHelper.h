//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME cmbFaceMeshHelper
// .SECTION Description
// Convert a vtkModelFace to a triangle input for meshing.

#ifndef __smtk_vtk_cmbFaceMeshHelper_h
#define __smtk_vtk_cmbFaceMeshHelper_h

#include "smtk/common/CompilerInformation.h"
#include "smtk/extension/vtk/meshing/Exports.h" // For export macro
#include "vtkABI.h"
#include "vtkType.h"

class vtkPolyData;
class cmbFaceMesherInterface;

namespace CmbFaceMesherClasses
{
class VTKSMTKMESHINGEXT_EXPORT meshVertex
{
public:
  meshVertex();
  meshVertex(const double& a, const double& b);
  meshVertex(const double& a, const double& b,
      const vtkIdType& ModelId, const int& ModelEntityType);

  double x;
  double y;
  vtkIdType modelId;
  int modelEntityType;

  //equality operator needed for map storage
  bool operator==(const meshVertex &p) const;

  //comparison operator needed for map storage
  bool operator<(const meshVertex &p) const;
};

class VTKSMTKMESHINGEXT_EXPORT meshEdge
{
public:
  meshEdge();
  meshEdge(const vtkIdType& f, const vtkIdType& s);
  meshEdge(const vtkIdType& f, const vtkIdType& s, const vtkIdType& id);

  //equality operator needed for list storage
  bool operator==(const meshEdge &es) const;

  //comparison operator needed for map storage
  bool operator<(const meshEdge &es) const;

  const vtkIdType& first() const {return First;}
  const vtkIdType& second() const {return Second;}

  void setModelId(const vtkIdType& id){ModelId=id;}
  const vtkIdType& modelId() const {return ModelId;}

  //can't set the entityType as it is always an edge
  int modelEntityType() const;
protected:
  vtkIdType First;
  vtkIdType Second;
  vtkIdType ModelId;
};

class ModelLoopRep;

class VTKSMTKMESHINGEXT_EXPORT ModelEdgeRep
{
public:
  friend class ModelLoopRep;

  ModelEdgeRep(const int &id);
  ~ModelEdgeRep();

  void addModelVert(const vtkIdType &id, double point[3]);
  void setMeshPoints(vtkPolyData *mesh, vtkIdType offset=0, vtkIdType size=-1);

  //Note: verts and mesh points need to be added
  //before valid result is returned
  int numberOfEdges() const;

  int numberOfVertices() const;

  const vtkIdType& getId() const;

protected:

  //Updates each locally stored mesh point in the edge mesh with the latest
  //relationship. If the point matches a model vert
  //it will have the id of the model vert and be set to vtkModelVertexType
  //else it will have the id of the model edge and be set to vtkModelEdgeType
  void updateModelRealtionships();

  struct Internals;
  Internals* Internal;
};

class VTKSMTKMESHINGEXT_EXPORT ModelLoopRep
{
public:
  ModelLoopRep();
  ModelLoopRep(const vtkIdType &id, const bool &isInternal);
  ~ModelLoopRep();

  //equality operator needed for list storage
  bool operator==(const ModelLoopRep &lr) const;

  //comparison operator needed for list storage
  bool operator<(const ModelLoopRep &lr) const;

  //returns true if an edge with the sameUniquePersistentId has already be added
  bool edgeExists(const vtkIdType &e) const;

  //Add an edge to the ModelLoopRep.
  //Only unique edges will be added.
  void addEdge(const ModelEdgeRep &edge);

  //returns: If the point is used in the loop.
  //The pointId is the loop based id of the point
  //
  //If the point is a model vertex, the modelEntityType
  //  will be set to vtkModelVertexType, and the uniqueId will be
  //  set to the UniquePersistentId of the model vertex.
  //If the point is a mesh edge rep point, the modelEntityType
  //  will be set to vtkModelEdgeType, and the uniqueId will be
  //  set to the UniquePersistenId of the edge.
  //Otherwise the modelEntityType and uniqueId will not be modified
  //and we will return false
  bool pointClassification(const vtkIdType &pointId,
    int &modelEntityType, vtkIdType &uniqueId) const;

  //returns: If the poin is used in the loop.
  //The pointId is the loop based id of the point
  //
  //If the point is a model vertex, the modelEntityType
  //  will be set to vtkModelVertexType, and the uniqueId will be
  //  set to the UniquePersistentId of the model vertex.
  //If the point is a mesh edge rep point, the modelEntityType
  //  will be set to vtkModelEdgeType, and the uniqueId will be
  //  set to the UniquePersistenId of the edge.
  //Otherwise the modelEntityType and uniqueId will not be modified
  //and we will return false
  bool pointClassification(const double &x, const double &y,
    int &modelEntityType, vtkIdType &uniqueId) const;

  //returns: True if the edge is used in the loop.
  //The pointIds are the loop based ids of the points
  //if the edge is a mesh edge the modelEntityType
  //  will be set to vtkModelEdgeType, and the uniqueId will be
  //  set to the UniquePersistenId of the edge of the edge
  //Otherwise the modelEntityType and uniqueId will not be modified
  //and we will return false
  bool edgeClassification(const vtkIdType &pointId1, const vtkIdType &pointId2,
    int &modelEntityType, vtkIdType &uniqueId) const;

  //returns true if the edge is contained in the loop.
  //The Ids passed in must be between zero and number of Points - 1
  //if the edge is a mesh edge the modelEntityType
  //  will be set to vtkModelEdgeType, and the uniqueId will be
  //  set to the UniquePersistenId of the edge of the edge
  //Otherwise the modelEntityType and uniqueId will not be modified
  //and we will return false
  bool edgeClassification(const double &x1, const double &y1,
    const double &x2, const double &y2,
    int &modelEntityType, vtkIdType &uniqueId) const;

  //returns the number of unique points in this loop
  int numberOfVertices() const;

  //get the number of line segments in the loop
  int numberOfEdges() const;

  bool isOuterLoop() const;
  bool isDegenerateLoop() const;

  //returns NULL if point is not found
  const meshVertex* getPoint(const vtkIdType &meshVertexId) const;

  //returns NULL if point is not found
  const meshVertex* getPoint(const double &x, const double &y) const;

  //returns the local loop id for the mesh vertex.
  //if not found it will return -1
  vtkIdType getMeshVertexId(const double &x, const double &y) const;

  // adds this loops information to the triangle interface
  // modifies the pointIndex, segment Index, and HoleIndex
  void addDataToTriangleInterface(cmbFaceMesherInterface *ti,
     int &pointIndex, int &segmentIndex, int &holeIndex);

  //returns true when we find the closest segment. It should
  //only return false when no edges have been added to the loop
  //The closest segment will be stored in the variable edge
  //The closest point on the segment will be stored in vertex.
  //NOTE: This method only uses the mid point of the segment
  //to find the closest segment.
  const meshEdge* findClosestSegment(const double &x, const double &y,
    meshVertex &vertex) const;

  //returns true if we find a valid point inside the loop
  //that doesn't lay on a boundary. The valid point coordinates
  //will be placed in the x and y variables that are passed in.
  bool findAPointInside(double &x, double &y) const;

  //returns true if the input coordinate is collinear with
  //any of the edges of the loop
  bool isBoundaryPoint(const double& x, const double& y) const;

  //returns true if the input coordinate is inside the loop.
  //This will not check if the point exists on the loops boundary
  //so you should call isBoundaryPoint first
  bool isPointInside(const double& x, const double& y) const;

  //returns the bounds in the order of:
  //xmin,ymin,xmax,ymax
  void bounds(double bounds[4]) const;

protected:
  //copy the information from the edge into the loop
  void addEdgeToLoop(const ModelEdgeRep &edge);

  //Inserts the point if it doesn't exist, and returns
  //the vtkIdType id of the point.
  vtkIdType insertPoint(const meshVertex &point);

  //Find if a the model edge id is to a model edge
  //that isn't part of the valid loop section of the loop.
  //i.e it is a hanging line
  bool isNonManifoldEdge(const vtkIdType &modelEdgeId) const;

  bool findPointInsideConvex(double& x, double &y) const;
  bool findPointInsideConcave(double& x, double &y) const;

  struct Internals;
  Internals* Internal;
};

class VTKSMTKMESHINGEXT_EXPORT ModelFaceRep
{
public:
  ModelFaceRep();
  ~ModelFaceRep();
  void addLoop(const ModelLoopRep &loop);
  int numberOfVertices();
  int numberOfEdges();
  int numberOfHoles();
  //returns the bounds in the order of:
  //xmin,ymin,xmax,ymax
  bool bounds(double bounds[4]);

  void fillTriangleInterface(cmbFaceMesherInterface *ti);

  //Add information from the mesher's input to its output
  bool RelateMeshToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId);

  static const char* Get2DAnalysisPointModelIdsString() {return "2DAnalysisPointModelIds";};
  static const char* Get2DAnalysisPointModelTypesString() {return "2DAnalysisPointModelTypes";};
  static const char* Get2DAnalysisCellModelIdsString() {return "2DAnalysisCellModelIds";};
  static const char* Get2DAnalysisCellModelTypesString() {return "2DAnalysisCellModelTypes";};
  static const char* Get2DAnalysisCellPointIdsString() {return "2DAnalysisCellPointIds";};

protected:
  bool RelateMeshPointsToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId);
  bool RelateMeshCellsToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId);
  bool SetFaceIdOnMesh(vtkPolyData *mesh, const vtkIdType &facePersistenId);

  struct Internals;
  Internals* Internal;
};
}

#endif
