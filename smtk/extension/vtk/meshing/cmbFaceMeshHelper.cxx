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
#include "cmbFaceMeshHelper.h"

#include "smtk/extension/vtk/meshing/cmbFaceMesherInterface.h"
#include "smtk/extension/vtk/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "vtkArrayIteratorIncludes.h" //needed for VTK_TT
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSetGet.h" //needed for VTK_TT
#include <algorithm> //Needed for std::max and std::min
#include <limits> //Needed for int max
#include <list> // Needed for STL list.
#include <map> // Needed for STL map.
#include <set> // Needed for STL set.

//#include "vtkModel.h"
//---FIXME---
// The following two enums are replicated from vtkModel.h
// in the ConceptualModel plugin. This is because this file is
// getting moved from SimBuilderMesh to VTKExtensions. This and
// the enums in vtkModel.h should eventually get consolidated into
// one common file. The  is added as a suffix to these enums
// so the names don't conflict
//

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#  if !defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#    pragma warning ( disable : 4996 ) /* disable security warning */
#  endif //!defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#endif //Windows specific stuff

enum ModelEntityTypesCOPY
{
  vtkModelTypeCOPY = 0,
  vtkModelVertexTypeCOPY,
  vtkModelVertexUseTypeCOPY,
  vtkModelEdgeTypeCOPY,
  vtkModelEdgeUseTypeCOPY,
  vtkModelLoopUseTypeCOPY,
  vtkModelFaceTypeCOPY,
  vtkModelFaceUseTypeCOPY,
  vtkModelShellUseTypeCOPY,
  vtkModelRegionTypeCOPY,
};

// Description:
// All the currently defined Model events are listed here.
enum ModelEventIdsCOPY {
  ModelGeometricEntityCreatedCOPY = 11000,
  ModelGeometricEntityBoundaryModifiedCOPY,
  ModelGeometricEntityAboutToDestroyCOPY,
  ModelGeometricEntitiesAboutToMergeCOPY,
  ModelGeometricEntitySplitCOPY // this event is called after the entity is split
};
//---END FIXME---

//for data dumping in debug
//#define DUMP_DEBUG_DATA
#ifdef DUMP_DEBUG_DATA
#define DUMP_DEBUG_DATA_DIR "E:/Work/"
#include "vtkNew.h"
#include "vtkXMLPolyDataWriter.h"
#include <sstream>
#endif

namespace
{
  inline bool arePointsCollinear(const double &x1, const double &y1,
                                 const double &x2, const double &y2,
                                 const double &x3, const double &y3)
    {
    double area = (x1 * ( y2 - y3 )  ) +
           (x2 * ( y3 - y1 )  ) +
           (x3 * ( y1 - y2 )  );
    return (fabs(area) <= 1e-9);
    }

  inline void centerOfBounds(double bounds[4], double center[2])
    {
    center[0] = (bounds[2] - bounds[0]);
    if ( center[0] > 0 )
      {
      center[0] = bounds[0] + (center[0]/2);
      }
    center[1] = (bounds[3] - bounds[1]);
    if ( center[1] > 0 )
      {
      center[1] = bounds[1] + (center[1]/2);
      }
    }
}

using namespace CmbFaceMesherClasses;

//----------------------------------------------------------------------------
meshVertex::meshVertex():
x(0),
y(0),
modelId(-1),
modelEntityType(vtkModelTypeCOPY)
{}

//----------------------------------------------------------------------------
meshVertex::meshVertex(const double& a, const double& b):
x(a),
y(b),
modelId(-1),
modelEntityType(vtkModelTypeCOPY)
{}

//----------------------------------------------------------------------------
meshVertex::meshVertex(const double& a, const double& b,
      const vtkIdType& id, const int& type):
x(a),
y(b),
modelId(id),
modelEntityType(type)
{}

//----------------------------------------------------------------------------
bool meshVertex::operator ==(const meshVertex &p) const
{
  return  ((this->x == p.x) &&
          (this->x == p.x && this->y && p.y));
}

//----------------------------------------------------------------------------
bool meshVertex::operator <(const meshVertex &p) const
{
  return  ((this->x < p.x) ||
          (this->x == p.x && this->y < p.y));
}

//----------------------------------------------------------------------------
meshEdge::meshEdge():
First(-1),
Second(-1),
ModelId(-1)
{}

//----------------------------------------------------------------------------
meshEdge::meshEdge(const vtkIdType& f, const vtkIdType& s):
First(f),
Second(s),
ModelId(-1)
{}

//----------------------------------------------------------------------------
meshEdge::meshEdge(const vtkIdType& f, const vtkIdType& s,
                         const vtkIdType& id):
First(f),
Second(s),
ModelId(id)
{}

//----------------------------------------------------------------------------
bool meshEdge::operator==(const meshEdge &p) const
{
  return ((this->First == p.First) &&
         (this->First == p.First && this->Second == p.Second));
}

//----------------------------------------------------------------------------
bool meshEdge::operator<(const meshEdge &p) const
{
  return ((this->First < p.First) ||
         (this->First == p.First && this->Second < p.Second));
}

//----------------------------------------------------------------------------
int meshEdge::modelEntityType() const
{
  return vtkModelEdgeTypeCOPY;
}
//----------------------------------------------------------------------------
struct ModelEdgeRep::Internals
{
  vtkIdType Id;
  std::list<meshEdge> Segments;
  std::map<vtkIdType,meshVertex> MeshPoints;
  std::set<meshVertex> ModelVerts;
};

//----------------------------------------------------------------------------
ModelEdgeRep::ModelEdgeRep(const int &id)
{
  this->Internal = new Internals();
  this->Internal->Id = id;
}
//----------------------------------------------------------------------------
ModelEdgeRep::~ModelEdgeRep()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
int ModelEdgeRep::numberOfEdges() const
{
  return static_cast<int>(this->Internal->Segments.size());
}

//----------------------------------------------------------------------------
int ModelEdgeRep::numberOfVertices() const
{
  return static_cast<int>(this->Internal->MeshPoints.size());
}

//----------------------------------------------------------------------------
const vtkIdType& ModelEdgeRep::getId() const
{
  return this->Internal->Id;
}

//----------------------------------------------------------------------------
void ModelEdgeRep::addModelVert(const vtkIdType &id, double point[3])
{
  meshVertex modelVert(point[0],point[1],id,vtkModelVertexTypeCOPY);
  this->Internal->ModelVerts.insert(modelVert);
  this->updateModelRealtionships();
}

//----------------------------------------------------------------------------
void ModelEdgeRep::setMeshPoints(vtkPolyData *mesh, vtkIdType offset/*=0*/, vtkIdType size/*=-1*/)
{

  this->Internal->MeshPoints.clear();
  vtkCellArray *lines = mesh->GetLines();
  double p[3];
  if (lines)
    {
    vtkIdType *pts,npts;
    //
    for (lines->SetTraversalLocation(offset);lines->GetNextCell(npts,pts);)
      {
      for (vtkIdType j=0; j < npts; ++j)
        {
        if ( this->Internal->MeshPoints.find(pts[j]) == this->Internal->MeshPoints.end())
          {
          mesh->GetPoint(pts[j],p);
          //by default everything is related to the edge, updateModelRealtionship
          //will correct the edges to be related to the model vert when verts are added
          meshVertex ep(p[0],p[1], this->Internal->Id,vtkModelEdgeTypeCOPY);
          this->Internal->MeshPoints.insert(std::pair<vtkIdType,meshVertex>(pts[j],ep));
          }
        if ( j > 0 )
          {
          meshEdge es(pts[j-1],pts[j], this->Internal->Id);
          this->Internal->Segments.push_back(es);
          }
        }
      //If size is specified only add lines in that range
      if (size != -1 && size <= lines->GetTraversalLocation() - offset)
        {
        break;
        }
      }
    }

  this->updateModelRealtionships();
}

//Updates each point in the edge with the latest
//relationship. If the point matches a model vert
//it will have the id of the model vert and be set too vtkModelVertexType
//else it will have the id of the edge and be set to vtkModelEdgeType
//----------------------------------------------------------------------------
void ModelEdgeRep::updateModelRealtionships()
{
  //not a fast algorithm. brue force compare
  std::map<vtkIdType,meshVertex>::iterator mpIt;
  std::set<meshVertex>::const_iterator mvIt;
  for ( mpIt = this->Internal->MeshPoints.begin(); mpIt != this->Internal->MeshPoints.end(); mpIt++)
    {
    mvIt = this->Internal->ModelVerts.find( mpIt->second );
    if ( mvIt != this->Internal->ModelVerts.end() )
      {
      //we found a model vert, update the point with the model vertex entity type
      //and id.
      mpIt->second.modelEntityType = mvIt->modelEntityType;
      mpIt->second.modelId = mvIt->modelId;
      }
    }

}

//----------------------------------------------------------------------------
struct ModelLoopRep::Internals
{
  vtkIdType Id;

  //stores if this loop represents the outer boundary of a face
  bool IsOuterLoop;

  //these store ids, so we don't have duplicates. We use the count
  //to determine if this edge is a valid edge to use in raycasting
  //when determing point inside. Note: the reason for this is that
  //edges that are used twice contain no volume so they can't be used
  //when ray tracing
  std::map<vtkIdType, int> ModelEdges;

  // The following is for classification of mesh entities
  // with respects to model edges and vertices

  //Stores all the segments. Stores the segments in a way that
  //is fast to see if an edge already exists. When iterated will not
  //form a sequential list of segments of the loop. This is done this
  //way because our mesher doesn't care about order so that isn't slowed
  //down but this makes mapping the mesh back to the model fast!
  std::set<meshEdge> Segments;

  //bi directional map implemented as two maps
  //PointsToIds needed for easy lookup on duplicate points
  //IdsToPoints needed for correct indexing from the segments, also needed
  //for fast lookup on points mapping back to model
  std::map<meshVertex,vtkIdType> PointsToIds;
  std::map<vtkIdType,meshVertex> IdsToPoints;
};

//----------------------------------------------------------------------------
ModelLoopRep::ModelLoopRep()
{
  this->Internal = new Internals();
  this->Internal->Id = -1;
  this->Internal->IsOuterLoop = true;
}

//----------------------------------------------------------------------------
ModelLoopRep::ModelLoopRep(const vtkIdType& id, const bool& isInternal)
{
  this->Internal = new Internals();
  this->Internal->Id = id;
  this->Internal->IsOuterLoop = isInternal;
}

//----------------------------------------------------------------------------
ModelLoopRep::~ModelLoopRep()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::operator==(const ModelLoopRep& lr) const
{
  return (this->Internal->Id == lr.Internal->Id &&
          this->Internal->IsOuterLoop == lr.Internal->IsOuterLoop &&
          this->Internal->ModelEdges == lr.Internal->ModelEdges &&
          this->Internal->Segments == lr.Internal->Segments &&
          this->Internal->PointsToIds == lr.Internal->PointsToIds &&
          this->Internal->IdsToPoints == lr.Internal->IdsToPoints);
}

//----------------------------------------------------------------------------
bool ModelLoopRep::operator<(const ModelLoopRep& lr) const
{
  return (this->Internal->Id < lr.Internal->Id);
}

//----------------------------------------------------------------------------
bool ModelLoopRep::isOuterLoop() const
{
  return this->Internal->IsOuterLoop;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::isDegenerateLoop() const
{
  bool isDegenerate = true;
  std::map<vtkIdType,int>::const_iterator modelEdgeIt;
  for ( modelEdgeIt = this->Internal->ModelEdges.begin();
        modelEdgeIt != this->Internal->ModelEdges.end() && isDegenerate;
        modelEdgeIt++)
    {
    //if we find a edge that is only used once we know that we don't have
    //a fully degenerate loop
    isDegenerate = (modelEdgeIt->second > 1);
    }
  if ( isDegenerate )
    {
    return true;
    }

  //these methods below presume every edge has only
  //one edge use
  if ( this->Internal->PointsToIds.size() <= 2 )
      {
      //two or less points can never form a hole
      return true;
      }

  //use case is a hole whose edge length has caused it to become
  //a straight line. In that case we need to verify that
  //the points aren't all collinear. If all the points are
  //collinear the loop is a line.
  std::map<meshVertex,vtkIdType>::const_iterator it;
  it = this->Internal->PointsToIds.begin();
  const meshVertex *p1 = &(it->first);
  it++;
  const meshVertex *p2 = &(it->first);
  for(;it != this->Internal->PointsToIds.end(); it++)
    {
    const meshVertex *p3 = &(it->first);
    isDegenerate = arePointsCollinear(
      p1->x,p1->y,p2->x,p2->y,p3->x,p3->y);
    if ( !isDegenerate)
      {
      //we found the first non collinear segment this does form a proper hole.
      break;
      }
    }
  return isDegenerate;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::edgeExists(const vtkIdType &e) const
{
  return this->Internal->ModelEdges.count(e) > 0;
}

//----------------------------------------------------------------------------
void ModelLoopRep::addEdge(const ModelEdgeRep &edge)
{
  if ( !this->edgeExists(edge.getId()) )
    {
    this->Internal->ModelEdges.insert(std::pair<vtkIdType,int>(edge.getId(),1));
    this->addEdgeToLoop(edge);
    }
  else
    {
    //duplicate edge, this helps determine if the loop is a hole
    //increment the model edges counter so we know that we can't use
    //this edge when doing any of the point inside calculations
    this->Internal->ModelEdges.find(edge.getId())->second++;
    }
}

//----------------------------------------------------------------------------
void ModelLoopRep::addEdgeToLoop(const ModelEdgeRep &edge)
{
  //remove the mesh points from the edge, and into the loop.
  //add all the model verts to the loop
  //add all the segments to the loop
  std::map<vtkIdType,meshVertex>& meshPoints = edge.Internal->MeshPoints;
  std::list<meshEdge>& edgeSegments = edge.Internal->Segments;

  vtkIdType pId1, pId2, newPId1, newPId2;
  std::list<meshEdge>::const_iterator it;
  for(it=edgeSegments.begin();it!=edgeSegments.end();it++)
    {
    //add each point to the mapping, and than add that segment
    pId1 = it->first();
    meshVertex ep = meshPoints.find(pId1)->second;
    newPId1 = this->insertPoint(ep);

    pId2 = it->second();
    ep = meshPoints.find(pId2)->second;
    newPId2 = this->insertPoint(ep);

    //add the new segment
    meshEdge es(newPId1,newPId2,it->modelId());
    this->Internal->Segments.insert(es);
    }
}
//----------------------------------------------------------------------------
vtkIdType ModelLoopRep::insertPoint(const meshVertex &point)
{
  std::pair<std::map<meshVertex,vtkIdType>::iterator,bool> ret;
  //use the number of points as ids. This way we make sure the no two
  //points map to the same id
  vtkIdType id(this->Internal->PointsToIds.size());
  ret = this->Internal->PointsToIds.insert(
      std::pair<meshVertex,vtkIdType>(point,id));
  if ( ret.second )
    {
    //if we added the point, update both parts of the bidirectional map
    this->Internal->IdsToPoints.insert(
      std::pair<vtkIdType,meshVertex>(id,point));
    }
  //the ret will point to the already existing element,
  //or the newely created element
  return ret.first->second;
}

//----------------------------------------------------------------------------
const meshVertex* ModelLoopRep::getPoint(const vtkIdType &id) const
{
  std::map<vtkIdType,meshVertex>::const_iterator it;
  it = this->Internal->IdsToPoints.find(id);
  if ( it == this->Internal->IdsToPoints.end() )
    {
    return NULL;
    }
  return &it->second;
}

//----------------------------------------------------------------------------
const meshVertex* ModelLoopRep::getPoint(const double &x, const double &y) const
{
  std::map<meshVertex,vtkIdType>::const_iterator it;
  meshVertex p(x,y);
  it = this->Internal->PointsToIds.find(p);
  if ( it == this->Internal->PointsToIds.end() )
    {
    return NULL;
    }
  return &it->first;
}

//----------------------------------------------------------------------------
vtkIdType ModelLoopRep::getMeshVertexId(const double &x, const double &y) const
{
  std::map<meshVertex,vtkIdType>::const_iterator it;
  meshVertex p(x,y);
  it = this->Internal->PointsToIds.find(p);
  if ( it == this->Internal->PointsToIds.end() )
    {
    return -1;
    }
  return it->second;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::pointClassification(const vtkIdType &pointId,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  const meshVertex *vert = this->getPoint(pointId);
  if ( vert == NULL)
    {
    return false;
    }

  //the point exists, make sure it has a valid modelEntityType
  //we don't want to modify the type and uniqueId unless our
  //point is properly classified
  if ( vert->modelEntityType != vtkModelTypeCOPY && vert->modelId > -1)
    {
    modelEntityType = vert->modelEntityType;
    uniqueId = vert->modelId;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::pointClassification(const double &x, const double &y,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  const meshVertex *vert = this->getPoint(x,y);
  if ( vert == NULL)
    {
    return false;
    }

  //the point exists, make sure it has a valid modelEntityType.
  //We don't want to modify the type and uniqueId unless our
  //point is properly classified
  if ( vert->modelEntityType != vtkModelTypeCOPY && vert->modelId > -1)
    {
    modelEntityType = vert->modelEntityType;
    uniqueId = vert->modelId;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool ModelLoopRep::edgeClassification(const vtkIdType &pointId1, const vtkIdType &pointId2,
  int &modelEntityType, vtkIdType &uniqueId) const
{
  if ( pointId1 < 0 || pointId2 < 0 )
    {
    return false;
    }

  //create the edge that we need to lookup
  meshEdge es(pointId1,pointId2);
  std::set<meshEdge>::const_iterator esIt = this->Internal->Segments.find(es);
  if (esIt == this->Internal->Segments.end())
    {
    //the lookup is limited by the ordering, so switch the ordering
    //and try again
    es = meshEdge(pointId2,pointId1);
    esIt = this->Internal->Segments.find(es);
    if ( esIt == this->Internal->Segments.end() )
      {
      //both edges that were tested are not valid
      return false;
      }
    }

  //We have found an edge.
  //We know only want to update the modelEntityType and uniqueId if
  //the edges modelEntityType and modelId are valid
  if (esIt->modelEntityType() != vtkModelTypeCOPY && esIt->modelId() > -1 )
    {
    modelEntityType = esIt->modelEntityType();
    uniqueId = esIt->modelId();
    return true;
    }
  return false;
}


//----------------------------------------------------------------------------
bool ModelLoopRep::edgeClassification(const double &x1, const double &y1,
    const double &x2, const double &y2,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  vtkIdType p1 = this->getMeshVertexId(x1,y1);
  vtkIdType p2 = this->getMeshVertexId(x2,y2);
  if (p1 == -1 || p2 == -1 )
    {
    return false;
    }
  else
    {
    return this->edgeClassification(p1,p2,modelEntityType,uniqueId);
    }
}

//----------------------------------------------------------------------------
int ModelLoopRep::numberOfVertices() const
{
  return static_cast<int>(this->Internal->PointsToIds.size());
}

//----------------------------------------------------------------------------
int ModelLoopRep::numberOfEdges() const
{
  return static_cast<int>(this->Internal->Segments.size());
}

//----------------------------------------------------------------------------
void ModelLoopRep::addDataToTriangleInterface(cmbFaceMesherInterface *ti,
   int &pointIndex, int &segmentIndex, int &holeIndex)
{
  std::map<vtkIdType,meshVertex>::iterator pointIt;
  //we have to iterate idsToPoints, to properly get the right indexing
  //for cell lookup
  int i = 0;
  for (pointIt=this->Internal->IdsToPoints.begin();pointIt!=this->Internal->IdsToPoints.end();pointIt++)
    {
    //account for the offset of the other points
    i = pointIndex + pointIt->first;
    if(pointIt->second.modelEntityType == vtkModelVertexTypeCOPY)
      {
      //If this is a model vertex, add the id to the point
      ti->setPoint(i,pointIt->second.x,pointIt->second.y, pointIt->second.modelId);
      }
    else
      {
      ti->setPoint(i,pointIt->second.x,pointIt->second.y);
      }

    }
  pointIndex += static_cast<int>(this->Internal->IdsToPoints.size()); //update the pointIndex

  std::set<meshEdge>::iterator segIt;
  i=segmentIndex;
  for (segIt=this->Internal->Segments.begin();segIt!=this->Internal->Segments.end();segIt++)
    {
    //make sure we offset the cell ids by the total
    //number of cells in all the previous loops
    ti->setSegment(i++,segmentIndex + segIt->first(), segmentIndex + segIt->second(), (segIt)->modelId());
    }
  segmentIndex += static_cast<int>(this->Internal->Segments.size());

  //we can have edges that are part of a loop, but not the hole in the loop
  //think of a cube with an interior line

  if (!this->isOuterLoop())
    {
    double holeX, holeY;
    bool pointInHoleFound = this->findAPointInside(holeX,holeY);
    if ( pointInHoleFound )
      {
      ti->setHole(holeIndex++,holeX,holeY);
      }
    }
}

bool ModelLoopRep::findAPointInside(double& x,double& y) const
{
  if ( this->isDegenerateLoop() )
    {
    return false;
    }
  //first we do a simple convex algorithm. This will find
  //a hole point for all simple shapes
  bool found = this->findPointInsideConvex(x,y);
  if ( found )
    {
    return true;
    }

  //if we can't find the a hole point with the simple check
  //we move onto using a slower but better algorithm
  found = this->findPointInsideConcave(x,y);
  if ( found )
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::isNonManifoldEdge(const vtkIdType &modelEdgeId) const
{
  std::map<vtkIdType,int>::const_iterator modelEdgeIt;
  modelEdgeIt = this->Internal->ModelEdges.find(modelEdgeId);
  if ( modelEdgeIt == this->Internal->ModelEdges.end() )
    {
    //this is an invalid segment for this loop!
    return false;
    }

  return modelEdgeIt->second > 1;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::findPointInsideConvex(double& x,double& y) const
{
  //use center of bounding box as the point.
  //This only works for simple convex shapes
  double boundsVar[4];
  this->bounds(boundsVar);

  double center[2];
  centerOfBounds(boundsVar,center);

  if ( !this->isBoundaryPoint(center[0],center[1]) &&
       this->isPointInside(center[0],center[1]))
    {
    x = center[0];
    y = center[1];
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::findPointInsideConcave(double& x,double& y) const
{
  //construct a circle that is larger than the bounding box.
  //select a point on the sphere and locate the closest segment to that point.

  //calculate out the ray from the sphere point and the found segment
  //Take the normal of the line thats dot product with the rays vector is
  //equal to zero. And generate a new point on the normal line that is just
  //a delta inside the loop.

  //if the new points  closest segment is the circle point closest segment
  //the point is a valid hole point. Otherwise move onto a new point on the circle.
  meshVertex pointOnSegment(0,0);


  double boundsVar[4];
  double center[2],circlePoint[2], holePoint[2], rad;

  this->bounds(boundsVar);
  double radius = std::max(boundsVar[2]-boundsVar[0], boundsVar[3]-boundsVar[1]);
  centerOfBounds(boundsVar,center); //get the center of the current bounds

  for (double i=0; i < 360; i+=5)
    {
    rad = vtkMath::RadiansFromDegrees(i);
    circlePoint[0] = center[0] + cos(rad) * radius;
    circlePoint[1] = center[1] + sin(rad) * radius;

    //now find the closest segment to this point
    const meshEdge *closestEdgeToCircle = this->findClosestSegment(
      circlePoint[0],circlePoint[1], pointOnSegment);
    if ( closestEdgeToCircle == NULL )
      {
      continue;
      }

    //verify the closest edge segment isn't collinear to the ray point
    //because that is an invalid place to start from
    const meshVertex *p1 = this->getPoint(closestEdgeToCircle->first());
    const meshVertex *p2 = this->getPoint(closestEdgeToCircle->second());
    bool collinear = arePointsCollinear(p1->x,p1->y,p2->x,p2->y,
      circlePoint[0],circlePoint[1]);
    if ( collinear )
      {
      continue;
      }

    //find the direction from the segment point of the circle point grab that
    //sign and flip it and multiple the tolerance by that to determine
    //the hole point
    double signX = (circlePoint[0] - pointOnSegment.x);
    double signY = (circlePoint[1] - pointOnSegment.y);
    if(signX < 0)
      {
      signX = 1;
      }
    else if(signX>0)
      {
      signX = -1;
      }
    if(signY < 0)
      {
      signY = 1;
      }
    else if(signY>0)
      {
      signY = -1;
      }

    double offsetX = ((p2->x - p1->x) * (p2->x - p1->x) * 0.005);
    double offsetY = ((p2->x - p1->x) * (p2->x - p1->x) * 0.005);

    holePoint[0] = pointOnSegment.x + (signX * offsetX);
    holePoint[1] = pointOnSegment.y + (signY * offsetY);

    //verify the closest segment is the one that the circle point found
    const meshEdge *closestEdgeToHole = this->findClosestSegment(
      holePoint[0],holePoint[1],pointOnSegment);
    if ( closestEdgeToHole == NULL )
      {
      continue;
      }

    if ( closestEdgeToHole == closestEdgeToCircle )
      {
      x = holePoint[0];
      y = holePoint[1];
      return true;
      }
    }

  //we have a problem!
  return false;
}
//----------------------------------------------------------------------------
bool ModelLoopRep::isBoundaryPoint(const double& x, const double& y) const
{
  //find if the point is collinear to any of the edges
  //http://mathworld.wolfram.com/Collinear.html
  //we are using the area of the triangle of the three points being
  //zero to mean colinear. If this fails email robert.maynard@kitware.com
  bool collinear = false;
  std::set<meshEdge>::const_iterator segIt;
  for(segIt=this->Internal->Segments.begin();segIt!=this->Internal->Segments.end() && !collinear;
    segIt++)
    {
    const meshVertex *p1 = this->getPoint(segIt->first());
    const meshVertex *p2 = this->getPoint(segIt->second());
    collinear = arePointsCollinear(p1->x,p1->y,p2->x,p2->y,x,y);
    }
  return collinear;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::isPointInside(const double& x, const double& y) const
{

  //http://en.wikipedia.org/wiki/Point_in_polygon RayCasting method
  //shooting the ray along the x axis
  // if this fails email robert.maynard@kitware.com
  bool inside = false;
  std::set<meshEdge>::const_iterator segIt;
  double xintersection;
  for(segIt=this->Internal->Segments.begin();segIt!=this->Internal->Segments.end();
    segIt++)
    {
    if ( this->isNonManifoldEdge(segIt->modelId()))
      {
      //we don't want to use degenerate or hanging lines in our
      //ray trace as they will create an incorrect result
      continue;
      }
    const meshVertex *p1 = this->getPoint(segIt->first());
    const meshVertex *p2 = this->getPoint(segIt->second());
    if (y > std::min(p1->y,p2->y) && y <= std::max(p1->y,p2->y)
      && p1->y != p2->y)
      {
      if (x <= std::max(p1->x,p2->x) )
        {
        xintersection = (y-p1->y)*(p2->x - p1->x)/(p2->y - p1->y) + p1->x;
        if ( p1->x == p2->x || x <= xintersection)
          {
          //each time we intersect we switch if we are in side or not
          inside = !inside;
          }
        }
      }
    }
  return inside;
}
//----------------------------------------------------------------------------
void ModelLoopRep::bounds( double b[4] ) const
{
  std::map<meshVertex,vtkIdType>::const_iterator pointIt;

  if ( this->Internal->PointsToIds.size() == 0 )
    {
    //handle the 0 point use case
    b[0]=b[1]=b[2]=b[3]=0.0;
    }

  //This way we don't care what the bounds variable was when passed in
  //this fixes a possible bug if the input area isn't setup properly
  pointIt=this->Internal->PointsToIds.begin();
  b[0] = b[2] = pointIt->first.x;
  b[1] = b[3] = pointIt->first.y;
  pointIt++;

  for (;pointIt!=this->Internal->PointsToIds.end();pointIt++)
    {
    b[0] = pointIt->first.x < b[0]? pointIt->first.x : b[0];
    b[2] = pointIt->first.x > b[2]? pointIt->first.x : b[2];
    b[1] = pointIt->first.y < b[1]? pointIt->first.y : b[1];
    b[3] = pointIt->first.y > b[3]? pointIt->first.y : b[3];
    }
}

//----------------------------------------------------------------------------
const meshEdge * ModelLoopRep::findClosestSegment(const double &x,
  const double &y, meshVertex &vertex) const
{
  const meshEdge *result = NULL;
  if ( this->Internal->Segments.size() == 0 )
    {
    return NULL;
    }

  double distance = std::numeric_limits<double>::max();
  double mx=0,my=0,dx=0,dy=0,segDistanceSquared=0;
  std::set<meshEdge>::const_iterator segIt;
  for(segIt=this->Internal->Segments.begin();segIt!=this->Internal->Segments.end();
      segIt++)
    {
    if (this->isNonManifoldEdge(segIt->modelId()) )
      {
      continue;
      }


    //we use the center of each segment as the only viable
    //spot for the ray to collide with. If we use the whole segment
    //we generally(90%) get the end of the segment as the closest point
    //which than fails to provide a good start point for the hole point
    //as it is too close to another segment
    const meshVertex *p1 = this->getPoint(segIt->first());
    const meshVertex *p2 = this->getPoint(segIt->second());


    mx = ((p1->x + p2->x ) / 2);
    my = ((p1->y + p2->y ) / 2);

    //find the squared distance from the middle of the segment
    //to the point passed in
    dx = mx - x;
    dy = my - y;
    segDistanceSquared = dx * dx + dy * dy;
    if (distance > segDistanceSquared)
      {
      //found a closer edge
      distance = segDistanceSquared;
      result = &(*segIt);
      vertex.x = mx;
      vertex.y = my;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
struct ModelFaceRep::Internals
{
  std::list<ModelLoopRep> Loops;
};

//----------------------------------------------------------------------------
ModelFaceRep::ModelFaceRep()
{
  this->Internal = new Internals();
}

//----------------------------------------------------------------------------
ModelFaceRep::~ModelFaceRep()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void ModelFaceRep::addLoop(const ModelLoopRep &loop)
{
  this->Internal->Loops.push_back(loop);
}

//----------------------------------------------------------------------------
int ModelFaceRep::numberOfVertices()
{
  //we presume model verts are not shared between loops for this pass
  int sum=0;
  std::list<ModelLoopRep>::iterator it;
  for(it=this->Internal->Loops.begin();it!=this->Internal->Loops.end();it++)
    {
    sum += it->numberOfVertices();
    }
  return sum;
}
//----------------------------------------------------------------------------
int ModelFaceRep::numberOfEdges()
{
  int sum=0;
  std::list<ModelLoopRep>::const_iterator it;
  for(it=this->Internal->Loops.begin();it!=this->Internal->Loops.end();it++)
    {
    sum += it->numberOfEdges();
    }
  return sum;
}
//----------------------------------------------------------------------------
int ModelFaceRep::numberOfHoles()
{
  int sum=0;
  std::list<ModelLoopRep>::const_iterator it;
  for(it=this->Internal->Loops.begin();it!=this->Internal->Loops.end();it++)
    {
    sum += (!it->isOuterLoop() && !it->isDegenerateLoop())? 1 : 0;
    }
  return sum;
}
//----------------------------------------------------------------------------
bool ModelFaceRep::bounds( double b[4])
  {
  std::list<ModelLoopRep>::iterator it = this->Internal->Loops.begin();
  for(; it != this->Internal->Loops.end(); it++)
    {
    if((*it).isOuterLoop())
      {
      (*it).bounds(b);
      return true;
      }
    }
  //vtkWarningMacro("Called bounds on ModelFaceRep without any outer loops");
  return false;
  }
//----------------------------------------------------------------------------
void ModelFaceRep::fillTriangleInterface(cmbFaceMesherInterface *ti)
{
  int pIdx = 0, sId = 0, hId=0;
  std::list<ModelLoopRep>::iterator it;

  for(it=this->Internal->Loops.begin();it!=this->Internal->Loops.end();it++)
    {
    it->addDataToTriangleInterface(ti, pIdx, sId, hId);
    }
}
bool ModelFaceRep::RelateMeshToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId)
{
  bool valid  = this->RelateMeshCellsToModel(mesh, facePersistenId);
  this->RelateMeshPointsToModel(mesh, facePersistenId);
  this->SetFaceIdOnMesh(mesh,facePersistenId);
  //Debug code, dump mesh to file

#ifdef DUMP_DEBUG_DATA
  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInput(mesh);
  std::stringstream buffer;
  buffer << DUMP_DEBUG_DATA_DIR <<"rel"<< facePersistenId << ".vtp";
  writer->SetFileName(buffer.str().c_str());
  writer->Write();
  writer->Update();
#endif

  return valid;
}

//----------------------------------------------------------------------------
bool ModelFaceRep::RelateMeshPointsToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId)
{
  //Currently not needed, as cell relationship is good enough
  vtkPoints *points = mesh->GetPoints();
  if (points == NULL)
    {
    return false;
    }
  //construct the array for each points model type and UniquePersistentId
  vtkIdType size = points->GetNumberOfPoints();

  vtkIntArray *pointModelType = vtkIntArray::New();
  pointModelType->SetNumberOfComponents(1);
  pointModelType->SetNumberOfTuples(size);
  pointModelType->SetName(
    ModelFaceRep::Get2DAnalysisPointModelTypesString());

  vtkIdTypeArray *pointModelUseId = vtkIdTypeArray::New();
  pointModelUseId->SetNumberOfComponents(1);
  pointModelUseId->SetNumberOfTuples(size);
  pointModelUseId->SetName(
    ModelFaceRep::Get2DAnalysisPointModelIdsString());

  //Optimization:
  //because we are generating a mesh with fixed boundaries
  //we know that we know that the first N points are fixed
  //to be identical to the input points. So we know exactly which
  //loop each point is from. Plus after we are done with the loops
  //all the points better be on the face.
  std::list<ModelLoopRep>::iterator it;
  int i=0,loopPointSize=0, type=0;
  vtkIdType id;

  //use direct pointers for speed
  int *pmt = reinterpret_cast<int *>(pointModelType->GetVoidPointer(0));
  vtkIdType *pmu = reinterpret_cast<vtkIdType *>(pointModelUseId->GetVoidPointer(0));

  for(it=this->Internal->Loops.begin();it!=this->Internal->Loops.end();it++)
    {
    loopPointSize = it->numberOfVertices();
    for (i=0; i < loopPointSize; ++i)
      {
      if (it->pointClassification(i,type,id))
        {
        *pmt = type;
        *pmu = id;
        }
      else
        {
        //this should never happen
        *pmt = vtkModelFaceTypeCOPY;
        *pmu = facePersistenId;
        }
      pmt++;
      pmu++;
      }
    }

  //now the rest are owned by the face model we can use fill.
  vtkIdType length = size - this->numberOfVertices();
  if ( length > 0 ) //maybe no new points have been added
    {
    std::fill_n(pmt,length,vtkModelFaceTypeCOPY);
    std::fill_n(pmu,length,facePersistenId);
    }

  mesh->GetPointData()->AddArray(pointModelType);
  mesh->GetPointData()->AddArray(pointModelUseId);

  pointModelType->FastDelete();
  pointModelUseId->FastDelete();
  return true;
}

//----------------------------------------------------------------------------
bool ModelFaceRep::RelateMeshCellsToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId)
{
  //we presume we only have triangle cells
  //if we have other cell arrays this will break
  //if we have non triangle cells in the poly list this will break
  vtkCellArray *cells = mesh->GetPolys();
  if ( cells == NULL )
    {
    return false;
    }

  vtkIdType size = cells->GetNumberOfCells();

  //model type for each point in the cell
  vtkIntArray *cellModelType = vtkIntArray::New();
  cellModelType->SetNumberOfComponents(3);
  cellModelType->SetNumberOfTuples(size);
  cellModelType->SetName(
    ModelFaceRep::Get2DAnalysisCellModelTypesString());

  vtkIdTypeArray *cellModelUseId = vtkIdTypeArray::New();
  cellModelUseId->SetNumberOfComponents(3);
  cellModelUseId->SetNumberOfTuples(size);
  cellModelUseId->SetName(
    ModelFaceRep::Get2DAnalysisCellModelIdsString());

  //Info:
  //We are finding if the line created by each two points in the triangle
  //is an edge in the model if both points that create the edge
  //are part of the same loop. The nice thing is that all loop
  //ids are sequential from zero so we can do this 'easily'

  //use direct pointers for speed
  int *cmt = reinterpret_cast<int *>(cellModelType->GetVoidPointer(0));
  vtkIdType *cmu = reinterpret_cast<vtkIdType *>(cellModelUseId->GetVoidPointer(0));

  //set the cells to all default to face / faceId
  //that makes the following logic easier
  vtkIdType length = size * 3;
  std::fill_n(cmt,length,vtkModelFaceTypeCOPY);
  std::fill_n(cmu,length,facePersistenId);

  std::list<ModelLoopRep>::const_iterator it;
  //stores the current cost of the index to move to next bin
  vtkIdType costs[3]={0,0,0};
  bool canMoveToNextBin[3]={false,false,false}; //stores if this item can move
  bool validBin[3]={false,false,false}; //stores if this is the correct bin
  int currentCost = 0, previousCost=0;
  const int maxCost = this->numberOfVertices();

  //data structs needed inside loops
  int numCanMove = 0;
  int indices[4] = {0,1,2,0}; //used for cell point indexes
  vtkIdType *pts,npts;
  cells->InitTraversal();
  while( cells->GetNextCell(npts,pts) )
    {
    //the number of pts better be 3 or this will walk right off the edge
    //only check if two or more points are under the number of points in the loop
    costs[0] = pts[0]; costs[1] = pts[1]; costs[2] = pts[2];
    int valid = (costs[0] < maxCost && costs[1] < maxCost ) ? 1 : 0;
    valid += (costs[1] < maxCost && costs[2] < maxCost ) ? 1 : 0;
    valid += (costs[2] < maxCost && costs[0] < maxCost ) ? 1 : 0;
    if ( valid >= 1 )
      {
      //on each bin subtract the cost of that bin from both
      //if two are less than/equal to the bin cost and greater than zero.
      //  go register those edges
      //if two are greater than the bin cost, goto next bin.
      //else invalid edge stop.
      previousCost = 0;
      for(it=this->Internal->Loops.begin();it!=this->Internal->Loops.end();it++)
        {
        numCanMove = 0;
        currentCost = it->numberOfVertices();
        //update the costs for this point
        for (int i=0; i<3;++i)
          {
          costs[i] -= previousCost;
          canMoveToNextBin[i] = (costs[i] >= currentCost );
          validBin[i] = (costs[i] >= 0 && !canMoveToNextBin[i]);
          numCanMove += canMoveToNextBin[i] ? 1 : 0;
          }

        //NOTE we specify the cell ordering to be the same as the
        //ordering of the points so: 0-1,1-2,2-0. Because
        //of this we don't ever incement the cell array pointers here
        for ( int i=0; i < 3; ++i)
          {
          int pos0=indices[i], pos1=indices[i+1];
          //verify edge 0 to 1 is from the inputed edge mesh
          if ( validBin[pos0] && validBin[pos1] )
            {
            //set the relation in the array if true only
            //otherwise it is done automatically by the default fill

            //each loop point indexing is zero based
            //so we need to convert the current point id to a the loop point
            //luckily this is the current cost
            it->edgeClassification(costs[pos0],costs[pos1], cmt[pos0], cmu[pos0]);
            }
          }

        //now confirm we have at least two items that can go on
        if ( numCanMove < 2 )
          {
          break;
          }

        previousCost = currentCost;
        }
      }
    cmt+=3;
    cmu+=3;
    }

  mesh->GetCellData()->AddArray(cellModelType);
  mesh->GetCellData()->AddArray(cellModelUseId);

  cellModelType->FastDelete();
  cellModelUseId->FastDelete();

  return true;
}

//----------------------------------------------------------------------------
bool ModelFaceRep::SetFaceIdOnMesh(vtkPolyData *mesh,
  const vtkIdType &facePersistenId)
{
  //we presume we only have triangle cells
  vtkCellArray *cells = mesh->GetPolys();
  if ( cells == NULL )
    {
    return false;
    }

  //we are just applying the full face id to all the cells in the mesh
  //this is needed for saving the entire models mesh to a 2DM file.
  vtkIdType size = cells->GetNumberOfCells();

  vtkIdType *faceIds = new vtkIdType[size]; //cellModelId deletes the memory
  std::fill_n(faceIds,size,facePersistenId);

  //model type for each point in the cell
  vtkIdTypeArray *cellModelId = vtkIdTypeArray::New();
  cellModelId->SetNumberOfComponents(1);
  cellModelId->SetName("ModelId");
  cellModelId->SetArray(faceIds,size,facePersistenId);

  mesh->GetCellData()->AddArray(cellModelId);
  cellModelId->FastDelete();
  return true;
}
