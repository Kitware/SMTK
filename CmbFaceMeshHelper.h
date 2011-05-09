/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME CmbFaceMeshHelper
// .SECTION Description
// Convert a vtkModelFace to a triangle input for meshing.

#ifndef __CmbFaceMeshHelper_h
#define __CmbFaceMeshHelper_h

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/list> // Needed for STL list.
#include "vtkType.h"

class CmbFaceMesherInterface;
class vtkPolyData;

//-----------------------------------------------------------------------------
namespace CmbModelFaceMeshPrivate
{
class edgePoint
{
public:
  edgePoint(const double& a, const double& b);
  edgePoint(const double& a, const double& b,
      const vtkIdType& ModelId, const int& ModelEntityType);

  double x;
  double y;
  vtkIdType modelId;
  int modelEntityType;

  //comparison operator needed for map storage
  bool operator<(const edgePoint &p) const;
};

class edgeSegment
{
public:
  edgeSegment(const vtkIdType& f, const vtkIdType& s);
  edgeSegment(const vtkIdType& f, const vtkIdType& s, const vtkIdType& id);

  //comparison operator needed for map storage
  bool operator<(const edgeSegment &es) const;

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

class InternalEdge
{
public:
  InternalEdge(const int &id):Id(id){}

  void addModelVert(const vtkIdType &id, double point[3]);
  void setMeshPoints(vtkPolyData *mesh);

  //Note: verts and mesh points need to be added
  //before valid result is returned
  int numberLineSegments() const;

  int numberMeshPoints() const {return (int)MeshPoints.size();}

  const std::set<edgePoint>& getModelVerts() const {return ModelVerts;}
  const std::list<edgeSegment>& getSegments() const {return Segments;}
  const std::map<vtkIdType,edgePoint>& getMeshPoints() const {return MeshPoints;}

  const vtkIdType& getId() const{return Id;}
protected:

  //Updates each point in the edge with the latest
  //relationship. If the point matches a model vert
  //it will have the id of the model vert and be set too vtkModelVertexType
  //else it will have the id of the edge and be set to vtkModelEdgeType
  void updateModelRealtionships();

  const vtkIdType Id;
  std::list<edgeSegment> Segments;
  std::map<vtkIdType,edgePoint> MeshPoints;
  std::set<edgePoint> ModelVerts;
};

class InternalLoop
{
public:
  InternalLoop(const vtkIdType &id, const bool &isInternal)
    :EdgeCount(0),CanBeHole(isInternal),Id(id){}

  bool edgeExists(const vtkIdType &e) const;

  //only adds unique edges
  void addEdge(const InternalEdge &edge);

  //returns true if the point is contained on the loop.
  //The Id passed in must be between zero and number of Points - 1
  //if the point is a model vertice the modelEntityType
  //  will be set to vtkModelVertexType, and the uniqueId will be
  //  set to the UniquePersistentId of the model vert.
  //if the point is a mesh edge point the modelEntityType
  //  will be set to vtkModelEdgeType, and the uniqueId will be
  //  set to the UniquePersistenId of the edge the point is contained on
  // if the point isn't on the loop the modelEntityType and uniqueId
  //  WILL NOT BE MODIFIED
  bool pointModelRelation(const vtkIdType &pointId,
    int &modelEntityType, vtkIdType &uniqueId) const;

  //returns true if the edge is contained in the loop.
  //The Ids passed in must be between zero and number of Points - 1
  //if the edge is a mesh edge the modelEntityType
  //  will be set to vtkModelEdgeType, and the uniqueId will be
  //  set to the UniquePersistenId of the edge of the edge
  // if the edge isn't on the loop the modelEntityType and uniqueId
  //  WILL NOT BE MODIFIED
  bool edgeModelRelation(const vtkIdType &pointId1, const vtkIdType &pointId2,
    int &modelEntityType, vtkIdType &uniqueId) const;

  //mark that we have a duplicate edge
  //this determines if the loop is a hole
  void markEdgeAsDuplicate(const vtkIdType &edgeId);

  //returns the number of  unique points in this loop
  int getNumberOfPoints() const;

  //get the number of line segments in the loop
  int getNumberOfLineSegments() const;

  //returns if this loop is a hole
  bool isHole() const;

  //returns NULL if point is not found
  const edgePoint* getPoint(const vtkIdType &id) const;

  // adds this loops information to the triangle interface
  // modifies the pointIndex, segment Index, and HoleIndex
  void addDataToTriangleInterface(CmbFaceMesherInterface *ti,
     int &pointIndex, int &segmentIndex, int &holeIndex);

  bool pointOnBoundary(const edgePoint &point) const;

  bool pointInside(const edgePoint &point) const;

  //returns the bounds in the order of:
  //xmin,ymin,xmax,ymax
  void bounds(double bounds[4]) const;

protected:
  //copy the information from the edge into the loop
  void addEdgeToLoop(const InternalEdge &edge);

  //Inserts the point if it doesn't exist, and returns
  //the vtkIdType id of the point.
  vtkIdType insertPoint(const edgePoint &point);

  const vtkIdType Id;
  const bool CanBeHole;

  //holds the number of unique edges we have. If greater than zero
  //we have a hole
  int EdgeCount;

  //these store ids, so we don't have duplicates
  std::set<vtkIdType> ModelEdges;

  //Stores all the segments. Stores the segments in a way that
  //is fast to see if an edge already exists. When iterated will not
  //form a sequential list of segments of the loop. This is done this
  //way because our mesher doesn't care about order so that isn't slowed
  //down but this makes mapping the mesh back to the model fast!
  std::set<edgeSegment> Segments;

  //bi directional map implemented as two maps
  //PointsToIds needed for easy lookup on duplicate points
  //IdsToPoints needed for correct indexing from the segments, also needed
  //for fast lookup on points mapping back to model
  std::map<edgePoint,vtkIdType> PointsToIds;
  std::map<vtkIdType,edgePoint> IdsToPoints;
};

class InternalFace
{
  public:
    void addLoop(const InternalLoop &loop);
    int numberOfPoints();
    int numberOfLineSegments();
    int numberOfHoles();

    void fillTriangleInterface(CmbFaceMesherInterface *ti);
    bool RelateMeshToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId);

  protected:
    bool RelateMeshPointsToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId);
    bool RelateMeshCellsToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId);
    std::list<InternalLoop> Loops;
};
}
#endif
