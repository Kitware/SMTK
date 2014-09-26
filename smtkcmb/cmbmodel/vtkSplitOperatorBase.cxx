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
