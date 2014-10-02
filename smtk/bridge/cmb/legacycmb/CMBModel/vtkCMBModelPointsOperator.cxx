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

#include "vtkCMBModelPointsOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkAlgorithm.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkCMBModelPointsOperator);

vtkCxxSetObjectMacro(vtkCMBModelPointsOperator,ModelPoints,vtkPointSet);
vtkCxxSetObjectMacro(vtkCMBModelPointsOperator,ModelPointData,vtkPointData);

vtkCMBModelPointsOperator::vtkCMBModelPointsOperator()
{
  this->ModelPoints = 0;
  this->ModelPointData = 0;
  this->OperateSucceeded = 0;
}

vtkCMBModelPointsOperator:: ~vtkCMBModelPointsOperator()
{
  this->SetModelPoints(0);
  this->SetModelPointData(0);
}

void vtkCMBModelPointsOperator::SetModelPointsInput(vtkAlgorithm* dataAlg)
{
  if(dataAlg)
    {
    this->SetModelPoints(vtkPointSet::SafeDownCast(
      dataAlg->GetOutputDataObject(0)));
    }
}

void vtkCMBModelPointsOperator::SetModelPointDataInput(vtkAlgorithm* dataAlg)
{
  if(dataAlg)
    {
    vtkDataSet* dataObj = vtkDataSet::SafeDownCast(
      dataAlg->GetOutputDataObject(0));
    if(dataObj)
      {
      this->SetModelPointData(dataObj->GetPointData());
      }
    }
}

void vtkCMBModelPointsOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Changing points of a CMB model.");
  this->OperateSucceeded = 0;
  if((!this->ModelPoints || !this->ModelPoints->GetPoints()) &&
      !this->ModelPointData)
    {
    vtkWarningMacro("Must set a valid point set or point data.");
    return;
    }

  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }

  if(this->ModelPoints && this->ModelPoints->GetPoints())
    {
    ModelWrapper->SetGeometricEntityPoints(this->ModelPoints->GetPoints());
    }

  if(this->ModelPointData)
    {
    ModelWrapper->SetGeometricEntityPointData(this->ModelPointData);
    }

  this->OperateSucceeded = 1;
}

void vtkCMBModelPointsOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ModelPoints: " << this->ModelPoints << endl;
}
