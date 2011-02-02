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

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/list> // Needed for STL list.
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
        if ( this->MeshPoints.find(pts[j]) != this->MeshPoints.end())
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
  return this->Segments.size();
}

//----------------------------------------------------------------------------
bool InternalLoop::edgeExists(const vtkIdType &e) const
{
  return this->ModelEdges.count(e) > 0;
}

//----------------------------------------------------------------------------
void InternalLoop::addEdge(InternalEdge &edge)
{
  if ( !this->edgeExists(edge.getId()) )
    {
    this->ModelEdges.insert(edge.getId());
    this->Hole = this->Hole || (edge.getEdgeUse() == 1);
    this->addEdgeToLoop(edge);
    }
}

//----------------------------------------------------------------------------
int InternalLoop::addEdgeToLoop(const InternalEdge &edge)
{
  //remove the mesh points from the edge, and into the loop.
  //add all the model verts to the loop
  //add all the segments to the loop
  std::map<vtkIdType,edgePoint> meshPoints = edge.meshPoints();
  std::vector<edgeSegment> edgeSegments = edge.segments();
  std::set<vtkIdType> mv = edge.modeVerts();

  vtkIdType pId1, pId2, newPId1, newPId2;
  std::vector<edgeSegment>::const_iterator it;
  for(it=edgeSegments.begin();it!=edgeSegments.end();it++)
    {
    //add each point to the mapping, and than add that segment
    vtkIdType pId1 = it->first;
    edgePoint ep = meshPoints.find(pId1);
    newPId1 = this->insertPoint(edgePoint,pId1);

    vtkIdType pId2 = it->second;
    edgePoint ep = meshPoints.find(pId2);
    newPId2 = this->insertPoint(edgePoint,pId2);

    //add the new segment
    edgeSegment es(newPId1,newPId2);
    this->Segments.push_back(es);

    //update the model vert set
    if (mv.count(pId1) > 0)
      {
      this->ModelVerts.insert(newPId1);
      mv.remove(pId1);
      }
    if (mv.count(pId2) > 0)
      {
      this->ModelVerts.insert(newPId2);
      mv.remove(pId2);
      }
    }
}
//----------------------------------------------------------------------------
vtkIdType InternalLoop::insertPoint(const edgePoint &point,const vtkIdType &id)
{
  std::pair<std::map<edgePoint,vtkIdType>::iterator,bool> ret;
  ret = this->Points.insert(
      std::pair<edgePoint,vtkIdType>(point,id));
  //the ret will point to the already existing element,
  //or the newely created element
  return ret.first->second;
}

//----------------------------------------------------------------------------
int InternalLoop::getNumberOfPoints()
{
  this->Points.size();
}

//----------------------------------------------------------------------------
int InternalLoop::getNumberOfLineSegments() const
{
  return this->Segments.size();
}

//----------------------------------------------------------------------------
void InternalLoop::addDataToTriangleInterface(CmbTriangleInterface *ti,
   int &pointIndex, int &segmentIndex, int &holeIndex);
{
  std::map<edgePoint,vtkIdType>::iterator pointIt;
  for (pointIt=this->Points.begin();pointIt!=this->Points.end();pointIt++)
    {
    ti->setPoint(pointIndex++,pointIt->first.x,pointIt->first.y);
    }

  std::vector<edgeSegment>::iterator segIt;
  for (segIt=this->Segments.begin();segIt!=this->Segments.end();segIt++)
    {
    ti->setSegement(segmentIndex++,segIt->first,segIt->second);
    }
  if (this->Hole)
    {
    double bounds[4];
    this->getBounds(bounds);

    edgePoint holePoint;
    bool pointInHoleFound = false;
    while(!pointInHoleFound)
      {
      for (segIt=this->Segments.begin();
          segIt!=this->Segments.end() && pointInHoleFound;
          segIt++)
        {
        //use the middle point on the segment
        //Woops, need a way to reference segments to points!
        holePoint.x = ();
        holePoint.y = ();
        //see if this point is on an edge of the loop
        if ( !this->pointOnBoundary(holePoint) )
          {
          pointInHoleFound = this->PointInside(holePoint);
          }
        if (!pointInHoleFound)
          {
          //move our point slight the other way
          }
        }
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

  std::map<edgePoint,vtkIdType>::const_iterator pointIt, point2It;
  point2It = this->Points.begin();
  point2It++; //loop while point2 isn't at the end
  for (pointIt=this->Points.begin();pointIt2!=this->Points.end();
    pointIt++,point2It++)
    {
    double rise = point2It->first.y - pointIt->first.y;
    double run =  point2It->first.x - pointIt->first.x;

    double Ub1 = ((run*pointIt->first.y)-(rise*pointIt->first.x));
    double Ub2 = ((run*point.y)-(rise*point.x));
    double diff = fabs(Ub1 - Ub2);
    //because of floating point errors round off
    //the last few digits
    lowest_diff = lowest_diff < diff ? lowest_diff : diff;
    }
  return lowest_diff <= .00001;
}

//----------------------------------------------------------------------------
bool InternalLoop::PointInside( const edgePoint &point ) const
{


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
  int numPoints, numSegments;
  int pIdx = 0; sId = 0;
  std::list<InternalLoop>::const_iterator it;
  for(it=this->Loops.begin();it!=this->Loops.end();it++)
    {
    it->addDataToTriangleInterface(ti, pIdx, sId)
    }
}
