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
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "CmbFaceMesherInterface.h"

#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"
#include "vtkArrayIteratorIncludes.h" //needed for VTK_TT
#include "vtkSetGet.h" //needed for VTK_TT

#include "vtkModel.h"

using namespace CmbModelFaceMeshPrivate;
//----------------------------------------------------------------------------
void InternalEdge::addModelVert(const vtkIdType &id)
{
  ModelVerts.insert(id);
}
//----------------------------------------------------------------------------

void InternalEdge::setMeshPoints(vtkPolyData *mesh)
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
          edgePoint ep(p[0],p[1]);
          this->MeshPoints.insert(std::pair<vtkIdType,edgePoint>(pts[j],ep));
          }
        if ( j > 0 )
          {
          edgeSegment es(pts[j-1],pts[j]);
          this->Segments.push_back(es);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
int InternalEdge::numberLineSegments() const
{
  return (int)this->Segments.size();
}

//----------------------------------------------------------------------------
bool InternalLoop::isHole() const
{
  //holds the number of unique edges we have. If greater than zero
  //we have a hole. Also make sure we are an internal loop (CanBeHole)
  return this->CanBeHole && this->EdgeCount > 0;
}

//----------------------------------------------------------------------------
bool InternalLoop::edgeExists(const vtkIdType &e) const
{
  return this->ModelEdges.count(e) > 0;
}

//----------------------------------------------------------------------------
void InternalLoop::markEdgeAsDuplicate(const vtkIdType &edgeId)
{
  --this->EdgeCount;
}

//----------------------------------------------------------------------------
void InternalLoop::addEdge(const InternalEdge &edge)
{
  if ( !this->edgeExists(edge.getId()) )
    {
    this->ModelEdges.insert(edge.getId());
    this->addEdgeToLoop(edge);
    ++this->EdgeCount;
    }
}

//----------------------------------------------------------------------------
void InternalLoop::addEdgeToLoop(const InternalEdge &edge)
{
  //remove the mesh points from the edge, and into the loop.
  //add all the model verts to the loop
  //add all the segments to the loop
  std::map<vtkIdType,edgePoint> meshPoints = edge.getMeshPoints();
  std::list<edgeSegment> edgeSegments = edge.getSegments();
  std::set<vtkIdType> mv = edge.getModelVerts();

  vtkIdType pId1, pId2, newPId1, newPId2;
  std::list<edgeSegment>::iterator it;
  for(it=edgeSegments.begin();it!=edgeSegments.end();it++)
    {
    //add each point to the mapping, and than add that segment
    pId1 = it->first;
    edgePoint ep = meshPoints.find(pId1)->second;
    newPId1 = this->insertPoint(ep);

    pId2 = it->second;
    ep = meshPoints.find(pId2)->second;
    newPId2 = this->insertPoint(ep);

    //add the new segment
    edgeSegment es(newPId1,newPId2);
    this->Segments.push_back(es);

    //update the model relationship
    if (mv.count(pId1) > 0)
      {
      //make sure we label this index as relating to a model vert
      this->ModelRealtion.insert(std::pair<vtkIdType,vtkIdType>(newPId1,pId1));
      mv.erase(pId1);
      }
    else
      {
      //make sure we label this index as relating to a model edge
      this->ModelRealtion.insert(std::pair<vtkIdType,vtkIdType>(newPId1,edge.getId()));
      }

    if (mv.count(pId2) > 0)
      {
      //make sure we label this index as relating to a model vert
      this->ModelRealtion.insert(std::pair<vtkIdType,vtkIdType>(newPId2,pId2));
      mv.erase(pId2);
      }
    else
      {
      //make sure we label this index as relating to a model edge
      this->ModelRealtion.insert(std::pair<vtkIdType,vtkIdType>(newPId1,edge.getId()));
      }
    }
}
//----------------------------------------------------------------------------
vtkIdType InternalLoop::insertPoint(const edgePoint &point)
{
  std::pair<std::map<edgePoint,vtkIdType>::iterator,bool> ret;
  //use the number of points as ids. This way we make sure the no two
  //points map to the same id
  vtkIdType id(this->PointsToIds.size());
  ret = this->PointsToIds.insert(
      std::pair<edgePoint,vtkIdType>(point,id));
  if ( ret.second )
    {
    //if we added the point, update both parts of the bidirectional map
    this->IdsToPoints.insert(
      std::pair<vtkIdType,edgePoint>(id,point));
    }
  //the ret will point to the already existing element,
  //or the newely created element
  return ret.first->second;
}

//----------------------------------------------------------------------------
const edgePoint* InternalLoop::getPoint(const vtkIdType &id) const
{
  std::map<vtkIdType,edgePoint>::const_iterator it;
  it = this->IdsToPoints.find(id);
  if ( it == this->IdsToPoints.end() )
    {
    return NULL;
    }
  return &it->second;
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
bool InternalLoop::pointModelRelation(const vtkIdType &pointId,
    int &modelEntityType, vtkIdType &uniqueId) const
{
  if ( pointId < 0 )
    {
    return false;
    }

  std::map<vtkIdType,vtkIdType>::const_iterator it =
     this->ModelRealtion.find(pointId);
  if ( it == this->ModelRealtion.end() )
    {
    return false;
    }

  //the point exists, see if it is a model edge relationship
  modelEntityType = (this->edgeExists( it->second )) ?
      vtkModelEdgeType : vtkModelVertexType;
    uniqueId = it->second;
  return true;
}

//----------------------------------------------------------------------------
int InternalLoop::getNumberOfPoints()
{
  return (int)this->PointsToIds.size();
}

//----------------------------------------------------------------------------
int InternalLoop::getNumberOfLineSegments() const
{
  return (int)this->Segments.size();
}

//----------------------------------------------------------------------------
void InternalLoop::addDataToTriangleInterface(CmbFaceMesherInterface *ti,
   int &pointIndex, int &segmentIndex, int &holeIndex)
{
  std::map<vtkIdType,edgePoint>::iterator pointIt;
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

  std::list<edgeSegment>::iterator segIt;
  i=segmentIndex;
  for (segIt=this->Segments.begin();segIt!=this->Segments.end();segIt++)
    {
    //make sure we offset the cell ids by the total
    //number of cells in all the previous loops
    ti->setSegement(i++,segmentIndex + segIt->first, segmentIndex + segIt->second);
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
      const edgePoint *p1 = this->getPoint(segIt->first);
      const edgePoint *p2 = this->getPoint(segIt->second);

      //hole point start off at middle of the segment
      double mx = (p1->x + p2->x)/2;
      double my = (p1->y + p2->y)/2;
      double dx = (p2->x - p1->x);
      double dy = (p2->y - p1->y);
      edgePoint holePoint(mx,my);

      //we are only going to attempt this 6 times
      //for each edge, before switching to a new edge.
      //after 6 times we will most likely start getting false positives from the
      int timesTried=0;
      while(!pointInHoleFound && timesTried <= 6)
        {
        //use the middle point on the segment
        holePoint.x = mx - dx;
        holePoint.x = mx - dy;
        //see if this point is on an edge of the loop
        if ( !this->pointOnBoundary(holePoint) )
          {
          pointInHoleFound = this->pointInside(holePoint);
          }
        if (!pointInHoleFound)
          {
          //flip the point to the other side
          holePoint.x = mx + dx;
          holePoint.x = mx + dy;
          if ( !this->pointOnBoundary(holePoint) )
            {
            pointInHoleFound = this->pointInside(holePoint);
            }
          }

        //==================
        //Now try move the hole point along the y axis
        //==================
        if (!pointInHoleFound)
          {
          //use the middle point on the segment
          holePoint.y = mx - dx;
          holePoint.y = mx - dy;

          //see if this point is on an edge of the loop
          if ( !this->pointOnBoundary(holePoint) )
            {
            pointInHoleFound = this->pointInside(holePoint);
            }
          if (!pointInHoleFound)
            {
            //flip the point to the other side
            holePoint.y = mx + dx;
            holePoint.y = mx + dy;
            if ( !this->pointOnBoundary(holePoint) )
              {
              pointInHoleFound = this->pointInside(holePoint);
              }
            }
          }
        dx /= 2; //move the ray to half the distance
        dy /= 2;
        ++timesTried;
        }

      if ( pointInHoleFound )
        {
        //only add the point if the while loop was valid
        ti->setHole(holeIndex++,holePoint.x,holePoint.y);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool InternalLoop::pointOnBoundary( const edgePoint &point ) const
{
  //find if the point is collinear to any of the lines
  //http://mathworld.wolfram.com/Collinear.html
  // if this fails email robert.maynard@kitware.com
  bool collinear = false;
  std::list<edgeSegment>::const_iterator segIt;
  for(segIt=this->Segments.begin();segIt!=this->Segments.end() && !collinear;
    segIt++)
    {
    const edgePoint *p1 = this->getPoint(segIt->first);
    const edgePoint *p2 = this->getPoint(segIt->second);
    collinear = fabs((point.y - p1->y) * (point.x - p2->x) -
      (point.y - p2->y) * (point.x - p1->y)) <= 1e-9;
    }
  return collinear;
}

//----------------------------------------------------------------------------
bool InternalLoop::pointInside( const edgePoint &point ) const
{

  //http://en.wikipedia.org/wiki/Point_in_polygon RayCasting method
  //shooting the ray along the x axis
  bool inside = false;
  std::list<edgeSegment>::const_iterator segIt;
  double xintersection;
  for(segIt=this->Segments.begin();segIt!=this->Segments.end();
    segIt++)
    {
    const edgePoint *p1 = this->getPoint(segIt->first);
    const edgePoint *p2 = this->getPoint(segIt->second);
    if (point.y > std::min(p1->y,p2->y) && point.y <= std::max(p1->y,p2->y)
      && p1->y != p2->y)
      {
      if (point.x <= std::max(p1->x,p2->x) )
        {
        xintersection = (point.y-p1->y)*(p2->x - p1->x)/(p2->y - p1->y) + p1->x;
        if ( p1->x == p2->x || point.x <= xintersection)
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
void InternalLoop::bounds( double b[4] ) const
{
  std::map<edgePoint,vtkIdType>::const_iterator pointIt;

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
void InternalFace::addLoop(const InternalLoop &loop)
{
  this->Loops.push_back(loop);
}

//----------------------------------------------------------------------------
int InternalFace::numberOfPoints()
{
  //we presume model verts are not shared between loops for this pass
  int sum=0;
  std::list<InternalLoop>::iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    sum += it->getNumberOfPoints();
    }
  return sum;
}
//----------------------------------------------------------------------------
int InternalFace::numberOfLineSegments()
{
  int sum=0;
  std::list<InternalLoop>::const_iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    sum += it->getNumberOfLineSegments();
    }
  return sum;
}
//----------------------------------------------------------------------------
int InternalFace::numberOfHoles()
{
  int sum=0;
  std::list<InternalLoop>::const_iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    sum += it->isHole() ? 1 : 0;
    }
  return sum;
}

//----------------------------------------------------------------------------
void InternalFace::fillTriangleInterface(CmbFaceMesherInterface *ti)
{
  int pIdx = 0, sId = 0, hId=0;
  std::list<InternalLoop>::iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    it->addDataToTriangleInterface(ti, pIdx, sId, hId);
    }
}

//----------------------------------------------------------------------------
bool InternalFace::RelateMeshToModel(vtkPolyData *mesh, const vtkIdType &facePersistenId)
{
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
  std::list<InternalLoop>::iterator it;
  int i=0,loopPointSize=0, type=0;
  vtkIdType id;

  //use direct pointers for speed
  int *pmt = reinterpret_cast<int *>(pointModelType->GetVoidPointer(0));
  vtkIdType *pmu = reinterpret_cast<vtkIdType *>(pointModelUseId->GetVoidPointer(0));

  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    loopPointSize = it->getNumberOfPoints();
    for (int i=0; i < loopPointSize; ++i)
      {
      if (it->pointModelRelation(i,type,id))
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
  vtkIdType length = size - this->numberOfPoints();
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
