//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkSplitOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkSplitOperatorBase);

vtkSplitOperatorBase::vtkSplitOperatorBase()
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

vtkSplitOperatorBase::~vtkSplitOperatorBase()
{
  if(this->CreatedModelFaceIDs)
    {
    CreatedModelFaceIDs->Delete();
    }
}

void vtkSplitOperatorBase::SetId(vtkIdType id)
{
  this->IsIdSet = 1;
  if(id != this->Id)
    {
    this->Modified();
    this->Id = id;
    }
}

void vtkSplitOperatorBase::SetFeatureAngle(double featureAngle)
{
  this->IsFeatureAngleSet = 1;
  if(this->FeatureAngle != featureAngle)
    {
    this->Modified();
    this->FeatureAngle = featureAngle;
    }
}

vtkModelEntity* vtkSplitOperatorBase::GetModelEntity(
  vtkDiscreteModel* Model)
{
  if(!Model || !this->GetIsIdSet())
    {
    return 0;
    }
  return Model->GetModelEntity(vtkModelFaceType, this->GetId());
}

bool vtkSplitOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  if(this->GetIsIdSet() == 0)
    {
    vtkErrorMacro("No entity id specified.");
    return 0;
    }
  if(this->IsFeatureAngleSet == 0)
    {
    vtkErrorMacro("FeatureAngle is not set.");
    }

  // make sure the object is really a model face
  vtkDiscreteModelFace* ModelFace = vtkDiscreteModelFace::SafeDownCast(
    this->GetModelEntity(Model));
  if(!ModelFace)
    {
    vtkErrorMacro("No model face found with Id " << this->GetId() );
    return 0;
    }
  return 1;
}

void vtkSplitOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "CreatedModelFaceIDs: " << this->CreatedModelFaceIDs << endl;
  os << indent << "Id: " << this->Id << endl;
  os << indent << "IsIdSet: " << this->IsIdSet << endl;
  os << indent << "FeatureAngle: " << this->FeatureAngle << endl;
  os << indent << "IsFeatureAngleSet: " << this->IsFeatureAngleSet << endl;
}
