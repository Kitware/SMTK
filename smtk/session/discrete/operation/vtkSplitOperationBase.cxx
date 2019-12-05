//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSplitOperationBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkSplitOperationBase);

vtkSplitOperationBase::vtkSplitOperationBase()
{
  this->CreatedModelFaceIDs = vtkIdTypeArray::New();
  this->CreatedModelFaceIDs->SetNumberOfComponents(1);
  this->CreatedModelFaceIDs->SetNumberOfTuples(0);
  this->Id = 0;
  this->IsIdSet = 0;
  this->FeatureAngle = 0;
  this->IsFeatureAngleSet = 0;
  this->CurrentNewFaceId = -1;
}

vtkSplitOperationBase::~vtkSplitOperationBase()
{
  if (this->CreatedModelFaceIDs)
  {
    CreatedModelFaceIDs->Delete();
  }
}

void vtkSplitOperationBase::SetId(vtkIdType id)
{
  this->IsIdSet = 1;
  if (id != this->Id)
  {
    this->Modified();
    this->Id = id;
  }
}

void vtkSplitOperationBase::SetFeatureAngle(double featureAngle)
{
  this->IsFeatureAngleSet = 1;
  if (this->FeatureAngle != featureAngle)
  {
    this->Modified();
    this->FeatureAngle = featureAngle;
  }
}

vtkModelEntity* vtkSplitOperationBase::GetModelEntity(vtkDiscreteModel* Model)
{
  if (!Model || !this->GetIsIdSet())
  {
    return nullptr;
  }
  return Model->GetModelEntity(vtkModelFaceType, this->GetId());
}

bool vtkSplitOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  if (this->GetIsIdSet() == 0)
  {
    vtkErrorMacro("No entity id specified.");
    return 0;
  }
  if (this->IsFeatureAngleSet == 0)
  {
    vtkErrorMacro("FeatureAngle is not set.");
  }

  // make sure the object is really a model face
  vtkDiscreteModelFace* ModelFace = vtkDiscreteModelFace::SafeDownCast(this->GetModelEntity(Model));
  if (!ModelFace)
  {
    vtkErrorMacro("No model face found with Id " << this->GetId());
    return 0;
  }
  return 1;
}

void vtkSplitOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CreatedModelFaceIDs: " << this->CreatedModelFaceIDs << endl;
  os << indent << "Id: " << this->Id << endl;
  os << indent << "IsIdSet: " << this->IsIdSet << endl;
  os << indent << "FeatureAngle: " << this->FeatureAngle << endl;
  os << indent << "IsFeatureAngleSet: " << this->IsFeatureAngleSet << endl;
}
