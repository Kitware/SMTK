//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelPointsOperator.h"

#include "vtkAlgorithm.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBModelPointsOperator);

vtkCxxSetObjectMacro(vtkCMBModelPointsOperator, ModelPoints, vtkPointSet);
vtkCxxSetObjectMacro(vtkCMBModelPointsOperator, ModelPointData, vtkPointData);

vtkCMBModelPointsOperator::vtkCMBModelPointsOperator()
{
  this->ModelPoints = 0;
  this->ModelPointData = 0;
  this->OperateSucceeded = 0;
}

vtkCMBModelPointsOperator::~vtkCMBModelPointsOperator()
{
  this->SetModelPoints(0);
  this->SetModelPointData(0);
}

void vtkCMBModelPointsOperator::SetModelPointsInput(vtkAlgorithm* dataAlg)
{
  if (dataAlg)
  {
    this->SetModelPoints(vtkPointSet::SafeDownCast(dataAlg->GetOutputDataObject(0)));
  }
}

void vtkCMBModelPointsOperator::SetModelPointDataInput(vtkAlgorithm* dataAlg)
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

void vtkCMBModelPointsOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
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

void vtkCMBModelPointsOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelPoints: " << this->ModelPoints << endl;
}
