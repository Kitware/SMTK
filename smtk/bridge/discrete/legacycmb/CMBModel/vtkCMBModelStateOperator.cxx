//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelStateOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelStateOperator);

vtkCMBModelStateOperator::vtkCMBModelStateOperator()
{
  this->OperatorMode = 0;
}

vtkCMBModelStateOperator::~vtkCMBModelStateOperator()
{
}

bool vtkCMBModelStateOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

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
