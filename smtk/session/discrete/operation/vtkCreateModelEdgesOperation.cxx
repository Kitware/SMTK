//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCreateModelEdgesOperation.h"

#include "ModelEdgeHelper.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItemListIterator.h"
#include "vtkNew.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCreateModelEdgesOperation);

vtkCreateModelEdgesOperation::vtkCreateModelEdgesOperation()
{
  this->OperateSucceeded = 0;
  this->ShowEdges = 0;
}

vtkCreateModelEdgesOperation::~vtkCreateModelEdgesOperation()
{
}

bool vtkCreateModelEdgesOperation::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

void vtkCreateModelEdgesOperation::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Operating on a model.");

  if (!this->AbleToOperate(ModelWrapper))
  {
    this->OperateSucceeded = 0;
    return;
  }

  this->Model = ModelWrapper->GetModel();
  // Need to build links to get facet neighborhoods
  const DiscreteMesh& mesh = this->Model->GetMesh();
  mesh.BuildLinks();
  vtkNew<vtkModelItemListIterator> iter;
  iter->SetRoot(this->Model);
  iter->SetItemType(vtkModelFaceType);
  vtkDiscreteModelFace* face;
  FaceEdgeSplitInfo dummyInfo;
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    face = dynamic_cast<vtkDiscreteModelFace*>(iter->GetCurrentItem());
    if (!face)
    {
      vtkErrorMacro("Failed to get a discrete model face back!.");
      continue;
    }
    face->BuildEdges(this->ShowEdges != 0, dummyInfo, false);
  }

  // Add the new edge and vertex geometries to the wrapper, so that
  // they can be rendered in model mapper
  ModelWrapper->AddGeometricEntities(vtkModelEdgeType);
  ModelWrapper->AddGeometricEntities(vtkModelVertexType);

  vtkDebugMacro("Finished operating on a model.");
  this->OperateSucceeded = 1;
  return;
}

void vtkCreateModelEdgesOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
