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

#include "vtkSplitOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkModelUserName.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSplitOperator);

vtkSplitOperator::vtkSplitOperator()
{
  this->OperateSucceeded = 0;
}

vtkSplitOperator::~vtkSplitOperator()
{
}

vtkModelEntity* vtkSplitOperator::GetModelEntity(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper || !this->GetIsIdSet())
    {
    return 0;
    }
  return this->Superclass::GetModelEntity(ModelWrapper->GetModel());

}

bool vtkSplitOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkSplitOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Operating on a model.");

  if(!this->AbleToOperate(ModelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  vtkDiscreteModelFace* Face =
    vtkDiscreteModelFace::SafeDownCast(this->GetModelEntity(ModelWrapper));

  vtkIdTypeArray* newFaces = this->GetCreatedModelFaceIDs();
  newFaces->Reset();
  newFaces->SetNumberOfComponents(1);
  newFaces->SetNumberOfTuples(0);

  this->OperateSucceeded =
    Face->Split(this->GetFeatureAngle(), this->FaceSplitInfo);
  if(this->OperateSucceeded)
    {
    newFaces->SetNumberOfTuples(this->FaceSplitInfo.size());
    std::set<vtkIdType> newEnts;
    std::map<vtkIdType, FaceEdgeSplitInfo>::iterator
      mit=this->FaceSplitInfo.begin();
    for(vtkIdType i=0;mit!=this->FaceSplitInfo.end();++mit, ++i)
      {
      newEnts.insert(mit->first);
      newFaces->SetValue(i, mit->first);
      // Also add edges for new faces if they are available
      if(Face->GetNumberOfModelEdges())
        {
        vtkModelFace* newFace = vtkModelFace::SafeDownCast(
          ModelWrapper->GetModelEntity(vtkModelFaceType, mit->first));
        newFace->GetModelEdgeIds(newEnts);
        newFace->GetModelVertexIds(newEnts);
        }
      }
    if(Face->GetNumberOfModelEdges())
      {
      Face->GetModelEdgeIds(newEnts); // There could be new edges for original face
      }
    ModelWrapper->AddGeometricEntities(newEnts);
    }

  vtkDebugMacro("Finished operating on a model.");
  return;
}

void vtkSplitOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
