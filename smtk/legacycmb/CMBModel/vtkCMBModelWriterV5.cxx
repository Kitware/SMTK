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

#include "vtkCMBModelWriterV5.h"

#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkModel3dmGridRepresentation.h"
#include "vtkModel3dm2DGridRepresentation.h"
#include "vtkModelBCGridRepresentation.h"
//#include "vtkCMBMeshGridRepresentationServer.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkCMBParserBase.h"
#include "vtkFieldData.h"
#include <vtkIdList.h>
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include <vtkStringArray.h>

vtkStandardNewMacro(vtkCMBModelWriterV5);

vtkCMBModelWriterV5::vtkCMBModelWriterV5()
{
}

vtkCMBModelWriterV5:: ~vtkCMBModelWriterV5()
{
}

void vtkCMBModelWriterV5::SetModelEdgeData(vtkDiscreteModel* model, vtkPolyData* poly)
{
  // for 3D problems we only write out floating model edges to
  // some 3d models have non-floating edges in them which they shouldn't
  std::vector<vtkModelEntity*> entities;
  vtkModelItemIterator* edges = model->NewIterator(vtkModelEdgeType);
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
    if(edge->GetModelRegion() != NULL || model->GetModelDimension() == 2)
      {
      entities.push_back(edge);
      }
    }
  edges->Delete();
  vtkIdTypeArray* pointIdArray = vtkIdTypeArray::New();
  pointIdArray->SetNumberOfComponents(2);
  pointIdArray->SetNumberOfTuples(entities.size());
  vtkIdTypeArray* edgeRegionId = vtkIdTypeArray::New();
  edgeRegionId->SetNumberOfComponents(1);
  edgeRegionId->SetNumberOfTuples(entities.size());
  vtkIntArray* lineSpacing = vtkIntArray::New();
  lineSpacing->SetNumberOfComponents(1);
  lineSpacing->SetNumberOfTuples(entities.size());

  for(size_t counter=0;counter<entities.size();counter++)
    {
    vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(entities[counter]);
    vtkIdType pointIds[2] = {-1, -1};
    for(int i=0;i<2;i++)
      {
      if(vtkModelVertex* vertex = edge->GetAdjacentModelVertex(i))
        {
        pointIds[i] = vertex->GetUniquePersistentId();
        }
      }
    pointIdArray->SetTupleValue(counter, pointIds);
    if(edge->GetModelRegion())
      {
      edgeRegionId->SetValue(counter, edge->GetModelRegion()->GetUniquePersistentId());
      }
    else
      {
      edgeRegionId->SetValue(counter, -1);
      }
    lineSpacing->SetValue(counter, edge->GetLineResolution());
    }

  pointIdArray->SetName(vtkCMBParserBase::GetModelEdgeVerticesString());
  poly->GetFieldData()->AddArray(pointIdArray);
  pointIdArray->Delete();

  edgeRegionId->SetName(vtkCMBParserBase::GetFloatingEdgesString());
  poly->GetFieldData()->AddArray(edgeRegionId);
  edgeRegionId->Delete();

  lineSpacing->SetName(vtkCMBParserBase::GetEdgeLineResolutionString());
  poly->GetFieldData()->AddArray(lineSpacing);
  lineSpacing->Delete();

  this->SetModelEntityData(poly, entities, "ModelEdge");
}

void vtkCMBModelWriterV5::SetModelFaceData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  std::vector<vtkModelEntity*> Entities;
  if(Model->GetModelDimension() == 2)
    {
    // the adjacent edge ids for each model face is separated by a -1
    vtkIdTypeArray* ModelFaceAdjacentEdgesId = vtkIdTypeArray::New();
    vtkIntArray* ModelEdgeDirections = vtkIntArray::New();

    vtkIdTypeArray* ModelFaceMaterialIds = vtkIdTypeArray::New();

    vtkModelItemIterator* Faces = Model->NewIterator(vtkModelFaceType);
    for(Faces->Begin();!Faces->IsAtEnd();Faces->Next())
      {
      vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Faces->GetCurrentItem());
      Entities.push_back(Face);
      vtkModelMaterial* Material = Face->GetMaterial();
      ModelFaceMaterialIds->InsertNextValue(Material->GetUniquePersistentId());
      // Loop Information is encoded as follows: nL l0e0 l0e1 ... -1 l1e0 ...
      // Where nL is the number of loops in the face followed by the edges in
      // loop (for example l0e0 is edge 0 of loop 0) - with each loop separated by -1
      // The ModelEdgeDirections has the same number of tuples as ModelFaceAdjacentEdgesId
      // with -1 put in for nL and the -1 flag to indicate the end of a loop for
      // the corresponding entry in ModelfaceAdjacentEdgesId.  This is done as a
      // safeguard to make sure we're reading ModelEdgeDirections properly.
      vtkModelFaceUse* faceUse1 = Face->GetModelFaceUse(1);
      int nLoops = faceUse1->GetNumberOfLoopUses();
      ModelFaceAdjacentEdgesId->InsertNextValue(nLoops);
      ModelEdgeDirections->InsertNextValue(-1);
      if (nLoops != 0)
        {
        vtkModelItemIterator *liter = faceUse1->NewLoopUseIterator();
        vtkModelLoopUse* lu;
        for (liter->Begin(); !liter->IsAtEnd(); liter->Next())
          {
          lu = vtkModelLoopUse::SafeDownCast(liter->GetCurrentItem());
          vtkModelItemIterator* EdgeUses = lu->NewModelEdgeUseIterator();
          for(EdgeUses->Begin();!EdgeUses->IsAtEnd();EdgeUses->Next())
            {
            vtkModelEdgeUse* EdgeUse = vtkModelEdgeUse::SafeDownCast(EdgeUses->GetCurrentItem());
            ModelFaceAdjacentEdgesId->InsertNextValue(
              EdgeUse->GetModelEdge()->GetUniquePersistentId());
            ModelEdgeDirections->InsertNextValue(EdgeUse->GetDirection());
            }
          ModelFaceAdjacentEdgesId->InsertNextValue(-1);
          ModelEdgeDirections->InsertNextValue(-1);
          EdgeUses->Delete();
          }
        liter->Delete();
        }
      }
    Faces->Delete();

    ModelFaceAdjacentEdgesId->SetName(vtkCMBParserBase::GetModelFaceEdgesString());
    Poly->GetFieldData()->AddArray(ModelFaceAdjacentEdgesId);
    ModelFaceAdjacentEdgesId->Delete();

    ModelEdgeDirections->SetName(vtkCMBParserBase::GetModelEdgeDirectionsString());
    Poly->GetFieldData()->AddArray(ModelEdgeDirections);
    ModelEdgeDirections->Delete();

    ModelFaceMaterialIds->SetName(vtkCMBParserBase::GetFaceMaterialIdString());
    Poly->GetFieldData()->AddArray(ModelFaceMaterialIds);
    ModelFaceMaterialIds->Delete();
    }
  else if(Model->GetModelDimension() == 3)
    {
    vtkIdTypeArray* ModelFaceAdjacentRegionsId = vtkIdTypeArray::New();
    ModelFaceAdjacentRegionsId->SetNumberOfComponents(2);

    vtkModelItemIterator* Faces = Model->NewIterator(vtkModelFaceType);
    for(Faces->Begin();!Faces->IsAtEnd();Faces->Next())
      {
      vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Faces->GetCurrentItem());
      Entities.push_back(Face);
      vtkIdType ids[2] = {-1, -1};
      for(int j=0;j<2;j++)
        {
        vtkModelRegion* Region = Face->GetModelRegion(j);
        if(Region)
          {
          ids[j] = Region->GetUniquePersistentId();
          }
        }
      ModelFaceAdjacentRegionsId->InsertNextTupleValue(ids);
      }
    Faces->Delete();
    ModelFaceAdjacentRegionsId->SetName(vtkCMBParserBase::GetModelFaceRegionsString());
    Poly->GetFieldData()->AddArray(ModelFaceAdjacentRegionsId);
    ModelFaceAdjacentRegionsId->Delete();
    }

  this->SetModelEntityData(Poly, Entities, "ModelFace");
}

void vtkCMBModelWriterV5::SetAnalysisGridData(
  vtkDiscreteModel* model, vtkPolyData* poly)
{
  vtkModelGridRepresentation* gridRepresentation =
    model->GetAnalysisGridInfo();
  if(gridRepresentation == NULL ||
     gridRepresentation->IsModelConsistent(model) == false)
    {
    return;
    }
  if(const char*  analysisGridName =
     gridRepresentation->GetGridFileName())
    {
    vtkStringArray* gridName = vtkStringArray::New();
    gridName->SetName(vtkCMBParserBase::GetAnalysisGridFileName());
    gridName->InsertNextValue(analysisGridName);
    poly->GetFieldData()->AddArray(gridName);
    gridName->Delete();
    }

  if(vtkModel3dmGridRepresentation::SafeDownCast(gridRepresentation))
    {
    this->Superclass::SetAnalysisGridData(model, poly);
    return;
    }
  else if(vtkModel3dm2DGridRepresentation::SafeDownCast(gridRepresentation))
    {
    vtkStringArray* gridType = vtkStringArray::New();
    gridType->SetName(vtkCMBParserBase::GetAnalysisGridFileType());
    gridType->InsertNextValue("2D ADH");
    poly->GetFieldData()->AddArray(gridType);
    gridType->Delete();
    return;
    }
  else if(vtkModelBCGridRepresentation::SafeDownCast(gridRepresentation))
    {
    // the analysis grid info came from a bc file
    vtkDebugMacro("Currently the vtkModelBCGridRepresentation will not be saved with the model, "
      << " and it can be saved out separately in a m2m file");
    return;
    }
}

void vtkCMBModelWriterV5::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
