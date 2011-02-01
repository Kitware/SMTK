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
// Convert a vtkModelFace to a triangle input for meshing. Also
// restores the resulting mesh to a vtkPolyData
#include "CmbFaceMeshHelper.h"

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/list> // Needed for STL list.
#include "vtkPolyData.h"

using namespace CmbModelFaceMeshPrivate;
//----------------------------------------------------------------------------
void InternalEdge::addModelVert(const vtkIdType &id)
{
  ModelVerts.insert(id);
}
//----------------------------------------------------------------------------

void InternalEdge::setNumberMeshPoints(const int &numPoints)
{
  this->numMeshPoints = numPoints;
}

//----------------------------------------------------------------------------
int InternalEdge::numberLineSegments() const
{
  return this->numMeshPoints - 1;
}

//----------------------------------------------------------------------------
bool InternalLoop::edgeExists(const vtkIdType &e) const
{
  return this->ModelEdges.find(e) != this->ModelEdges.end();
}

//----------------------------------------------------------------------------
void InternalLoop::addEdge(InternalEdge &edge)
{
  if ( !this->edgeExists(edge.getId()) )
    {
    this->ModelEdges.insert(std::pair<vtkIdType,InternalEdge>(edge.getId(),edge));
    this->Hole = this->Hole || (edge.getEdgeUse() == 1);
    }
}

//----------------------------------------------------------------------------
int InternalLoop::getNumberOfPoints()
{
  int sum = 0;
  std::set<vtkIdType> mVerts; //needed to remove duplicate vert ids across edges
  std::map<vtkIdType,InternalEdge>::iterator it;
  for(it=this->ModelEdges.begin();it!=this->ModelEdges.end(); it++)
    {
    sum += it->second.numberMeshPoints();
    //not using set_union as the result iterator range can't overlap either inputs
    const std::set<vtkIdType> mv = it->second.modelVerts();
    mVerts.insert(mv.begin(),mv.end());
    }
  sum += static_cast<int>(mVerts.size());
  return sum;
}

//----------------------------------------------------------------------------
int InternalLoop::getNumberOfLineSegments() const
{
  int sum = 0, numEdges=0;
  std::map<vtkIdType,InternalEdge>::const_iterator it;
  for(it=this->ModelEdges.begin();it!=this->ModelEdges.end(); it++, ++numEdges)
    {
    sum += it->second.numberLineSegments();
    }
  if (numEdges == 1 && this->isHole())
    {
    //if a single edge forms a hole the edge number of segments
    //is off by 1
    ++sum;
    }
  return sum;
}


//----------------------------------------------------------------------------
void MeshInformation::addLoop(const InternalLoop &loop)
{
  this->Loops.push_back(loop);
}
//----------------------------------------------------------------------------
int MeshInformation::numberOfPoints()
{
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