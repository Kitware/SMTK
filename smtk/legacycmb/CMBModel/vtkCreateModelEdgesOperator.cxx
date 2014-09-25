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

#include "vtkCreateModelEdgesOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItemListIterator.h"
#include "vtkNew.h"
#include "ModelEdgeHelper.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCreateModelEdgesOperator);

vtkCreateModelEdgesOperator::vtkCreateModelEdgesOperator()
{
  this->OperateSucceeded = 0;
  this->ShowEdges = 0;
}

//----------------------------------------------------------------------------
vtkCreateModelEdgesOperator::~vtkCreateModelEdgesOperator()
{
}

//----------------------------------------------------------------------------
bool vtkCreateModelEdgesOperator::AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return this->Superclass::AbleToOperate(ModelWrapper->GetModel());
}

//----------------------------------------------------------------------------
void vtkCreateModelEdgesOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  vtkDebugMacro("Operating on a model.");

  if(!this->AbleToOperate(ModelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  this->Model = ModelWrapper->GetModel();
  // Need to build links to get facet neighborhoods
  const DiscreteMesh &mesh = this->Model->GetMesh();
  mesh.BuildLinks();
  vtkNew<vtkModelItemListIterator> iter;
  iter->SetRoot(this->Model);
  iter->SetItemType(vtkModelFaceType);
  vtkDiscreteModelFace* face;
  FaceEdgeSplitInfo dummyInfo;
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
    face = dynamic_cast<vtkDiscreteModelFace *>(iter->GetCurrentItem());
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

//----------------------------------------------------------------------------
void vtkCreateModelEdgesOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
