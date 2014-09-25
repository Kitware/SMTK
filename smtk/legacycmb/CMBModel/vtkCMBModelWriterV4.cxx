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

#include "vtkCMBModelWriterV4.h"

#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkCMBParserBase.h"
#include "vtkModelUserName.h"
#include "vtkCMBModelWriterBase.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkIdList.h"
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
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelWriterV4);

vtkCMBModelWriterV4::vtkCMBModelWriterV4()
{
}

vtkCMBModelWriterV4:: ~vtkCMBModelWriterV4()
{
}

void vtkCMBModelWriterV4::SetModelVertexData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  vtkIdType NumberOfVertices = Model->GetNumberOfAssociations(vtkModelVertexType);
  std::vector<vtkModelEntity*> Entities(NumberOfVertices);
  vtkIdTypeArray* PointIdArray = vtkIdTypeArray::New();
  PointIdArray->SetNumberOfTuples(NumberOfVertices);
  vtkDoubleArray* LocationArray = vtkDoubleArray::New();
  LocationArray->SetNumberOfComponents(3);
  LocationArray->SetNumberOfTuples(NumberOfVertices);
  vtkModelItemIterator* Vertices = Model->NewIterator(vtkModelVertexType);
  vtkIdType Counter = 0;
  for(Vertices->Begin();!Vertices->IsAtEnd();Vertices->Next(),Counter++)
    {
    vtkDiscreteModelVertex* Vertex =
      vtkDiscreteModelVertex::SafeDownCast(Vertices->GetCurrentItem());
    PointIdArray->SetValue(Counter, Vertex->GetPointId());
    double xyz[3];
    if(Vertex->GetPoint(xyz))
      {
      LocationArray->SetTupleValue(Counter, xyz);
      }
    else
      {
      vtkWarningMacro("Model vertex does not have a valid location.");
      }
    Entities[Counter] = Vertex;
    }
  Vertices->Delete();

  PointIdArray->SetName(vtkCMBParserBase::GetVertexPointIdString());
  Poly->GetFieldData()->AddArray(PointIdArray);
  PointIdArray->Delete();

  LocationArray->SetName(vtkCMBParserBase::GetVertexLocationString());
  Poly->GetFieldData()->AddArray(LocationArray);
  LocationArray->Delete();
  this->SetModelEntityData(Poly, Entities, "ModelVertex");
}

void vtkCMBModelWriterV4::SetModelEdgeData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  vtkIdType NumberOfEdges = Model->GetNumberOfAssociations(vtkModelEdgeType);
  std::vector<vtkModelEntity*> Entities(NumberOfEdges);
  vtkIdTypeArray* PointIdArray = vtkIdTypeArray::New();
  PointIdArray->SetNumberOfComponents(2);
  PointIdArray->SetNumberOfTuples(NumberOfEdges);
  vtkIdTypeArray* EdgeRegionId = vtkIdTypeArray::New();
  EdgeRegionId->SetNumberOfComponents(1);
  EdgeRegionId->SetNumberOfTuples(NumberOfEdges);
  vtkIntArray* lineSpacing = vtkIntArray::New();
  lineSpacing->SetNumberOfComponents(1);
  lineSpacing->SetNumberOfTuples(NumberOfEdges);
  vtkModelItemIterator* Edges = Model->NewIterator(vtkModelEdgeType);

  vtkIdType Counter = 0;
  for(Edges->Begin();!Edges->IsAtEnd();Edges->Next(),Counter++)
    {
    vtkDiscreteModelEdge* Edge =
      vtkDiscreteModelEdge::SafeDownCast(Edges->GetCurrentItem());
    vtkIdType PointIds[2] = {-1, -1};
    for(int i=0;i<2;i++)
      {
      vtkModelVertex* Vertex = Edge->GetAdjacentModelVertex(i);
      if(Vertex)
        {
        PointIds[i] = Vertex->GetUniquePersistentId();
        }
      }
    PointIdArray->SetTupleValue(Counter, PointIds);
    if(Edge->GetModelRegion())
      {
      EdgeRegionId->SetValue(Counter, Edge->GetModelRegion()->GetUniquePersistentId());
      }
    else
      {
      EdgeRegionId->SetValue(Counter, -1);
      }
    lineSpacing->SetValue(Counter, Edge->GetLineResolution());
    Entities[Counter] = Edge;
    }
  Edges->Delete();

  PointIdArray->SetName(vtkCMBParserBase::GetModelEdgeVerticesString());
  Poly->GetFieldData()->AddArray(PointIdArray);
  PointIdArray->Delete();

  EdgeRegionId->SetName(vtkCMBParserBase::GetFloatingEdgesString());
  Poly->GetFieldData()->AddArray(EdgeRegionId);
  EdgeRegionId->Delete();

  lineSpacing->SetName(vtkCMBParserBase::GetEdgeLineResolutionString());
  Poly->GetFieldData()->AddArray(lineSpacing);
  lineSpacing->Delete();

  this->SetModelEntityData(Poly, Entities, "ModelEdge");
}

void vtkCMBModelWriterV4::SetModelFaceData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  vtkIdTypeArray* ModelFaceAdjacentRegionsId = vtkIdTypeArray::New();
  ModelFaceAdjacentRegionsId->SetNumberOfComponents(2);

  // the adjacent edge ids for each model face is separated by a -1
  vtkIdTypeArray* ModelFaceAdjacentEdgesId = vtkIdTypeArray::New();
  vtkIntArray* ModelEdgeDirections = vtkIntArray::New();

  vtkIdTypeArray* ModelFaceMaterialIds = vtkIdTypeArray::New();

  std::vector<vtkModelEntity*> Entities;
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

    vtkModelMaterial* Material = Face->GetMaterial();
    if(Material)
      {
      ModelFaceMaterialIds->InsertNextValue(Material->GetUniquePersistentId());
      }
    else
      {
      ModelFaceMaterialIds->InsertNextValue(-1);
      }
    //TODO: Do we have to encode loop uses???  Just deal with the outerloop
    vtkModelLoopUse* LoopUse = Face->GetModelFaceUse(0)->GetOuterLoopUse();
    if(LoopUse)
      {
      vtkModelItemIterator* EdgeUses = LoopUse->NewModelEdgeUseIterator();
      for(EdgeUses->Begin();!EdgeUses->IsAtEnd();EdgeUses->Next())
        {
        vtkModelEdgeUse* EdgeUse = vtkModelEdgeUse::SafeDownCast(EdgeUses->GetCurrentItem());
        ModelFaceAdjacentEdgesId->InsertNextValue(
          EdgeUse->GetModelEdge()->GetUniquePersistentId());
        ModelEdgeDirections->InsertNextValue(EdgeUse->GetDirection());
        }
      ModelFaceAdjacentEdgesId->InsertNextValue(-1);
      ModelEdgeDirections->InsertNextValue(1);
      EdgeUses->Delete();
      }
    }
  Faces->Delete();
  ModelFaceAdjacentRegionsId->SetName(vtkCMBParserBase::GetModelFaceRegionsString());
  Poly->GetFieldData()->AddArray(ModelFaceAdjacentRegionsId);
  ModelFaceAdjacentRegionsId->Delete();

  ModelFaceAdjacentEdgesId->SetName(vtkCMBParserBase::GetModelFaceEdgesString());
  Poly->GetFieldData()->AddArray(ModelFaceAdjacentEdgesId);
  ModelFaceAdjacentEdgesId->Delete();

  ModelEdgeDirections->SetName(vtkCMBParserBase::GetModelEdgeDirectionsString());
  Poly->GetFieldData()->AddArray(ModelEdgeDirections);
  ModelEdgeDirections->Delete();

  ModelFaceMaterialIds->SetName(vtkCMBParserBase::GetFaceMaterialIdString());
  Poly->GetFieldData()->AddArray(ModelFaceMaterialIds);
  ModelFaceMaterialIds->Delete();

  this->SetModelEntityData(Poly, Entities, "ModelFace");
}

void vtkCMBModelWriterV4::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
