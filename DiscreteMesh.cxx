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

namespace detail
{
  DiscreteMesh::EdgePointIds make_SortedEdge(
                                      const DiscreteMesh::EdgePointIds& e)
  {
    return DiscreteMesh::EdgePointIds( std::min(e.first, e.second),
                                       std::max(e.first, e.second) );
  }

  //converts an edge id into the global id space
  //will return the new id
  vtkIdType GetDataTypeIndex(vtkIdType globalId)
    {
    const int is_negative = static_cast<int>(globalId < 0);
    return (globalId ^ -is_negative);
    }

  DiscreteMesh::DataType GetDataType(vtkIdType globalId)
    {
    return static_cast<DiscreteMesh::DataType>(globalId < 0);
    }

  //type represents the type of id we are transforming
  vtkIdType ConvertIndex(DiscreteMesh::DataType type,
                         vtkIdType& id)
    {
    //convert the type to 0 or -1 where -1 represents is_negative
    const int is_negative_type = -type;
    return id ^ is_negative_type;
    }

  //type represents the type of ids we are transforming
  void ConvertIndices(DiscreteMesh::DataType type,
                           vtkIdList* idsToTransform)
    {
    //convert the type to 0 or -1 where -1 represents is_negative
    const int is_negative_type = -type;
    const vtkIdType size = idsToTransform->GetNumberOfIds();
    vtkIdType* values = idsToTransform->GetPointer(0);
    for(vtkIdType i=0; i < size; ++i)
      {
      values[i] = (values[i] ^ is_negative_type);
      }
    }
}

//=============================================================================
DiscreteMesh::DiscreteMesh()
{
  this->FaceData = vtkPolyData::New();
  this->EdgeData = vtkPolyData::New();
  this->SharedPoints=NULL;

  this->FaceData->SetPoints(this->SharedPoints);
  this->EdgeData->SetPoints(this->SharedPoints);
}

//=============================================================================
DiscreteMesh::DiscreteMesh(vtkPolyData* allData)
{
  this->FaceData = vtkPolyData::New();
  this->EdgeData = vtkPolyData::New();
  this->SharedPoints = allData->GetPoints();

  //we rip the face and edge data out to the allData,
  //and move it into different polydatas here
  this->FaceData->SetPolys(allData->GetPolys());
  this->EdgeData->SetLines(allData->GetLines());

  if(this->SharedPoints)
    {
    this->SharedPoints->Register(NULL);
    }
  this->FaceData->SetPoints(this->SharedPoints);
  this->EdgeData->SetPoints(this->SharedPoints);
}
//=============================================================================
DiscreteMesh::DiscreteMesh(const DiscreteMesh& other)
{
  this->ShallowAssignment(other);
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
  if(this->SharedPoints == other.SharedPoints)
    {
    return *this;
    }

  this->ShallowAssignment(other);
  return *this;
}

//=============================================================================
DiscreteMesh::~DiscreteMesh()
{
  if(this->FaceData)
    {
    this->FaceData->Delete();
    this->FaceData=NULL;
    }

  if(this->EdgeData)
    {
    this->EdgeData->Delete();
    this->EdgeData=NULL;
    }

  if(this->SharedPoints)
    {
    this->SharedPoints->Delete();
    this->SharedPoints=NULL;
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
    vtkIdType numOfCells = 0;
    numOfCells += this->FaceData->GetNumberOfCells();
    numOfCells += this->EdgeData->GetNumberOfCells();
    return numOfCells;
}

//=============================================================================
vtkIdType DiscreteMesh::GetNumberOfFaces() const
{
    return this->FaceData->GetNumberOfCells();
}

//=============================================================================
vtkIdType DiscreteMesh::GetNumberOfEdges() const
{
    return this->EdgeData->GetNumberOfCells();
}

//=============================================================================
vtkIdType DiscreteMesh::GetNumberOfPoints() const
{
  if(this->SharedPoints)
    {
    return this->SharedPoints->GetNumberOfPoints();
    }
  return 0;
}

//=============================================================================
DiscreteMesh::cell_const_iterator DiscreteMesh::CellsBegin() const
{
  return DiscreteMesh::cell_const_iterator(
                this->EdgeData->GetNumberOfCells(),
                this->FaceData->GetNumberOfCells());
}

//=============================================================================
DiscreteMesh::cell_const_iterator DiscreteMesh::CellsEnd() const
{
  const vtkIdType ne = this->EdgeData->GetNumberOfCells();
  const vtkIdType nf = this->FaceData->GetNumberOfCells();
  return DiscreteMesh::cell_const_iterator(ne,nf,ne+nf);
}


//=============================================================================
vtkSmartPointer<vtkPolyData> DiscreteMesh::GetAsSinglePolyData() const
{
  vtkPolyData* shallowStruct = vtkPolyData::New();
  shallowStruct->CopyStructure( this->FaceData );
  shallowStruct->SetLines( this->EdgeData->GetLines() );
  shallowStruct->SetPoints( this->SharedPoints ); //always make sure we have the shared points


  vtkSmartPointer<vtkPolyData> copy = vtkSmartPointer<vtkPolyData>::New();
  copy->DeepCopy(shallowStruct);
  return copy;
}

//=============================================================================
vtkPoints* DiscreteMesh::SharePointsPtr() const
{
  return this->SharedPoints;
}

//=============================================================================
void DiscreteMesh::BuildLinks() const
{
  this->FaceData->BuildLinks();
  this->EdgeData->BuildLinks();
}

//=============================================================================
vtkSmartPointer<vtkIncrementalOctreePointLocator>
DiscreteMesh::BuildPointLocator(DataType type) const
{
  vtkSmartPointer<vtkIncrementalOctreePointLocator> locator =
      vtkSmartPointer<vtkIncrementalOctreePointLocator>::New();

  if(type == FACE_DATA)
    {
    locator->SetDataSet(this->FaceData);
    }
  else if(type == EDGE_DATA )
    {
    locator->SetDataSet(this->EdgeData);
    }
  else
    {
    //both
    vtkNew<vtkPolyData> bothData;
    bothData->SetLines(this->EdgeData->GetLines());
    bothData->SetPolys(this->FaceData->GetPolys());
    locator->SetDataSet(bothData.GetPointer());
    }

  locator->AutomaticOn();
  locator->SetTolerance(0.0);
  locator->BuildLocator();
  return locator;
}

//=============================================================================
bool DiscreteMesh::ComputeCellNormal(vtkIdType index, double norm[3]) const
{
  const DiscreteMesh::DataType type = detail::GetDataType(index);
  index = detail::GetDataTypeIndex(index);
  vtkPolyData* data = this->GetDataFromType(type);

  vtkDataArray *normals = data->GetCellData()->GetNormals();
  if(normals)
    {
    normals->GetTuple(index,norm);
    }
  else
    {
    //mesh doesn't have the normal, lets manually compute it
    vtkNew<vtkIdList> pointIds;
    this->GetCellPointIds(index,pointIds.GetPointer());

    vtkPolygon::ComputeNormal(this->SharedPoints,
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
  const DiscreteMesh::DataType type = detail::GetDataType(index);
  index = detail::GetDataTypeIndex(index);
  vtkPolyData* data = this->GetDataFromType(type);

  return data->GetCellType(index);
}

//=============================================================================
void DiscreteMesh::GetCellPointIds(vtkIdType index, vtkIdList* points) const
{
  const DiscreteMesh::DataType type = detail::GetDataType(index);
  vtkIdType post_index = detail::GetDataTypeIndex(index);
  vtkPolyData* data = this->GetDataFromType(type);
  data->GetCellPoints(post_index,points);
}

//=============================================================================
void DiscreteMesh::GetCellNeighbors(vtkIdType index, vtkIdList* edge,
                                    vtkIdList* neighbors) const
{
  const DiscreteMesh::DataType type = detail::GetDataType(index);
  index = detail::GetDataTypeIndex(index);
  vtkPolyData* data = this->GetDataFromType(type);

  //we have to convert the input edge list so copy
  vtkNew<vtkIdList> transformedEdges;
  transformedEdges->DeepCopy(edge);

  detail::ConvertIndices(type,transformedEdges.GetPointer());

  data->GetCellNeighbors(index,transformedEdges.GetPointer(),neighbors);

  detail::ConvertIndices(type,neighbors);
}

//=============================================================================
void DiscreteMesh::GetCellEdgeNeighbors(vtkIdType index, vtkIdType startEdge,
                          vtkIdType endEdge, vtkIdList* neighbors) const
{
  const DiscreteMesh::DataType type = detail::GetDataType(index);
  index = detail::GetDataTypeIndex(index);
  vtkPolyData* data = this->GetDataFromType(type);

  //start amd end are point indicies so they don't need to be converted/
  //neighbors are cell ids so they do need to be converted
  data->GetCellEdgeNeighbors(index,
                             startEdge,
                             endEdge,
                             neighbors);

  detail::ConvertIndices(type,neighbors);
}

//=============================================================================
void DiscreteMesh::UpdatePoints(vtkPoints* points) const
{
  this->SharedPoints->ShallowCopy(points);
}

//=============================================================================
void DiscreteMesh::GetPoint(vtkIdType index, double xyz[3]) const
{
  this->SharedPoints->GetPoint(index,xyz);
}

//=============================================================================
void DiscreteMesh::GetPointFaceCells(vtkIdType index, vtkIdList* cellsForPoint) const
{
  this->FaceData->GetPointCells(index,cellsForPoint);
}

//=============================================================================
void DiscreteMesh::GetBounds(double bounds[6]) const
{
  this->SharedPoints->GetBounds(bounds);
}

//=============================================================================
void DiscreteMesh::MovePoint(vtkIdType pos, double xyz[3]) const
{
  this->SharedPoints->SetPoint(pos,xyz);
}

//=============================================================================
DiscreteMesh::EdgePointIds DiscreteMesh::AddEdgePoints(
                                         const DiscreteMesh::EdgePoints& e)
{
  DiscreteMesh::EdgePointIds edge(
        this->SharedPoints->InsertNextPoint(e.first),
        this->SharedPoints->InsertNextPoint(e.second) );
   return edge;
}

//=============================================================================
vtkIdType DiscreteMesh::AddEdge(DiscreteMesh::EdgePointIds& e)
{
  vtkIdType id = this->EdgeData->InsertNextCell(VTK_LINE,2,&e.first);
  return detail::ConvertIndex(DiscreteMesh::EDGE_DATA,id);
}

//=============================================================================
DiscreteMesh::FaceResult DiscreteMesh::AddFace(const DiscreteMesh::Face& f) const
{
  typedef DiscreteMesh::Face::ids_const_iterator iter;
  typedef DiscreteMesh::Face::points_const_iterator points_iter;

  const vtkIdType numPoints = f.GetNumberOfPoints();

  DiscreteMesh::FaceResult result;
  result.reserve(numPoints);

  points_iter newPoints = f.points_begin();
  for(iter i=f.ids_begin(); i!=f.ids_end(); ++i)
    {
    vtkIdType pointId;
    if(*i==f.InvalidId())
      {
       pointId = this->SharedPoints->InsertNextPoint(
                                  newPoints->x, newPoints->y, newPoints->z);
      ++newPoints;
      }
    else
      {
      pointId = *i;
      }
    result.push_back(pointId);
    }
  result.CellId =
      this->FaceData->InsertNextCell(f.CellType(),numPoints,&result.front());
  return result;
}

//=============================================================================
//=============================================================================
// Private methods
//=============================================================================
//=============================================================================

//=============================================================================
vtkSmartPointer<vtkPolyData> DiscreteMesh::ShallowCopyFaceData() const
{
  vtkSmartPointer<vtkPolyData> copy = vtkSmartPointer<vtkPolyData>::New();
  copy->ShallowCopy(this->FaceData);
  return copy;
}


//=============================================================================
void DiscreteMesh::ShallowAssignment(const DiscreteMesh& other)
{
  this->FaceData->ShallowCopy( other.FaceData );
  this->EdgeData->ShallowCopy( other.EdgeData );

  if(this->SharedPoints)
   {
   this->SharedPoints->Delete();
   this->SharedPoints=NULL;
   }

  this->SharedPoints = other.SharedPoints;

  if(this->SharedPoints)
   {
   this->SharedPoints->Register(NULL);
   }
}

//=============================================================================
vtkPolyData* DiscreteMesh::GetDataFromType(DiscreteMesh::DataType type) const
{
  vtkPolyData* data_arrays[2] = { this->FaceData, this->EdgeData };
  return data_arrays[static_cast<int>(type)];
}
