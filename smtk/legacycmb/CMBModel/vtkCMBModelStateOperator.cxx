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
#include "vtkCMBModelStateOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItemIterator.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelStateOperator);

//-----------------------------------------------------------------------------
vtkCMBModelStateOperator::vtkCMBModelStateOperator()
{
  this->OperatorMode = 0;
}

//-----------------------------------------------------------------------------
vtkCMBModelStateOperator::~vtkCMBModelStateOperator()
{
}

//-----------------------------------------------------------------------------
bool vtkCMBModelStateOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

//-----------------------------------------------------------------------------
void vtkCMBModelStateOperator::Operate(vtkDiscreteModelWrapper *modelWrapper)
{
  if(!this->AbleToOperate(modelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  if(this->OperatorMode == 0)
    {
    this->OperateSucceeded = this->SaveState(modelWrapper);
    }
  else if(this->OperatorMode == 1)
    {
    this->OperateSucceeded = this->LoadSavedState(modelWrapper);
    }
}

//-----------------------------------------------------------------------------
int vtkCMBModelStateOperator::SaveState(vtkDiscreteModelWrapper *modelWrapper)
{
  vtkStringArray* serialModel = modelWrapper->SerializeModel();

  if(serialModel && serialModel->GetNumberOfTuples()>0)
    {
    this->SerializedModelString->Reset();
    this->SerializedModelString->SetNumberOfTuples(1);

    this->SerializedModelString->DeepCopy(serialModel);

    this->FaceToIds.clear();
    this->EdgeToIds.clear();
    this->VertexToIds.clear();
    this->EntityToProperties.clear();
    vtkDiscreteModel* cmbModel = modelWrapper->GetModel();

    // Model faces
    vtkModelItemIterator* iter=cmbModel->NewIterator(vtkModelFaceType);
    vtkDiscreteModelGeometricEntity* gmEntity = NULL;
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      gmEntity = vtkDiscreteModelFace::SafeDownCast(iter->GetCurrentItem());
      if(gmEntity)
        {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        vtkIdTypeArray* idarray = gmEntity->GetReverseClassificationArray();
        vtkIdType entId = gmEntity->GetThisModelEntity()->GetUniquePersistentId();
        if(idarray)
          {
          vtkIdType numIds = idarray->GetNumberOfTuples();
          vtkIdType* idsP = ids->WritePointer(0, numIds);
          memcpy(idsP, idarray->GetPointer(0), numIds*sizeof(vtkIdType));
          this->FaceToIds[entId] = ids;
          }
        this->EntityToProperties[entId] = vtkModelGeometricEntity::SafeDownCast(
          gmEntity->GetThisModelEntity())->GetDisplayProperty();
        }
      }
    iter->Delete();

    // Model edges
    iter=cmbModel->NewIterator(vtkModelEdgeType);
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      gmEntity = vtkDiscreteModelEdge::SafeDownCast(iter->GetCurrentItem());
      if(gmEntity)
        {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        vtkIdTypeArray* idarray = gmEntity->GetReverseClassificationArray();
        vtkIdType entId = gmEntity->GetThisModelEntity()->GetUniquePersistentId();
        if(idarray)
          {
          vtkIdType numIds = idarray->GetNumberOfTuples();
          vtkIdType* idsP = ids->WritePointer(0, numIds);
          memcpy(idsP, idarray->GetPointer(0), numIds*sizeof(vtkIdType));
          this->EdgeToIds[entId] = ids;
          }
        this->EntityToProperties[entId] = vtkModelGeometricEntity::SafeDownCast(
          gmEntity->GetThisModelEntity())->GetDisplayProperty();
        }
      }
    iter->Delete();

    // Model Vertex
    iter=cmbModel->NewIterator(vtkModelVertexType);
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      vtkDiscreteModelVertex* vtxEntity = vtkDiscreteModelVertex::SafeDownCast(iter->GetCurrentItem());
      if(vtxEntity)
        {
        this->VertexToIds[vtxEntity->GetUniquePersistentId()] = vtxEntity->GetPointId();
        }
      }
    iter->Delete();

    this->Modified();
    return 1;
    }

  return 0;
}

//-----------------------------------------------------------------------------
int vtkCMBModelStateOperator::LoadSavedState(vtkDiscreteModelWrapper *modelWrapper)
{
  if(modelWrapper && this->SerializedModelString->GetNumberOfTuples()>0)
    {
    return modelWrapper->RebuildModel(
      this->SerializedModelString->GetValue(0).c_str(),
      this->FaceToIds, this->EdgeToIds, this->VertexToIds,
      this->EntityToProperties);
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkCMBModelStateOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FaceToIds: \n";
  for(std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it=this->FaceToIds.begin();
    it!=this->FaceToIds.end();it++)
    {
    os << indent << "Face: " << it->first << " IDs: " << it->second << "\n";
    }
  for(std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator it=this->FaceToIds.begin();
    it!=this->EdgeToIds.end();it++)
    {
    os << indent << "Edge: " << it->first << " IDs: " << it->second << "\n";
    }
  for(std::map<vtkIdType, vtkIdType >::iterator it=this->VertexToIds.begin();
    it!=this->VertexToIds.end();it++)
    {
    os << indent << "Vertex: " << it->first << " ID: " << it->second << "\n";
    }
}
