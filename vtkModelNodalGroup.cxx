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
#include "vtkModelNodalGroup.h"

#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkFeatureEdges.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkSerializer.h"
#include "vtkModelItemIterator.h"
#include "vtkDiscreteModelVertex.h"

#include <set>

struct vtkModelNodalGroupInternals
{
  typedef std::set<vtkIdType> Set;
  typedef Set::iterator SetIterator;
  Set PointIds;
  typedef std::set<vtkModelEntity*> EntitySet;
  typedef EntitySet::iterator EntityIterator;
  EntitySet Entities;
};


vtkModelNodalGroup* vtkModelNodalGroup::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelNodalGroup");
  if(ret)
    {
    return static_cast<vtkModelNodalGroup*>(ret);
    }
  return new vtkModelNodalGroup;
}

vtkModelNodalGroup::vtkModelNodalGroup()
{
  this->Internal = new vtkModelNodalGroupInternals;
  this->PointLocationType = vtkDiscreteModelEntityAllPoints;
}

vtkModelNodalGroup::~vtkModelNodalGroup()
{
  if(this->Internal)
    {
    delete this->Internal;
    }
  this->Internal = 0;
}
void vtkModelNodalGroup::AddModelEntity(vtkModelEntity* modelEntity)
{
  if(modelEntity)
    {
    this->Internal->Entities.insert(modelEntity);
    }
}
void vtkModelNodalGroup::RemoveModelEntity(vtkModelEntity* modelEntity)
{
  this->Internal->Entities.erase(modelEntity);
}
void vtkModelNodalGroup::ClearEntities()
{
  this->Internal->Entities.clear();
}
std::set<vtkModelEntity*>& vtkModelNodalGroup::GetModelEntities() const
{
  return this->Internal->Entities;
}

void vtkModelNodalGroup::AddPointId(vtkIdType pointId)
{
  this->Internal->PointIds.insert(pointId);
}

void vtkModelNodalGroup::RemovePointId(vtkIdType pointId)
{
  this->Internal->PointIds.erase(pointId);
}

void vtkModelNodalGroup::AddPointIds(vtkIdList* pointIds)
{
  for(vtkIdType i=0;i<pointIds->GetNumberOfIds();i++)
    {
    this->AddPointId(pointIds->GetId(i));
    }
}

void vtkModelNodalGroup::RemovePointIds(vtkIdList* pointIds)
{
  for(vtkIdType i=0;i<pointIds->GetNumberOfIds();i++)
    {
    this->RemovePointId(pointIds->GetId(i));
    }
}

void vtkModelNodalGroup::AddPointIdsInModelFace(vtkDiscreteModelFace* modelFace)
{
  vtkBitArray* points = vtkBitArray::New();
  modelFace->GatherAllPointIdsMask(points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      this->AddPointId(i);
      }
    }
  points->Delete();
}
void vtkModelNodalGroup::AddPointIdsInModelEdge(vtkDiscreteModelEdge* modelEdge)
{
  vtkBitArray* points = vtkBitArray::New();
  this->GatherAllPointIdsOfModelEdge(modelEdge, points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      this->AddPointId(i);
      }
    }
  points->Delete();
}

void vtkModelNodalGroup::AddPointIdsInModelFaceInterior(vtkDiscreteModelFace* modelFace)
{
  vtkBitArray* allModelFacePoints = vtkBitArray::New();
  modelFace->GatherAllPointIdsMask(allModelFacePoints);
  vtkBitArray* boundaryModelFacePoints = vtkBitArray::New();
  modelFace->GatherBoundaryPointIdsMask(boundaryModelFacePoints);
  for(vtkIdType i=0;i<allModelFacePoints->GetNumberOfTuples();i++)
    {
    if(allModelFacePoints->GetValue(i) != 0 &&
       boundaryModelFacePoints->GetValue(i) == 0)
      {
      this->AddPointId(i);
      }
    }
  allModelFacePoints->Delete();
  boundaryModelFacePoints->Delete();
}

void vtkModelNodalGroup::AddPointIdsInModelFaceBoundary(vtkDiscreteModelFace* modelFace)
{
  vtkBitArray* points = vtkBitArray::New();
  modelFace->GatherBoundaryPointIdsMask(points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      this->AddPointId(i);
      }
    }
  points->Delete();
}

void vtkModelNodalGroup::RemovePointIdsInModelFace(vtkDiscreteModelFace* modelFace)
{
  vtkBitArray* points = vtkBitArray::New();
  modelFace->GatherAllPointIdsMask(points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      this->RemovePointId(i);
      }
    }
  points->Delete();
}

void vtkModelNodalGroup::RemovePointIdsInModelFaceInterior(vtkDiscreteModelFace* modelFace)
{
  vtkBitArray* allModelFacePoints = vtkBitArray::New();
  modelFace->GatherAllPointIdsMask(allModelFacePoints);
  vtkBitArray* boundaryModelFacePoints = vtkBitArray::New();
  modelFace->GatherBoundaryPointIdsMask(boundaryModelFacePoints);
  for(vtkIdType i=0;i<allModelFacePoints->GetNumberOfTuples();i++)
    {
    if(allModelFacePoints->GetValue(i) != 0 &&
       boundaryModelFacePoints->GetValue(i) == 0)
      {
      this->RemovePointId(i);
      }
    }
  allModelFacePoints->Delete();
  boundaryModelFacePoints->Delete();
}

void vtkModelNodalGroup::RemovePointIdsInModelFaceBoundary(vtkDiscreteModelFace* modelFace)
{
  vtkBitArray* points = vtkBitArray::New();
  modelFace->GatherBoundaryPointIdsMask(points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      this->RemovePointId(i);
      }
    }
  points->Delete();
}

void vtkModelNodalGroup::ClearPointIds()
{
  this->Internal->PointIds.clear();
}

vtkIdType vtkModelNodalGroup::GetNumberOfPointIds()
{
  return static_cast<vtkIdType>(this->Internal->PointIds.size());
}

void vtkModelNodalGroup::GetPointIds(vtkIdList* pointIds)
{
  vtkIdType numberOfIds = this->GetNumberOfPointIds();
  pointIds->SetNumberOfIds(numberOfIds);
  vtkIdType counter = 0;
  for(vtkModelNodalGroupInternals::SetIterator iter=this->Internal->PointIds.begin();
      iter!=this->Internal->PointIds.end();iter++,counter++)
    {
    pointIds->SetId(counter, *iter);
    }
}

int vtkModelNodalGroup::GetType()
{
  return vtkModelNodalGroupType;
}

bool vtkModelNodalGroup::ConstructRepresentation(vtkPolyData* grid)
{
  grid->Initialize();
  vtkDiscreteModel* model = this->GetModel();
  vtkPointSet* masterGrid = vtkPointSet::SafeDownCast(model->GetGeometry());
  if(!masterGrid)
    {
    vtkWarningMacro("Could not access the main grid.");
    return false;
    }
  grid->SetPoints(masterGrid->GetPoints());

  vtkIdList* pointIds = vtkIdList::New();
  this->GetPointIds(pointIds);
  vtkIdType numberOfPointIds = pointIds->GetNumberOfIds();

  vtkCellArray* verts = vtkCellArray::New();
  verts->Allocate(numberOfPointIds);
  for(vtkIdType i=0;i<numberOfPointIds;i++)
    {
    vtkIdType pointId = pointIds->GetId(i);
    verts->InsertNextCell(1, &pointId);
    }
  pointIds->Delete();

  grid->SetVerts(verts);
  verts->Delete();
  return true;
}

vtkDiscreteModel* vtkModelNodalGroup::GetModel()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelType);
  iter->Begin();
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return model;
}

void vtkModelNodalGroup::GatherAllPointIdsOfModelEdge(
  vtkDiscreteModelEdge* modelEdge, vtkBitArray* points)
{
  vtkPointSet* grid = vtkPointSet::SafeDownCast(modelEdge->GetGeometry());
  if(!grid)
  {
    vtkWarningMacro("Cannot access model edge's grid.");
    return;
  }
  // Since not all points in Grid's points are attached to cells, we
  // iterate over the cells and get their points to set them in Points.
  points->SetNumberOfComponents(1);
  points->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    points->SetValue(i, 0);
    }
  vtkIdType numberOfCells = grid->GetNumberOfCells();
  vtkIdList* cellPoints = vtkIdList::New();
  for(vtkIdType i=0;i<numberOfCells;i++)
    {
    grid->GetCellPoints(i, cellPoints);
    for(vtkIdType j=0;j<cellPoints->GetNumberOfIds();j++)
      {
      vtkIdType id=cellPoints->GetId(j);
      points->SetValue(id, 1);
      }
    }
  cellPoints->Delete();
}

bool vtkModelNodalGroup::IsDestroyable()
{
  return 1;
}

bool vtkModelNodalGroup::Destroy()
{
  // This doesn't contain references to any other objects besides the model
  // so we don't need to remove any associations here.
  return 1;
}

void vtkModelNodalGroup::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}


void vtkModelNodalGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
