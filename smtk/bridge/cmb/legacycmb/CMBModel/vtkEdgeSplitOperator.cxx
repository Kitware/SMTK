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

#include "vtkEdgeSplitOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelUserName.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkEdgeSplitOperator);

vtkEdgeSplitOperator::vtkEdgeSplitOperator()
{
  this->OperateSucceeded = 0;
}

vtkEdgeSplitOperator::~vtkEdgeSplitOperator()
{
}

vtkModelEntity* vtkEdgeSplitOperator::GetModelEntity(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper || !this->GetIsEdgeIdSet())
    {
    return 0;
    }
  return this->Superclass::GetModelEntity(ModelWrapper->GetModel());
}

bool vtkEdgeSplitOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkEdgeSplitOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Operating on a model.");

  if(!this->AbleToOperate(ModelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  vtkDiscreteModelEdge* Edge =
    vtkDiscreteModelEdge::SafeDownCast(this->GetModelEntity(ModelWrapper));

  vtkIdType newEdgeId = -1, newVertexId;
  this->OperateSucceeded =
    Edge->Split(this->GetPointId(), newVertexId, newEdgeId);
  if(this->OperateSucceeded)
    {
    this->SetCreatedModelEdgeId(newEdgeId);
    this->SetCreatedModelVertexId(newVertexId);
    std::set<vtkIdType> newEnts;
    newEnts.insert(newEdgeId);
    newEnts.insert(newVertexId);
    ModelWrapper->AddGeometricEntities(newEnts);
    }
  vtkDebugMacro("Finished operating on a model.");
  return;
}

void vtkEdgeSplitOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
