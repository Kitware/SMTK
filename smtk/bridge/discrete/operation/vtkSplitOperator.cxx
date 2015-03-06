//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


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
  this->FaceSplitInfo.clear();

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
