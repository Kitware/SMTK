/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

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

vtkCxxRevisionMacro(vtkModel, "$Revision: 2586 $");

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
vtkModelEntity* vtkModel::GetModelEntity(vtkIdType UniquePersistentId)
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
        if(modelEntity->GetUniquePersistentId() == UniquePersistentId)
          {
          iter->Delete();
          return modelEntity;
          }
        vtkModelEntity* adjacentModelEntity =
          modelEntity->GetModelEntity(UniquePersistentId);
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
vtkModelEntity* vtkModel::GetModelEntity(int itemType, vtkIdType UniquePersistentId)
{
  vtkModelItemIterator* iter=this->NewIterator(itemType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEntity* modelEntity =
      vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
    if(modelEntity)
      {
      if(modelEntity->GetUniquePersistentId() == UniquePersistentId)
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

  this->RemoveAssociation(entity->GetType(), entity);
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

