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
#include "CmbTriangleInterface.h"

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
bool InternalLoop::edgeExists(const vtkIdType &e) const
{
  return this->ModelEdges.count(e) > 0;
}

//----------------------------------------------------------------------------
void InternalLoop::addEdge(const InternalEdge &edge)
{
  if ( !this->edgeExists(edge.getId()) )
    {
    this->ModelEdges.insert(edge.getId());
    this->Hole = this->Hole || (edge.getEdgeUse() == 1);
    this->addEdgeToLoop(edge);
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
    newPId1 = this->insertPoint(ep,pId1);

    pId2 = it->second;
    ep = meshPoints.find(pId2)->second;
    newPId2 = this->insertPoint(ep,pId2);

    //add the new segment
    edgeSegment es(newPId1,newPId2);
    this->Segments.push_back(es);

    //update the model vert set
    if (mv.count(pId1) > 0)
      {
      this->ModelVerts.insert(newPId1);
      mv.erase(pId1);
      }
    if (mv.count(pId2) > 0)
      {
      this->ModelVerts.insert(newPId2);
      mv.erase(pId2);
      }
    }
}
//----------------------------------------------------------------------------
vtkIdType InternalLoop::insertPoint(const edgePoint &point,const vtkIdType &id)
{
  std::pair<std::map<edgePoint,vtkIdType>::iterator,bool> ret;
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
void InternalLoop::addDataToTriangleInterface(CmbTriangleInterface *ti,
   int &pointIndex, int &segmentIndex, int &holeIndex)
{
  std::map<edgePoint,vtkIdType>::iterator pointIt;
  for (pointIt=this->PointsToIds.begin();pointIt!=this->PointsToIds.end();pointIt++)
    {
    ti->setPoint(pointIndex++,pointIt->first.x,pointIt->first.y);
    }

  std::list<edgeSegment>::iterator segIt;
  for (segIt=this->Segments.begin();segIt!=this->Segments.end();segIt++)
    {
    ti->setSegement(segmentIndex++,segIt->first,segIt->second);
    }

  if (this->Hole)
    {
    segIt=this->Segments.begin();
    const edgePoint *p1 = this->getPoint(segIt->first);
    const edgePoint *p2 = this->getPoint(segIt->second);

    //hole point start off at middle of the segment
    double mx = (p1->x + p2->x)/2;
    double my = (p1->y + p2->y)/2;
    double dx = (p2->x - p1->x);
    edgePoint holePoint(mx,my);

    bool pointInHoleFound = false;
    while(!pointInHoleFound)
      {
      //use the middle point on the segment
      holePoint.x = mx - dx;
      //see if this point is on an edge of the loop
      if ( !this->pointOnBoundary(holePoint) )
        {
        pointInHoleFound = this->pointInside(holePoint);
        }
      if (!pointInHoleFound)
        {
        //flip the point to the other side
        holePoint.x = mx + dx;
        pointInHoleFound = this->pointInside(holePoint);
        }
      dx /= 2; //move the ray to half the distance

      //Note: should we also move Y if both x fails?
      }
    ti->setHole(holeIndex++,holePoint.x,holePoint.y);
    }
}

//----------------------------------------------------------------------------
bool InternalLoop::pointOnBoundary( const edgePoint &point ) const
{
  //arbitrary big number, if point is on the edge the difference should
  //be much much lower
  double lowest_diff = std::numeric_limits<int>::max();

  std::list<edgeSegment>::const_iterator segIt;
  for(segIt=this->Segments.begin();segIt!=this->Segments.end();
    segIt++)
    {
    const edgePoint *p1 = this->getPoint(segIt->first);
    const edgePoint *p2 = this->getPoint(segIt->second);

    double rise= p2->y - p1->y;
    double run = p2->x - p1->x;

    double Ub1 = ((run*p1->y)-(rise*p1->x));
    double Ub2 = ((run*point.y)-(rise*point.x));
    double diff = fabs(Ub1 - Ub2);
    //because of floating point errors round off
    //the last few digits
    lowest_diff = lowest_diff < diff ? lowest_diff : diff;
    }
  return lowest_diff <= .00001;
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
  for (pointIt=this->PointsToIds.begin();pointIt!=this->PointsToIds.end();pointIt++)
    {
    b[0] = pointIt->first.x < b[0]? pointIt->first.x : b[0];
    b[2] = pointIt->first.x > b[2]? pointIt->first.x : b[2];
    b[1] = pointIt->first.y < b[1]? pointIt->first.y : b[1];
    b[3] = pointIt->first.y > b[3]? pointIt->first.y : b[3];
    }
}

//----------------------------------------------------------------------------
void MeshInformation::addLoop(const InternalLoop &loop)
{
  this->Loops.push_back(loop);
}

//----------------------------------------------------------------------------
int MeshInformation::numberOfPoints()
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
int MeshInformation::numberOfLineSegments()
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
int MeshInformation::numberOfHoles()
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
void MeshInformation::fillTriangleInterface(CmbTriangleInterface *ti)
{
  int pIdx = 0, sId = 0, hId=0;
  std::list<InternalLoop>::iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    it->addDataToTriangleInterface(ti, pIdx, sId, hId);
    }
}
