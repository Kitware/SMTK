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

#include "vtkModelEdgeMeshOperator.h"

#include "vtkCMBModel.h"
#include "vtkCMBModelWrapper.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelEdgeMeshOperator);
vtkCxxRevisionMacro(vtkModelEdgeMeshOperator, "792");

vtkModelEdgeMeshOperator::vtkModelEdgeMeshOperator()
{
  this->UniquePersistentId = 0;
  this->Length = 0;
  this->OperateSucceeded = 0;
}

vtkModelEdgeMeshOperator::~vtkModelEdgeMeshOperator()
{
}

void vtkModelEdgeMeshOperator::Operate(vtkCMBModelWrapper* modelWrapper)
{
  vtkModel* model = modelWrapper->GetModel();
  vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(
    model->GetModelEntity(vtkModelEdgeType, this->UniquePersistentId));
  if(modelEdge == NULL)
    {
    vtkErrorMacro("Could not find edge entity with id " << this->GetUniquePersistentId());
    this->OperateSucceeded = 0;
    return ;
    }
  Entity->SetLineResolution(this->GetLineResolution());

  this->OperateSucceeded  = this->Superclass::Operate(ModelWrapper->GetModel());
}

void vtkModelEdgeMeshOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "UniquePersistentId: " << this->UniquePersistentId << endl;
  os << indent << "Length: " << this->Length << endl;
}
