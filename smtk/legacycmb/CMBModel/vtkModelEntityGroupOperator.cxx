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

#include "vtkModelEntityGroupOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"
#

vtkStandardNewMacro(vtkModelEntityGroupOperator);

vtkModelEntityGroupOperator::vtkModelEntityGroupOperator()
{
  this->OperateSucceeded = 0;
  this->BuiltModelEntityGroupId = -1;
  this->DestroySucceeded = 0;
}

vtkModelEntityGroupOperator::~vtkModelEntityGroupOperator()
{
}

bool vtkModelEntityGroupOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkModelEntityGroupOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
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

void vtkModelEntityGroupOperator::Build(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->BuiltModelEntityGroupId =
    this->Superclass::Build(ModelWrapper->GetModel());
}

void vtkModelEntityGroupOperator::Destroy(vtkDiscreteModelWrapper* ModelWrapper)
{
  this->DestroySucceeded = this->Superclass::Destroy(ModelWrapper->GetModel());
}

void vtkModelEntityGroupOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "BuiltModelEntityGroupId: " <<
    this->BuiltModelEntityGroupId << endl;
  os << indent << "DestroySucceeded: " << this->DestroySucceeded << endl;
}
