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
INCLUDING,M
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkCmbModelFaceMesh.h"
#include "vtkCmbMesh.h"

#include <vtkModelFace.h>
#include <vtkModelEdgeUse.h>
#include <vtkModelEdge.h>
#include <vtkModelItemIterator.h>
#include <vtkModelFaceUse.h>
#include <vtkModelLoopUse.h>
#include <vtkModelVertex.h>

#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

#include "CmbFaceMeshHelper.h"
#include "CmbTriangleInterface.h"
using namespace CmbModelFaceMeshPrivate;

vtkStandardNewMacro(vtkCmbModelFaceMesh);
vtkCxxRevisionMacro(vtkCmbModelFaceMesh, "");

//----------------------------------------------------------------------------
vtkCmbModelFaceMesh::vtkCmbModelFaceMesh()
{
  this->ModelFace = NULL;
  this->MeshInfo = NULL;
  this->MaximumArea = 0.;
  this->MinimumAngle = 0.;
}

//----------------------------------------------------------------------------
vtkCmbModelFaceMesh::~vtkCmbModelFaceMesh()
{
}

//----------------------------------------------------------------------------
vtkModelGeometricEntity* vtkCmbModelFaceMesh::GetModelGeometricEntity()
{
  return this->ModelFace;
}

//----------------------------------------------------------------------------
void vtkCmbModelFaceMesh::Initialize(vtkCmbMesh* masterMesh, vtkModelFace* face)
{
  if(masterMesh == NULL || face == NULL)
    {
    vtkErrorMacro("Passed in masterMesh or face is NULL.");
    return;
    }
  if(this->GetMasterMesh() != masterMesh)
    {
    this->SetMasterMesh(masterMesh);
    this->Modified();
    }
  if(this->ModelFace != face)
    {
    this->ModelFace = face;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMesh::BuildModelEntityMesh(
  bool meshHigherDimensionalEntities)
{
  if(!this->ModelFace)
    {
    return false;
    }
  if( (this->MaximumArea <= 0. && this->GetMasterMesh()->GetGlobalMaximumArea() <= 0.) ||
      (this->MinimumAngle <= 0. && this->GetMasterMesh()->GetGlobalMinimumAngle() <= 0.) )
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
  return this->BuildMesh(meshHigherDimensionalEntities);
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMesh::BuildMesh(bool meshHigherDimensionalEntities)
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
  mesh->Initialize();
  mesh->Allocate();

  if (this->MeshInfo)
    {
    delete this->MeshInfo;
    this->MeshInfo = NULL;
    }
  this->MeshInfo = new CmbModelFaceMeshPrivate::MeshInformation();

  bool valid = true;
  valid = valid && this->CreateMeshInfo();
  valid = valid && this->Triangulate(mesh);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMesh::CreateMeshInfo()
{
  //we need to walk the topology once to determine the following:
  // number of loops that are holes
  // number of total line segements across all loops
  // number of unique points used in all line segments
  // we need all this information so we can properly construct the triangle memory
  vtkModelItemIterator *liter = this->ModelFace->GetModelFaceUse(0)->NewLoopUseIterator();
  const int START_LOOP_ID = 1;
  vtkIdType loopId = START_LOOP_ID;
  for (liter->Begin();!liter->IsAtEnd(); liter->Next(), ++loopId)
    {
    //by design the first loop is the external loop, all other loops are internal loops
    //for a loop to be a hole it has to be an internal loop, with an edge that isn't used twice
    InternalLoop loop(loopId,loopId != START_LOOP_ID);
    vtkModelItemIterator* edgeUses = vtkModelLoopUse::SafeDownCast(
        liter->GetCurrentItem())->NewModelEdgeUseIterator();
    for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
      {
      //add each edge to the loop
      vtkModelEdge* modelEdge = vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem())->GetModelEdge();
      //we now have to walk the mesh lines cell data to get all the verts for this edge
      vtkIdType edgeId =modelEdge->GetUniquePersistentId();
      if (!loop.edgeExists(edgeId))
        {
        vtkPolyData *mesh = this->GetMasterMesh()->
          GetModelEntityMesh(modelEdge)->GetModelEntityMesh();

        InternalEdge edge(edgeId);
        edge.setMeshPoints(mesh);
        int numVerts = modelEdge->GetNumberOfModelVertexUses();
        for(int i=0;i<numVerts;++i)
          {
          vtkModelVertex* vertex = modelEdge->GetAdjacentModelVertex(i);
          if(vertex)
            {
            //vertex->GetPoint(modelVert);
            edge.addModelVert(vertex->GetUniquePersistentId());
            }
          }
        loop.addEdge(edge);
        }
      else
        {
        loop.markEdgeAsDuplicate(edgeId);
        }
      }
    edgeUses->Delete();
    this->MeshInfo->addLoop(loop);
    }
  if ( liter )
    {
    liter->Delete();
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMesh::Triangulate(vtkPolyData *mesh)
{
  //we now get to construct the triangulate structs based on our mapping
  int numPoints = this->MeshInfo->numberOfPoints();
  int numSegs = this->MeshInfo->numberOfLineSegments();
  int numHoles = this->MeshInfo->numberOfHoles();
  CmbTriangleInterface ti(numPoints,numSegs,numHoles);

  double global = this->GetMasterMesh()->GetGlobalMaximumArea();
  double local = this->MaximumArea;
  if ( local == 0 || global < local )
    {
    local = global;
    }
  ti.setUseMaxArea(true);
  ti.setMaxArea(local);

  global = this->GetMasterMesh()->GetGlobalMinimumAngle();
  local = this->MinimumAngle;
  if ( local == 0 || global < local )
    {
    local = global;
    }
  ti.setUseMinAngle(true);
  ti.setMinAngle(local);

  ti.setOutputMesh(mesh);

  this->MeshInfo->fillTriangleInterface(&ti);
  bool valid = ti.buildFaceMesh();
  return valid;
}

//----------------------------------------------------------------------------
void vtkCmbModelFaceMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->ModelFace)
    {
    os << indent << "ModelFace: " << this->ModelFace << "\n";
    }
  else
    {
    os << indent << "ModelFace: (NULL)\n";
    }
  os << indent << "MaximumArea: " << this->MaximumArea << "\n";
  os << indent << "MinimumAngle: " << this->MinimumAngle << "\n";
}
