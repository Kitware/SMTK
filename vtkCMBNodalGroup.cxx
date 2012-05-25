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
#include "vtkCMBNodalGroup.h"

#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkDiscreteModel.h"
#include "vtkCMBModelEdge.h"
#include "vtkCMBModelFace.h"
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
#include "vtkCMBModelVertex.h"

#include <set>

struct vtkCMBNodalGroupInternals
{
  typedef std::set<vtkIdType> Set;
  typedef Set::iterator SetIterator;
  Set PointIds;
  typedef std::set<vtkModelEntity*> EntitySet;
  typedef EntitySet::iterator EntityIterator;
  EntitySet Entities;
};

vtkCxxRevisionMacro(vtkCMBNodalGroup, "");

vtkCMBNodalGroup* vtkCMBNodalGroup::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCMBNodalGroup"); 
  if(ret) 
    {                                    
    return static_cast<vtkCMBNodalGroup*>(ret);
    } 
  return new vtkCMBNodalGroup;
}

vtkCMBNodalGroup::vtkCMBNodalGroup()
{
  this->Internal = new vtkCMBNodalGroupInternals;
  this->PointLocationType = vtkCMBModelEntityAllPoints;
}

vtkCMBNodalGroup::~vtkCMBNodalGroup()
{
  if(this->Internal)
    {
    delete this->Internal;
    }
  this->Internal = 0;
}
void vtkCMBNodalGroup::AddModelEntity(vtkModelEntity* ModelEntity)
{
  if(ModelEntity)
    {
    this->Internal->Entities.insert(ModelEntity);
    }
}
void vtkCMBNodalGroup::RemoveModelEntity(vtkModelEntity* ModelEntity)
{
  this->Internal->Entities.erase(ModelEntity);
}
void vtkCMBNodalGroup::ClearEntities()
{
  this->Internal->Entities.clear();
}
std::set<vtkModelEntity*>& vtkCMBNodalGroup::GetModelEntities() const
{
  return this->Internal->Entities;
}

void vtkCMBNodalGroup::AddPointId(vtkIdType PointId)
{
  this->Internal->PointIds.insert(PointId);
}

void vtkCMBNodalGroup::RemovePointId(vtkIdType PointId)
{
  this->Internal->PointIds.erase(PointId);
}

void vtkCMBNodalGroup::AddPointIds(vtkIdList* PointIds)
{
  for(vtkIdType i=0;i<PointIds->GetNumberOfIds();i++)
    {
    this->AddPointId(PointIds->GetId(i));
    }
}

void vtkCMBNodalGroup::RemovePointIds(vtkIdList* PointIds)
{
  for(vtkIdType i=0;i<PointIds->GetNumberOfIds();i++)
    {
    this->RemovePointId(PointIds->GetId(i));
    }
}

void vtkCMBNodalGroup::AddPointIdsInModelFace(vtkCMBModelFace* ModelFace)
{
  vtkBitArray* Points = vtkBitArray::New();
  ModelFace->GatherAllPointIdsMask(Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      this->AddPointId(i);
      }
    }
  Points->Delete();
}
void vtkCMBNodalGroup::AddPointIdsInModelEdge(vtkCMBModelEdge* ModelEdge)
{
  vtkBitArray* Points = vtkBitArray::New();
  this->GatherAllPointIdsOfModelEdge(ModelEdge, Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      this->AddPointId(i);
      }
    }
  Points->Delete();
}

void vtkCMBNodalGroup::AddPointIdsInModelFaceInterior(vtkCMBModelFace* ModelFace)
{
  vtkBitArray* AllModelFacePoints = vtkBitArray::New();
  ModelFace->GatherAllPointIdsMask(AllModelFacePoints);
  vtkBitArray* BoundaryModelFacePoints = vtkBitArray::New();
  ModelFace->GatherBoundaryPointIdsMask(BoundaryModelFacePoints);
  for(vtkIdType i=0;i<AllModelFacePoints->GetNumberOfTuples();i++)
    {
    if(AllModelFacePoints->GetValue(i) != 0 && 
       BoundaryModelFacePoints->GetValue(i) == 0)
      {
      this->AddPointId(i);
      }
    }
  AllModelFacePoints->Delete();
  BoundaryModelFacePoints->Delete();
}

void vtkCMBNodalGroup::AddPointIdsInModelFaceBoundary(vtkCMBModelFace* ModelFace)
{
  vtkBitArray* Points = vtkBitArray::New();
  ModelFace->GatherBoundaryPointIdsMask(Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      this->AddPointId(i);
      }
    }
  Points->Delete();
}

void vtkCMBNodalGroup::RemovePointIdsInModelFace(vtkCMBModelFace* ModelFace)
{
  vtkBitArray* Points = vtkBitArray::New();
  ModelFace->GatherAllPointIdsMask(Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      this->RemovePointId(i);
      }
    }
  Points->Delete();
}

void vtkCMBNodalGroup::RemovePointIdsInModelFaceInterior(vtkCMBModelFace* ModelFace)
{
  vtkBitArray* AllModelFacePoints = vtkBitArray::New();
  ModelFace->GatherAllPointIdsMask(AllModelFacePoints);
  vtkBitArray* BoundaryModelFacePoints = vtkBitArray::New();
  ModelFace->GatherBoundaryPointIdsMask(BoundaryModelFacePoints);
  for(vtkIdType i=0;i<AllModelFacePoints->GetNumberOfTuples();i++)
    {
    if(AllModelFacePoints->GetValue(i) != 0 && 
       BoundaryModelFacePoints->GetValue(i) == 0)
      {
      this->RemovePointId(i);
      }
    }
  AllModelFacePoints->Delete();
  BoundaryModelFacePoints->Delete();
}

void vtkCMBNodalGroup::RemovePointIdsInModelFaceBoundary(vtkCMBModelFace* ModelFace)
{
  vtkBitArray* Points = vtkBitArray::New();
  ModelFace->GatherBoundaryPointIdsMask(Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      this->RemovePointId(i);
      }
    }
  Points->Delete();

}  
void vtkCMBNodalGroup::ClearPointIds()
{
  this->Internal->PointIds.clear();
}

vtkIdType vtkCMBNodalGroup::GetNumberOfPointIds()
{
  vtkIdType size = this->Internal->PointIds.size();
  return size;
}

void vtkCMBNodalGroup::GetPointIds(vtkIdList* PointIds)
{
  vtkIdType NumberOfIds = this->GetNumberOfPointIds();
  PointIds->SetNumberOfIds(NumberOfIds);
  vtkIdType counter = 0;
  for(vtkCMBNodalGroupInternals::SetIterator iter=this->Internal->PointIds.begin();
      iter!=this->Internal->PointIds.end();iter++,counter++)
    {
    PointIds->SetId(counter, *iter);
    }
}

int vtkCMBNodalGroup::GetType()
{
  return vtkCMBNodalGroupType;
}

bool vtkCMBNodalGroup::ConstructRepresentation(vtkPolyData* Grid)
{
  Grid->Initialize();
  vtkDiscreteModel* Model = this->GetModel();
  vtkPointSet* MasterGrid = vtkPointSet::SafeDownCast(Model->GetGeometry());
  if(!MasterGrid)
    {
    vtkWarningMacro("Could not access the main grid.");
    return false;
    }
  Grid->SetPoints(MasterGrid->GetPoints());

  vtkIdList* PointIds = vtkIdList::New();
  this->GetPointIds(PointIds);
  vtkIdType NumberOfPointIds = PointIds->GetNumberOfIds();
  
  vtkCellArray* Verts = vtkCellArray::New();
  Verts->Allocate(NumberOfPointIds);
  for(vtkIdType i=0;i<NumberOfPointIds;i++)
    {
    vtkIdType PointId = PointIds->GetId(i);
    Verts->InsertNextCell(1, &PointId);
    }
  PointIds->Delete();
  
  Grid->SetVerts(Verts);
  Verts->Delete();
  return true;
}

vtkDiscreteModel* vtkCMBNodalGroup::GetModel()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelType);
  iter->Begin();
  vtkDiscreteModel* Model = vtkDiscreteModel::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return Model;
}

void vtkCMBNodalGroup::GatherAllPointIdsOfModelEdge(
  vtkCMBModelEdge* ModelEdge, vtkBitArray* Points)
{
  vtkPointSet* Grid = vtkPointSet::SafeDownCast(ModelEdge->GetGeometry());
  if(!Grid)
  {
    vtkWarningMacro("Cannot access model edge's grid.");
    return;
  }
  // Since not all points in Grid's points are attached to cells, we
  // iterate over the cells and get their points to set them in Points.  
  Points->SetNumberOfComponents(1);
  Points->SetNumberOfTuples(Grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<Grid->GetNumberOfPoints();i++)
    {
    Points->SetValue(i, 0);
    }
  vtkIdType NumberOfCells = Grid->GetNumberOfCells();
  vtkIdList* CellPoints = vtkIdList::New();
  for(vtkIdType i=0;i<NumberOfCells;i++)
    {
    Grid->GetCellPoints(i, CellPoints);
    for(vtkIdType j=0;j<CellPoints->GetNumberOfIds();j++)
      {
      vtkIdType id=CellPoints->GetId(j);
      Points->SetValue(id, 1);
      }
    }
  CellPoints->Delete();
}

bool vtkCMBNodalGroup::IsDestroyable()
{
  return 1;
}

bool vtkCMBNodalGroup::Destroy()
{
  // This doesn't contain references to any other objects besides the model
  // so we don't need to remove any associations here.
  return 1;
}

void vtkCMBNodalGroup::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}


void vtkCMBNodalGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
