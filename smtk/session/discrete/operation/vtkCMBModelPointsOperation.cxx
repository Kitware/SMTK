//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelPointsOperation.h"

#include "vtkAlgorithm.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBModelPointsOperation);

vtkCxxSetObjectMacro(vtkCMBModelPointsOperation, ModelPoints, vtkPointSet);
vtkCxxSetObjectMacro(vtkCMBModelPointsOperation, ModelPointData, vtkPointData);

vtkCMBModelPointsOperation::vtkCMBModelPointsOperation()
{
  this->ModelPoints = nullptr;
  this->ModelPointData = nullptr;
  this->OperateSucceeded = 0;
}

vtkCMBModelPointsOperation::~vtkCMBModelPointsOperation()
{
  this->SetModelPoints(nullptr);
  this->SetModelPointData(nullptr);
}

void vtkCMBModelPointsOperation::SetModelPointsInput(vtkAlgorithm* dataAlg)
{
  if (dataAlg)
  {
    this->SetModelPoints(vtkPointSet::SafeDownCast(dataAlg->GetOutputDataObject(0)));
  }
}

void vtkCMBModelPointsOperation::SetModelPointDataInput(vtkAlgorithm* dataAlg)
{
  if (dataAlg)
  {
    vtkDataSet* dataObj = vtkDataSet::SafeDownCast(dataAlg->GetOutputDataObject(0));
    if (dataObj)
    {
      this->SetModelPointData(dataObj->GetPointData());
    }
  }
}

void vtkCMBModelPointsOperation::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Changing points of a CMB model.");
  this->OperateSucceeded = 0;
  if ((!this->ModelPoints || !this->ModelPoints->GetPoints()) && !this->ModelPointData)
  {
    vtkWarningMacro("Must set a valid point set or point data.");
    return;
  }

  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model.");
    return;
  }

  if (this->ModelPoints && this->ModelPoints->GetPoints())
  {
    ModelWrapper->SetGeometricEntityPoints(this->ModelPoints->GetPoints());
  }

  if (this->ModelPointData)
  {
    ModelWrapper->SetGeometricEntityPointData(this->ModelPointData);
  }

  this->OperateSucceeded = 1;
}

void vtkCMBModelPointsOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelPoints: " << this->ModelPoints << endl;
}
