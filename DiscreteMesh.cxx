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
// .NAME DiscreteMesh - CMB's implementation of a modifable mesh.
// .SECTION Description
// The implementation of a mesh that can represent 2D/3D or mixed type
// models.

#include "DiscreteMesh.h"

#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"

//=============================================================================
DiscreteMesh::DiscreteMesh()
{
  this->Data=NULL;
}

//=============================================================================
DiscreteMesh::DiscreteMesh(vtkPolyData* data)
{
  this->Data = data;
  if(this->Data)
    {
    this->Data->Register(NULL);
    }
}

//=============================================================================
DiscreteMesh::DiscreteMesh(const DiscreteMesh& other)
{
  if(this->Data)
    {
    this->Data->Delete();
    this->Data=NULL;
    }

  this->Data = other.Data;

  if(this->Data)
    {
    this->Data->Register(NULL);
    }
}

//=============================================================================
DiscreteMesh& DiscreteMesh::operator=(const DiscreteMesh& other)
{
  //don't match our selves
  if(this == &other)
    {
    return *this;
    }

  //don't overwrite if the data object is shared
  if(this->Data == other.Data)
    {
    return *this;
    }

  //decrement our count
  if(this->Data)
    {
    this->Data->Delete();
    this->Data=NULL;
    }

  this->Data = other.Data;

  if(this->Data)
    {
    this->Data->Register(NULL);
    }

  return *this;
}

//=============================================================================
DiscreteMesh::~DiscreteMesh()
{
  if(this->Data)
    {
    this->Data->Delete();
    this->Data=NULL;
    }
}

//=============================================================================
bool DiscreteMesh::IsValid() const
{
  return (this->GetNumberOfCells() > 0 &&
          this->GetNumberOfPoints() > 0);
}

//=============================================================================
vtkIdType DiscreteMesh::GetNumberOfCells() const
{
    if(this->Data)
      {
      return this->Data->GetNumberOfCells();
      }
    return 0;
}

//=============================================================================
vtkIdType DiscreteMesh::GetNumberOfPoints() const
{
  if(this->Data)
    {
    return this->Data->GetNumberOfPoints();
    }
  return 0;
}

//=============================================================================
vtkSmartPointer<vtkPolyData> DiscreteMesh::ShallowCopy() const
{
  vtkSmartPointer<vtkPolyData> copy = vtkSmartPointer<vtkPolyData>::New();
  if(this->Data)
    {
    copy->ShallowCopy(this->Data);
    }
  return copy;
}

//=============================================================================
vtkSmartPointer<vtkPolyData> DiscreteMesh::DeepCopy() const
{
  vtkSmartPointer<vtkPolyData> copy = vtkSmartPointer<vtkPolyData>::New();
  if(this->Data)
    {
    copy->DeepCopy(this->Data);
    }
  return copy;
}

//=============================================================================
vtkPoints* DiscreteMesh::SharePointsPtr() const
{
  return this->Data->GetPoints();
}

//=============================================================================
void DiscreteMesh::BuildLinks() const
{
  this->Data->BuildLinks();
}

//=============================================================================
vtkSmartPointer<vtkIncrementalOctreePointLocator> DiscreteMesh::BuildPointLocator() const
{
  vtkSmartPointer<vtkIncrementalOctreePointLocator> locator;
  locator->SetDataSet(this->Data);
  locator->AutomaticOn();
  locator->SetTolerance(0.0);
  locator->BuildLocator();
  return locator;
}

//=============================================================================
bool DiscreteMesh::ComputeCellNormal(vtkIdType index, double norm[3]) const
{
  vtkDataArray *normals = this->Data->GetCellData()->GetNormals();
  if(normals)
    {
    normals->GetTuple(index,norm);
    }
  else
    {
    //mesh doesn't have the normal, lets manually compute it
    vtkNew<vtkIdList> pointIds;
    this->GetCellPointIds(index,pointIds.GetPointer());

    vtkPolygon::ComputeNormal(this->Data->GetPoints(),
                              pointIds->GetNumberOfIds(),
                              pointIds->GetPointer(0),
                              norm);
    }
  return true;
}

//=============================================================================
bool DiscreteMesh::ComputeCellCentroid(vtkIdType index, double centroid[3]) const
{
  const int cellType = this->GetCellType(index);
  if (cellType != VTK_TRIANGLE && cellType != VTK_POLYGON)
    {
    return false;
    }

  //lifted from vtkPolygon::ComputeCentroid
  vtkNew<vtkIdList> pointIds;
  this->GetCellPointIds(index,pointIds.GetPointer());
  const vtkIdType numPts = pointIds->GetNumberOfIds();
  double p0[3];
  double a = 1.0 / static_cast<double>(numPts);
  centroid[0] = centroid[1] = centroid[2] = 0.0;
  for (vtkIdType i=0; i < numPts; i++)
    {
    this->GetPoint(pointIds->GetId(i),p0);
    centroid[0] += p0[0];
    centroid[1] += p0[1];
    centroid[2] += p0[2];
    }
  centroid[0] *= a;
  centroid[1] *= a;
  centroid[2] *= a;

  return true;
}

//=============================================================================
int DiscreteMesh::GetCellType(vtkIdType index) const
{
  return this->Data->GetCellType(index);
}

//=============================================================================
void DiscreteMesh::GetCellPointIds(vtkIdType index, vtkIdList* points) const
{
  this->Data->GetCellPoints(index,points);
}

//=============================================================================
void DiscreteMesh::GetCellNeighbors(vtkIdType index, vtkIdList* edge,
                                    vtkIdList* neighbors) const
{
  this->Data->GetCellNeighbors(index,edge,neighbors);
}

//=============================================================================
void DiscreteMesh::GetCellEdgeNeighbors(vtkIdType index, vtkIdType startEdge,
                          vtkIdType endEdge, vtkIdList* neighbors) const
{
  this->Data->GetCellEdgeNeighbors(index,startEdge,endEdge,neighbors);
}

//=============================================================================
void DiscreteMesh::UpdatePoints(vtkPoints* points) const
{
  this->Data->SetPoints(points);
}

//=============================================================================
void DiscreteMesh::GetPoint(vtkIdType index, double xyz[3]) const
{
  this->Data->GetPoint(index,xyz);
}

//=============================================================================
void DiscreteMesh::GetPointCells(vtkIdType index, vtkIdList* cellsForPoint) const
{
  this->Data->GetPointCells(index,cellsForPoint);
}


//=============================================================================
void DiscreteMesh::GetBounds(double bounds[6]) const
{
  this->Data->GetBounds(bounds);
}

//=============================================================================
void DiscreteMesh::MovePoint(vtkIdType pos, double xyz[3]) const
{
  this->Data->GetPoints()->SetPoint(pos,xyz);
}

//=============================================================================
DiscreteMesh::EdgePointIds DiscreteMesh::AddEdgePoints(const DiscreteMesh::EdgePoints& e)
{
  DiscreteMesh::EdgePointIds edge(
        this->Data->GetPoints()->InsertNextPoint(e.first),
        this->Data->GetPoints()->InsertNextPoint(e.second) );
   return edge;
}

//=============================================================================
vtkIdType DiscreteMesh::AddEdge(const DiscreteMesh::EdgePointIds& e,
                                const vtkIdType geometricEdgeId)
{
  return this->Edges.AddEdge(e,geometricEdgeId);
}

//=============================================================================
DiscreteMesh::FaceIds DiscreteMesh::AddFace(const DiscreteMesh::Face& f) const
{
  typedef DiscreteMesh::Face::ids_const_iterator iter;
  typedef DiscreteMesh::Face::points_const_iterator points_iter;

  const vtkIdType numPoints = f.GetNumberOfPoints();

  DiscreteMesh::FaceIds result;
  result.reserve(numPoints);

  points_iter newPoints = f.points_begin();
  for(iter i=f.ids_begin(); i!=f.ids_end(); ++i)
    {
    if(*i==f.InvalidId())
      {
      const vtkIdType pointId = this->Data->GetPoints()->InsertNextPoint(
                                  newPoints->x, newPoints->y, newPoints->z);

      result.push_back(pointId);
      ++newPoints;
      }
    else
      {
      result.push_back(*i);
      }
    }
  this->Data->InsertNextCell(f.CellType(),numPoints,&result[0]);
  return result;
}

namespace
{
  DiscreteMesh::EdgePointIds make_SortedEdge(
                                      const DiscreteMesh::EdgePointIds& e)
  {
    return DiscreteMesh::EdgePointIds( std::min(e.first, e.second),
                                       std::max(e.first, e.second) );
  }
}

//=============================================================================
vtkIdType DiscreteMesh::EdgeMap::AddEdge(const DiscreteMesh::EdgePointIds& origEdge,
                                         const vtkIdType geometricEdgeId)
{
  const EdgePointIds sortedEdge = make_SortedEdge( origEdge );
  const DiscreteMesh::EdgeMap::const_iterator found = this->find(sortedEdge);
  if(found == this->end())
    {
    const std::pair<EdgePointIds,vtkIdType> inserted_edge(sortedEdge,
                                                          geometricEdgeId);
    this->insert(inserted_edge);
    return geometricEdgeId;
    }
  return found->second;
}

//=============================================================================
vtkIdType DiscreteMesh::EdgeMap::GetId(const DiscreteMesh::EdgePointIds &origEdge) const
{
  const EdgePointIds sortedEdge = make_SortedEdge( origEdge );
  const DiscreteMesh::EdgeMap::const_iterator found = this->find(sortedEdge);
  if(found != this->end())
    {
    return found->second;
    }
  return 0;
}
