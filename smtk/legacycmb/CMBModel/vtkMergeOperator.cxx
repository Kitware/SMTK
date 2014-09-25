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

#include "vtkMergeOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#

vtkStandardNewMacro(vtkMergeOperator);

vtkMergeOperator::vtkMergeOperator()
{
  this->OperateSucceeded = 0;
}

vtkMergeOperator::~vtkMergeOperator()
{
}

void vtkMergeOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!this->AbleToOperate(ModelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  this->OperateSucceeded = this->Superclass::Operate(ModelWrapper->GetModel());
  if(this->OperateSucceeded)
    {
    ModelWrapper->Modified();
    }

  return;
}

bool vtkMergeOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

vtkDiscreteModelGeometricEntity* vtkMergeOperator::GetTargetModelEntity(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  return this->Superclass::GetTargetModelEntity(ModelWrapper->GetModel());
}

void vtkMergeOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
