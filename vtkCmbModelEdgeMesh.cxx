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
#include "vtkCmbModelEdgeMesh.h"

#include "vtkCmbMesh.h"
#include "vtkCmbModelVertexMesh.h"
#include <vtkModelEdge.h>
#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

vtkStandardNewMacro(vtkCmbModelEdgeMesh);
vtkCxxRevisionMacro(vtkCmbModelEdgeMesh, "");

//----------------------------------------------------------------------------
vtkCmbModelEdgeMesh::vtkCmbModelEdgeMesh()
{
  this->ModelEdge = NULL;
}

//----------------------------------------------------------------------------
vtkCmbModelEdgeMesh::~vtkCmbModelEdgeMesh()
{
}

//----------------------------------------------------------------------------
vtkModelGeometricEntity* vtkCmbModelEdgeMesh::GetModelGeometricEntity()
{
  return this->ModelEdge;
}

//----------------------------------------------------------------------------
void vtkCmbModelEdgeMesh::Initialize(vtkCmbMesh* masterMesh, vtkModelEdge* edge)
{
  if(masterMesh == NULL || edge == NULL)
    {
    vtkErrorMacro("Passed in masterMesh or edge is NULL.");
    return;
    }
  if(this->GetMasterMesh() != masterMesh)
    {
    this->SetMasterMesh(masterMesh);
    this->Modified();
    }
  if(this->ModelEdge != edge)
    {
    this->ModelEdge = edge;
    this->Modified();
    }
  this->BuildModelEntityMesh();
}

//----------------------------------------------------------------------------
bool vtkCmbModelEdgeMesh::BuildModelEntityMesh()
{
  if(!this->ModelEdge)
    {
    vtkErrorMacro("No model edge to build mesh from.");
    return false;
    }
  if(this->GetModelEntityMeshSize() <= 0.)
    {
    return false;
    }
  bool doBuild = false;
  if(this->GetModelEntityMesh() == NULL)
    {
    doBuild = true;
    }
  else if(this->GetModelEntityMesh()->GetMTime() < this->GetMTime())
    {
    doBuild = true; // the polydata is out of date
    }
  if(doBuild == false)
    {
    return false;
    }
  return this->BuildMesh();
}

//----------------------------------------------------------------------------
vtkCmbModelVertexMesh* vtkCmbModelEdgeMesh::GetAdjacentModelVertexMesh(
  int which)
{
  if(this->ModelEdge == NULL || this->GetMasterMesh() == NULL)
    {
    vtkErrorMacro("Must initialize before using object.");
    return NULL;
    }
  vtkModelVertex* modelVertex = this->ModelEdge->GetAdjacentModelVertex(which);
  return vtkCmbModelVertexMesh::SafeDownCast(
    this->GetMasterMesh()->GetModelEntityMesh(modelVertex));
}

//----------------------------------------------------------------------------
bool vtkCmbModelEdgeMesh::BuildMesh()
{
  vtkPolyData* mesh = this->GetModelEntityMesh();
  if(mesh)
    {
    mesh->Reset();
    }
  else
    {
    mesh = vtkPolyData::New();
    this->SetModelEntityMesh(mesh);
    mesh->Delete();
    }
  // not done properly -- just doing a straight line with 1 cell for now
  mesh->Initialize();
  mesh->Allocate();
  vtkPoints* points = vtkPoints::New();
  points->SetDataTypeToDouble();
  points->SetNumberOfPoints(2);
  if(vtkModelVertex* modelVertex = this->ModelEdge->GetAdjacentModelVertex(0))
    {
    double point[3];
    modelVertex->GetPoint(point);
    points->SetPoint(0, point);
    modelVertex = this->ModelEdge->GetAdjacentModelVertex(1);
    modelVertex->GetPoint(point);
    points->SetPoint(1, point);
    }
  else
    {  // put in something random now
    double point[3] = {0,0,0};
    points->SetPoint(0, point);
    point[2] = 1;
    points->SetPoint(1, point);
    }
  mesh->SetPoints(points);
  points->Delete();
  vtkIdType pointIds[2] = {0, 1};
  mesh->InsertNextCell(VTK_LINE, 2, pointIds);

  return true;
}

//----------------------------------------------------------------------------
void vtkCmbModelEdgeMesh::PrintSelf(ostream& os, vtkIndent indent)
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

