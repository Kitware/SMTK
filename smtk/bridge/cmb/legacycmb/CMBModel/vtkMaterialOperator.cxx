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

#include "vtkMaterialOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"
#

vtkStandardNewMacro(vtkMaterialOperator);

vtkMaterialOperator::vtkMaterialOperator()
{
  this->OperateSucceeded = 0;
  this->BuiltMaterialId = -1;
  this->DestroySucceeded = 0;
}

vtkMaterialOperator::~vtkMaterialOperator()
{
}

bool vtkMaterialOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkMaterialOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
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

void vtkMaterialOperator::Build(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->BuiltMaterialId = this->Superclass::Build(ModelWrapper->GetModel());
}

void vtkMaterialOperator::Destroy(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->DestroySucceeded = this->Superclass::Destroy(ModelWrapper->GetModel());
}

void vtkMaterialOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "BuiltMaterialId: " << this->BuiltMaterialId << endl;
  os << indent << "DestroySucceeded: " << this->DestroySucceeded << endl;
}
