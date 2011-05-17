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
#include "CmbFaceMeshHelper.h"

#include <limits> //Needed for int max
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "CmbFaceMesherInterface.h"

#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"
#include "vtkArrayIteratorIncludes.h" //needed for VTK_TT
#include "vtkSetGet.h" //needed for VTK_TT

#include "vtkModel.h"

//for data dumping in debug
//#define DUMP_DEBUG_DATA
#ifdef DUMP_DEBUG_DATA
#define DUMP_DEBUG_DATA_DIR "E:/Work/"
#include <sstream>
#include "vtkNew.h"
#include "vtkXMLPolyDataWriter.h"
#endif

using namespace CmbModelFaceMeshPrivate;

//----------------------------------------------------------------------------
meshVertex::meshVertex(const double& a, const double& b):
x(a),
y(b),
modelId(-1),
modelEntityType(vtkModelType)
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
bool meshVertex::operator <(const meshVertex &p) const
{
  return  ((this->x < p.x) ||
          (this->x == p.x && this->y < p.y));
}

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
bool meshEdge::operator<(const meshEdge &p) const
{
  return ((this->First < p.First) ||
         (this->First == p.First && this->Second < p.Second));
}

//----------------------------------------------------------------------------
int meshEdge::modelEntityType() const
{
  return vtkModelEdgeType;
}

//----------------------------------------------------------------------------
void ModelEdgeRep::addModelVert(const vtkIdType &id, double point[3])
{
  meshVertex modelVert(point[0],point[1],id,vtkModelVertexType);
  ModelVerts.insert(modelVert);
  this->updateModelRealtionships();
}

//----------------------------------------------------------------------------
void ModelEdgeRep::setMeshPoints(vtkPolyData *mesh)
{

  this->MeshPoints.clear();
  vtkCellArray *lines = mesh->GetLines();
  double p[3];
  if (lines)
    {
    vtkIdType *pts,npts;
    for(lines->InitTraversal();lines->GetNextCell(npts,pts);)
      {
      for (vtkIdType j=0; j < npts; ++j)
        {
        if ( this->MeshPoints.find(pts[j]) == this->MeshPoints.end())
          {
          mesh->GetPoint(pts[j],p);
          //by default everything is related to the edge, updateModelRealtionship
          //will correct the edges to be related to the model vert when verts are added
          meshVertex ep(p[0],p[1], this->Id,vtkModelEdgeType);
          this->MeshPoints.insert(std::pair<vtkIdType,meshVertex>(pts[j],ep));
          }
        if ( j > 0 )
          {
          meshEdge es(pts[j-1],pts[j], this->Id);
          this->Segments.push_back(es);
          }
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
  int numModelVerts = this->ModelVerts.size();
  int index = 0;
  std::map<vtkIdType,meshVertex>::iterator mpIt;
  std::set<meshVertex>::const_iterator mvIt;
  for ( mpIt = this->MeshPoints.begin(); mpIt != this->MeshPoints.end(); mpIt++)
    {
    mvIt = this->ModelVerts.find( mpIt->second );
    if ( mvIt != this->ModelVerts.end() )
      {
      //we found a model vert, update the point with the model vertex entity type
      //and id.
      mpIt->second.modelEntityType = mvIt->modelEntityType;
      mpIt->second.modelId = mvIt->modelId;
      }
    }

}

//----------------------------------------------------------------------------
int ModelEdgeRep::numberOfEdges() const
{
  return (int)this->Segments.size();
}

//----------------------------------------------------------------------------
bool ModelLoopRep::isHole() const
{
  //holds the number of unique edges we have. If greater than zero
  //we have a hole. Also make sure we are an internal loop (CanBeHole)
  return this->CanBeHole && this->EdgeCount > 0;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::edgeExists(const vtkIdType &e) const
{
  return this->ModelEdges.count(e) > 0;
}

//----------------------------------------------------------------------------
void ModelLoopRep::markEdgeAsDuplicate(const vtkIdType &edgeId)
{
  --this->EdgeCount;
}

//----------------------------------------------------------------------------
void ModelLoopRep::addEdge(const ModelEdgeRep &edge)
{
  if ( !this->edgeExists(edge.getId()) )
    {
    this->ModelEdges.insert(edge.getId());
    this->addEdgeToLoop(edge);
    ++this->EdgeCount;
    }
}

//----------------------------------------------------------------------------
void ModelLoopRep::addEdgeToLoop(const ModelEdgeRep &edge)
{
  //remove the mesh points from the edge, and into the loop.
  //add all the model verts to the loop
  //add all the segments to the loop
  std::map<vtkIdType,meshVertex> meshPoints = edge.getMeshPoints();
  std::list<meshEdge> edgeSegments = edge.getSegments();

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
    this->Segments.insert(es);
    }
}
//----------------------------------------------------------------------------
vtkIdType ModelLoopRep::insertPoint(const meshVertex &point)
{
  std::pair<std::map<meshVertex,vtkIdType>::iterator,bool> ret;
  //use the number of points as ids. This way we make sure the no two
  //points map to the same id
  vtkIdType id(this->PointsToIds.size());
  ret = this->PointsToIds.insert(
      std::pair<meshVertex,vtkIdType>(point,id));
  if ( ret.second )
    {
    //if we added the point, update both parts of the bidirectional map
    this->IdsToPoints.insert(
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
  it = this->IdsToPoints.find(id);
  if ( it == this->IdsToPoints.end() )
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
  it = this->PointsToIds.find(p);
  if ( it == this->PointsToIds.end() )
    {
    return NULL;
    }
  return &it->first;
}

//----------------------------------------------------------------------------
vtkIdType ModelLoopRep::getPointId(const double &x, const double &y) const
{
  std::map<meshVertex,vtkIdType>::const_iterator it;
  meshVertex p(x,y);
  it = this->PointsToIds.find(p);
  if ( it == this->PointsToIds.end() )
    {
    return -1;
    }
  return it->second;
}

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
//----------------------------------------------------------------------------
bool ModelLoopRep::pointClassification(const vtkIdType &pointId,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  const meshVertex *vert = this->getPoint(pointId);
  if ( vert == NULL)
    {
    return false;
    }

  //the point exists, get the relationship it has
  modelEntityType = vert->modelEntityType;
  uniqueId = vert->modelId;
  return true;
}

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
//----------------------------------------------------------------------------
bool ModelLoopRep::pointClassification(const double &x, const double &y,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  const meshVertex *point = this->getPoint(x,y);
  if ( point == NULL)
    {
    return false;
    }

  //the point exists, get the relationship it has
  modelEntityType = point->modelEntityType;
  uniqueId = point->modelId;
  return true;
}

//returns true if the edge is contained in the loop.
//The Ids passed in must be between zero and number of Points - 1
//if the edge is a mesh edge the modelEntityType
//  will be set to vtkModelEdgeType, and the uniqueId will be
//  set to the UniquePersistenId of the edge of the edge
// if the edge isn't on the loop the modelEntityType and uniqueId
//  WILL NOT BE MODIFIED
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
  std::set<meshEdge>::const_iterator esIt = this->Segments.find(es);
  if (esIt == this->Segments.end())
    {
    //the lookup is limited by the ordering, so switch the ordering
    //and try again
    es = meshEdge(pointId2,pointId1);
    esIt = this->Segments.find(es);
    if ( esIt == this->Segments.end() )
      {
      //both edges that were tested are not valid
      return false;
      }
    }
  //we found a valid edge
  modelEntityType = esIt->modelEntityType();
  uniqueId = esIt->modelId();
  return true;
}


//returns true if the edge is contained in the loop.
//The Ids passed in must be between zero and number of Points - 1
//if the edge is a mesh edge the modelEntityType
//  will be set to vtkModelEdgeType, and the uniqueId will be
//  set to the UniquePersistenId of the edge of the edge
// if the edge isn't on the loop the modelEntityType and uniqueId
//  WILL NOT BE MODIFIED
//----------------------------------------------------------------------------
bool ModelLoopRep::edgeClassification(const double &x1, const double &y1,
    const double &x2, const double &y2,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  vtkIdType p1 = this->getPointId(x1,y1);
  vtkIdType p2 = this->getPointId(x2,y2);
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
  return (int)this->PointsToIds.size();
}

//----------------------------------------------------------------------------
int ModelLoopRep::numberOfEdges() const
{
  return (int)this->Segments.size();
}

//----------------------------------------------------------------------------
void ModelLoopRep::addDataToTriangleInterface(CmbFaceMesherInterface *ti,
   int &pointIndex, int &segmentIndex, int &holeIndex)
{
  std::map<vtkIdType,meshVertex>::iterator pointIt;
  //we have to iterate idsToPoints, to properly get the right indexing
  //for cell lookup
  int i = 0;
  for (pointIt=this->IdsToPoints.begin();pointIt!=this->IdsToPoints.end();pointIt++)
    {
    //account for the offset of the other points
    i = pointIndex + pointIt->first;
    ti->setPoint(i,pointIt->second.x,pointIt->second.y);
    }
  pointIndex += (int)this->IdsToPoints.size(); //update the pointIndex

  std::set<meshEdge>::iterator segIt;
  i=segmentIndex;
  for (segIt=this->Segments.begin();segIt!=this->Segments.end();segIt++)
    {
    //make sure we offset the cell ids by the total
    //number of cells in all the previous loops
    ti->setSegement(i++,segmentIndex + segIt->first(), segmentIndex + segIt->second());
    }
  segmentIndex += (int)this->Segments.size();

  if (this->isHole())
    {
    //we can have edges that are part of a loop, but not the hole in the loop
    //think of a cube with an interior line
    bool pointInHoleFound = false;
    for(segIt=this->Segments.begin();segIt!=this->Segments.end() && !pointInHoleFound;
    segIt++)
      {
      const meshVertex *p1 = this->getPoint(segIt->first());
      const meshVertex *p2 = this->getPoint(segIt->second());

      //hole point start off at middle of the segment
      double mx = (p1->x + p2->x)/2;
      double my = (p1->y + p2->y)/2;
      double dx = (p2->x - p1->x);
      double dy = (p2->y - p1->y);

      double holeX, holeY;
      //we are only going to attempt this 6 times
      //for each edge, before switching to a new edge.
      //after 6 times we will most likely start getting false positives from the
      int timesTried=0;
      while(!pointInHoleFound && timesTried <= 6)
        {
        //use the middle point on the segment
        holeX = mx - dy;
        holeY = my + dx;
        //see if this point is on an edge of the loop
        if ( !this->pointOnBoundary(holeX, holeY) )
          {
          pointInHoleFound = this->pointInside(holeX, holeY);
          }
        if (!pointInHoleFound)
          {
          //flip the point to the other side
          holeX = mx + dy;
          holeY = my - dx;
          if ( !this->pointOnBoundary(holeX, holeY) )
            {
            pointInHoleFound = this->pointInside(holeX, holeY);
            }
          }
        dx /= 2; //move the ray to half the distance
        dy /= 2;
        mx = mx - dy;
        my = my - dx;
        ++timesTried;
        }

      if ( pointInHoleFound )
        {
        //only add the point if the while loop was valid
        ti->setHole(holeIndex++,holeX,holeY);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool ModelLoopRep::pointOnBoundary(const double& x, const double& y) const
{
  //find if the point is collinear to any of the lines
  //http://mathworld.wolfram.com/Collinear.html
  //we are using the area of the triangle of the three points being
  //zero to mean colinear. If this fails email robert.maynard@kitware.com
  bool collinear = false;
  std::set<meshEdge>::const_iterator segIt;
  for(segIt=this->Segments.begin();segIt!=this->Segments.end() && !collinear;
    segIt++)
    {
    const meshVertex *p1 = this->getPoint(segIt->first());
    const meshVertex *p2 = this->getPoint(segIt->second());
    double area = (p1->x * ( p2->y - y )  ) +
                    (p2->x * ( y - p1->y )  ) +
                    (x * ( p1->y - p2->y )  );
    collinear = (fabs(area) <= 1e-9);
    }
  return collinear;
}

//----------------------------------------------------------------------------
bool ModelLoopRep::pointInside(const double& x, const double& y) const
{

  //http://en.wikipedia.org/wiki/Point_in_polygon RayCasting method
  //shooting the ray along the x axis
  // if this fails email robert.maynard@kitware.com
  bool inside = false;
  std::set<meshEdge>::const_iterator segIt;
  double xintersection;
  for(segIt=this->Segments.begin();segIt!=this->Segments.end();
    segIt++)
    {
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

  if ( this->PointsToIds.size() == 0 )
    {
    //handle the 0 point use case
    b[0]=b[1]=b[2]=b[3]=0.0;
    }

  //This way we don't care what the bounds variable was when passed in
  //this fixes a possible bug if the input area isn't setup properly
  pointIt=this->PointsToIds.begin();
  b[0] = b[2] = pointIt->first.x;
  b[1] = b[3] = pointIt->first.y;
  pointIt++;

  for (;pointIt!=this->PointsToIds.end();pointIt++)
    {
    b[0] = pointIt->first.x < b[0]? pointIt->first.x : b[0];
    b[2] = pointIt->first.x > b[2]? pointIt->first.x : b[2];
    b[1] = pointIt->first.y < b[1]? pointIt->first.y : b[1];
    b[3] = pointIt->first.y > b[3]? pointIt->first.y : b[3];
    }
}

//----------------------------------------------------------------------------
void ModelFaceRep::addLoop(const ModelLoopRep &loop)
{
  this->Loops.push_back(loop);
}

//----------------------------------------------------------------------------
int ModelFaceRep::numberOfVertices()
{
  //we presume model verts are not shared between loops for this pass
  int sum=0;
  std::list<ModelLoopRep>::iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
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
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
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
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    sum += it->isHole() ? 1 : 0;
    }
  return sum;
}

//----------------------------------------------------------------------------
void ModelFaceRep::fillTriangleInterface(CmbFaceMesherInterface *ti)
{
  int pIdx = 0, sId = 0, hId=0;
  std::list<ModelLoopRep>::iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    it->addDataToTriangleInterface(ti, pIdx, sId, hId);
    }
}

//----------------------------------------------------------------------------
bool ModelFaceRep::RelateMeshToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId)
{
  bool valid  = this->RelateMeshCellsToModel(mesh, facePersistenId);
  this->RelateMeshPointsToModel(mesh, facePersistenId);
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
  pointModelType->SetName("ModelType");

  vtkIdTypeArray *pointModelUseId = vtkIdTypeArray::New();
  pointModelUseId->SetNumberOfComponents(1);
  pointModelUseId->SetNumberOfTuples(size);
  pointModelUseId->SetName("ModelUseId");

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

  for(it=this->Loops.begin();it!=this->Loops.end();it++)
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
        *pmt = vtkModelFaceType;
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
    std::fill_n(pmt,length,vtkModelFaceType);
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
  cellModelType->SetName("ModelType");

  vtkIdTypeArray *cellModelUseId = vtkIdTypeArray::New();
  cellModelUseId->SetNumberOfComponents(3);
  cellModelUseId->SetNumberOfTuples(size);
  cellModelUseId->SetName("ModelUseId");

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
  std::fill_n(cmt,length,vtkModelFaceType);
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
      for(it=this->Loops.begin();it!=this->Loops.end();it++)
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
