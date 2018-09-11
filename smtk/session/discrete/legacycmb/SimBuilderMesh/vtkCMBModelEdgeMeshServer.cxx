//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelEdgeMeshServer.h"

#include "vtkCMBMesh.h"
#include "vtkCMBModelFaceMesh.h"
#include "vtkCMBModelVertexMesh.h"
#include "vtkCleanPolylines.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkMeshModelEdgesFilter.h"
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkModelUserName.h>
#include <vtkModelVertex.h>

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkCMBModelEdgeMeshServer);

vtkCMBModelEdgeMeshServer::vtkCMBModelEdgeMeshServer()
{
  vtkPolyData* poly = vtkPolyData::New();
  this->UseLengthAlongEdge = true;
  this->SetModelEntityMesh(poly);
  poly->FastDelete();
}

vtkCMBModelEdgeMeshServer::~vtkCMBModelEdgeMeshServer()
{
}

bool vtkCMBModelEdgeMeshServer::SetLocalLength(double length)
{
  if (length == this->GetLength())
  {
    return true;
  }
  this->SetLength(length);
  return true;
}

bool vtkCMBModelEdgeMeshServer::BuildMesh(bool meshHigherDimensionalEntities)
{
  this->SetMeshedLength(this->GetActualLength());
  if (this->GetActualLength() <= 0.)
  {
    this->SetModelEntityMesh(NULL);
    // also have to delete mesh of adjacent model faces
    vtkModelItemIterator* faces = this->GetModelEdge()->NewAdjacentModelFaceIterator();
    bool returnValue = true;
    for (faces->Begin(); !faces->IsAtEnd(); faces->Next())
    {
      vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      vtkCMBModelFaceMesh* faceMesh =
        vtkCMBModelFaceMesh::SafeDownCast(this->GetMasterMesh()->GetModelEntityMesh(face));
      returnValue = returnValue && faceMesh->BuildModelEntityMesh(true);
    }
    faces->Delete();
    return true;
  }

  // Clean polylines will create a "full strip", whereas vtkStripper
  // starts building a strip from a line and adds lines to only one end,
  // such that it is possible for (what could be) a single polyline to end
  // up as several (perhaps MANY) polylines.  Clean polylines also deals with
  // non-manifold points, but at this point, not expectign that to be an issue.
  vtkNew<vtkCleanPolylines> stripper;
  stripper->SetMinimumLineLength(0);
  stripper->UseRelativeLineLengthOff();
  stripper->SetInputData(vtkPolyData::SafeDownCast(
    vtkDiscreteModelEdge::SafeDownCast(this->GetModelEdge())->GetGeometry()));
  stripper->Update();

  if (stripper->GetOutput()->GetNumberOfLines() != 1)
  {
    vtkErrorMacro("Expecting one and exactly one edge / polyline: "
      << stripper->GetOutput()->GetNumberOfLines());
    return false;
  }

  vtkNew<vtkDoubleArray> targetCellLength;
  targetCellLength->SetNumberOfComponents(1);
  targetCellLength->SetNumberOfTuples(1);
  double length = this->GetActualLength();

  targetCellLength->SetValue(0, length);
  targetCellLength->SetName("TargetSegmentLength");

  vtkNew<vtkPolyData> polyLinePD;
  polyLinePD->ShallowCopy(stripper->GetOutput());
  polyLinePD->GetCellData()->AddArray(targetCellLength.GetPointer());

  vtkNew<vtkMeshModelEdgesFilter> meshEdgesFilter;
  meshEdgesFilter->SetUseLengthAlongEdge(this->UseLengthAlongEdge);
  meshEdgesFilter->SetInputData(polyLinePD.GetPointer());
  meshEdgesFilter->SetTargetSegmentLengthCellArrayName(targetCellLength->GetName());
  meshEdgesFilter->Update();

  // it would seem like we could just do this->SetModelEntityMesh(mesh);
  // but we can't.  i think this has to do with the polydataprovider.
  vtkPolyData* mesh = this->GetModelEntityMesh();
  if (!mesh)
  {
    mesh = vtkPolyData::New();
    this->SetModelEntityMesh(mesh);
    mesh->FastDelete();
  }
  mesh->ShallowCopy(meshEdgesFilter->GetOutput());

  bool returnValue = true;
  // now we go and remesh any adjacent model face meshes that exist
  if (meshHigherDimensionalEntities)
  {
    vtkModelItemIterator* faces = this->GetModelEdge()->NewAdjacentModelFaceIterator();
    for (faces->Begin(); !faces->IsAtEnd(); faces->Next())
    {
      vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      vtkCMBModelFaceMesh* faceMesh =
        vtkCMBModelFaceMesh::SafeDownCast(this->GetMasterMesh()->GetModelEntityMesh(face));
      returnValue = returnValue && faceMesh->BuildModelEntityMesh(true);
    }
    faces->Delete();
  }

  return returnValue;
}

void vtkCMBModelEdgeMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
