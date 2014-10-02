//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModel.h"

#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelRegion.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include "vtkSerializer.h"
#include "vtkSmartPointer.h"


//----------------------------------------------------------------------------
vtkModel::vtkModel()
{
  this->LargestUsedUniqueId = -1;
  this->BlockModelGeometricEntityEvent = false;
}

vtkModel::~vtkModel()
{
}

//----------------------------------------------------------------------------
int vtkModel::GetNumberOfGeometricEntities()
{
  int numVtx = this->GetNumberOfAssociations(vtkModelVertexType);
  int numEdge = this->GetNumberOfAssociations(vtkModelEdgeType);
  int numFace = this->GetNumberOfAssociations(vtkModelFaceType);
  return numVtx+numEdge+numFace;
}

//----------------------------------------------------------------------------
int vtkModel::GetNumberOfModelEntities(int itemType)
{
  return this->GetNumberOfAssociations(itemType);
}

//----------------------------------------------------------------------------
vtkModelEntity* vtkModel::GetModelEntity(vtkIdType uniquePersistentId)
{
  vtkSmartPointer<vtkIdList> types = vtkSmartPointer<vtkIdList>::New();
  this->GetItemTypesList(types);
  for(vtkIdType i=0;i<types->GetNumberOfIds();i++)
    {
    vtkModelItemIterator* iter=this->NewIterator(types->GetId(i));
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      vtkModelEntity* modelEntity =
        vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
      if(modelEntity)
        {
        if(modelEntity->GetUniquePersistentId() == uniquePersistentId)
          {
          iter->Delete();
          return modelEntity;
          }
        vtkModelEntity* adjacentModelEntity =
          modelEntity->GetModelEntity(uniquePersistentId);
        if(adjacentModelEntity)
          {
          iter->Delete();
          return adjacentModelEntity;
          }
        }
      }
    iter->Delete();
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkModelEntity* vtkModel::GetModelEntity(int itemType, vtkIdType uniquePersistentId)
{
  vtkModelItemIterator* iter=this->NewIterator(itemType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEntity* modelEntity =
      vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
    if(modelEntity)
      {
      if(modelEntity->GetUniquePersistentId() == uniquePersistentId)
        {
        iter->Delete();
        return modelEntity;
        }
      }
    }
  iter->Delete();

  return 0;
}

//----------------------------------------------------------------------------
bool vtkModel::DestroyModelGeometricEntity(vtkModelGeometricEntity* entity)
{
  if(!entity || !entity->IsDestroyable())
    {
    return 0;
    }
  if(!entity->Destroy())
    {
    return 0;
    }

  this->RemoveAssociation(entity);
  this->Modified();
  return 1;
}

//----------------------------------------------------------------------------
vtkIdType vtkModel::GetNextUniquePersistentId()
{
  this->Modified(); // we may be able to ignore this modification
  return ++this->LargestUsedUniqueId;
}

//----------------------------------------------------------------------------
int vtkModel::GetModelDimension()
{
  if(this->GetNumberOfAssociations(vtkModelRegionType))
    {
    return 3;
    }
  else if(this->GetNumberOfAssociations(vtkModelFaceType))
    {
    return 2;
    }
  else if(this->GetNumberOfAssociations(vtkModelEdgeType))
    {
    return 1;
    }
  else if(this->GetNumberOfAssociations(vtkModelVertexType))
    {
    return 0;
    }
  return -1;
}

//----------------------------------------------------------------------------
int vtkModel::GetType()
{
  return vtkModelType;
}

//----------------------------------------------------------------------------
void vtkModel::Reset()
{
  int types[4] = {vtkModelRegionType, vtkModelFaceType,
                  vtkModelEdgeType, vtkModelVertexType};
  for(int i=0;i<4;i++)
    {
    vtkModelItemIterator* iter = this->NewIterator(types[i]);
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      vtkModelGeometricEntity* geometricEntity =
        vtkModelGeometricEntity::SafeDownCast(iter->GetCurrentItem());
      if(!geometricEntity->IsDestroyable())
        {
        vtkErrorMacro("A model object is not destroyable.");
        }
      if(!geometricEntity->Destroy())
        {
        vtkErrorMacro("A model object couldn't be destroyed.");
        }
      }
    iter->Delete();
    this->RemoveAllAssociations(types[i]);
    }

  this->LargestUsedUniqueId = 0;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkModel::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
  ser->Serialize("LargestUsedUniqueId", this->LargestUsedUniqueId);
}

//----------------------------------------------------------------------------
void vtkModel::InvokeModelGeometricEntityEvent(unsigned long theEvent,
                                               void *callData)
{
  if(!this->BlockModelGeometricEntityEvent)
    {
    this->InvokeEvent(theEvent, callData);
    }
}

//----------------------------------------------------------------------------
void vtkModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "LargestUsedUniqueId: " << this->LargestUsedUniqueId <<"\n";
  os << indent << "BlockModelGeometricEntityEvent: "
     << this->BlockModelGeometricEntityEvent << "\n";
}
