//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelEdgeMesh.h"

#include "vtkCMBMesh.h"
#include "vtkCMBModelVertexMesh.h"
#include <vtkCMBModelFaceMesh.h>
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>
#include <vtkPolyData.h>

vtkCMBModelEdgeMesh::vtkCMBModelEdgeMesh()
{
  this->ModelEdge = NULL;
}

vtkCMBModelEdgeMesh::~vtkCMBModelEdgeMesh()
{
}

vtkModelGeometricEntity* vtkCMBModelEdgeMesh::GetModelGeometricEntity()
{
  return this->ModelEdge;
}

void vtkCMBModelEdgeMesh::Initialize(vtkCMBMesh* masterMesh, vtkModelEdge* edge)
{
  if(masterMesh == NULL || edge == NULL)
    {
    vtkErrorMacro("Passed in masterMesh or edge is NULL.");
    return;
    }
  this->SetMasterMesh(masterMesh);
  this->ModelEdge = edge;
  this->SetLength(0);
  this->SetMeshedLength(0);
  this->SetModelEntityMesh(NULL);
}

bool vtkCMBModelEdgeMesh::BuildModelEntityMesh(
  bool meshHigherDimensionalEntities)
{
  double length =  this->GetActualLength();
  if(!this->ModelEdge)
    {
    return false;
    }
  bool doBuild = false;
  if(this->GetModelEntityMesh() == NULL && length > 0.)
    {
    doBuild = true;
    }
  else if(this->GetModelEntityMesh() == NULL && length == 0)
    {
    doBuild = false;
    }
  else if(this->GetMeshedLength() != length)
    {
    doBuild = true;
    }
  else if( vtkPolyData* modelGeometry =
           vtkPolyData::SafeDownCast(this->ModelEdge->GetGeometry()) )
    {
    if(modelGeometry->GetMTime() >= this->GetModelEntityMesh()->GetMTime())
      { // if the model poly is newer than the mesh poly we need to remesh
      doBuild = true;
      }
    }
  if(doBuild == false)
    {
    return true;
    }
  return this->BuildMesh(meshHigherDimensionalEntities);
}

vtkCMBModelVertexMesh* vtkCMBModelEdgeMesh::GetAdjacentModelVertexMesh(
  int which)
{
  if(this->ModelEdge == NULL || this->GetMasterMesh() == NULL)
    {
    vtkErrorMacro("Must initialize before using object.");
    return NULL;
    }
  vtkModelVertex* modelVertex = this->ModelEdge->GetAdjacentModelVertex(which);
  return vtkCMBModelVertexMesh::SafeDownCast(
    this->GetMasterMesh()->GetModelEntityMesh(modelVertex));
}

double vtkCMBModelEdgeMesh::GetActualLength()
{
  double actualLength = this->GetLength();

  // Now we need to combine this with the faces of the model edge
  vtkModelFace *face;
  vtkCMBModelFaceMesh *faceMesh;
  bool hadFaces = false;
  vtkModelItemIterator *faces = this->ModelEdge->NewAdjacentModelFaceIterator();
  for(faces->Begin(); !faces->IsAtEnd(); faces->Next())
    {
    face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    faceMesh =
      vtkCMBModelFaceMesh::SafeDownCast(
                                        this->GetMasterMesh()->
                                        GetModelEntityMesh(face));
    actualLength =
      vtkCMBMesh::CombineMeshLengths(actualLength,
                                     faceMesh->GetActualLength());
    hadFaces = true;
    }
  faces->Delete();
  if (!hadFaces)
    {
    // We need to combine the actual length with global setting.
    // If the edge had faces this would have been done in the above loop
    // (several times infact)
    actualLength =
      vtkCMBMesh::CombineMeshLengths(actualLength,
                                     this->GetMasterMesh()->GetGlobalLength());
    }
  return actualLength;
}

void vtkCMBModelEdgeMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->ModelEdge)
    {
    os << indent << "ModelEdge: " << this->ModelEdge << "\n";
    }
  else
    {
    os << indent << "ModelEdge: (NULL)\n";
    }
}
