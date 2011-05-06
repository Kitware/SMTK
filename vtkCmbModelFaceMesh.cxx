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
#include "vtkCmbModelEdgeMesh.h"

#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include "CmbFaceMeshHelper.h"
#include "CmbFaceMesherInterface.h"
using namespace CmbModelFaceMeshPrivate;

vtkStandardNewMacro(vtkCmbModelFaceMesh);
vtkCxxRevisionMacro(vtkCmbModelFaceMesh, "");

//----------------------------------------------------------------------------
vtkCmbModelFaceMesh::vtkCmbModelFaceMesh()
{
  this->ModelFace = NULL;
  this->MinimumAngle = 0.;
  this->MeshedMinimumAngle = 0.;
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

  // check if all the edges are meshed first
  vtkModelItemIterator* edges = this->ModelFace->NewAdjacentModelEdgeIterator();
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelGeometricEntity* edge =
      vtkModelGeometricEntity::SafeDownCast(edges->GetCurrentItem());
    vtkCmbModelEdgeMesh* edgeMesh = vtkCmbModelEdgeMesh::SafeDownCast(
      this->GetMasterMesh()->GetModelEntityMesh(edge));
    if(edgeMesh->GetMeshedLength() <= 0.)
      {
      edges->Delete();
      this->SetModelEntityMesh(NULL);
      this->SetMeshedLength(0.);
      this->SetMeshedMinimumAngle(0.);
      return true;
      }
    }
  edges->Delete();
  bool doBuild = false;
  if(this->GetModelEntityMesh() == NULL && this->GetActualLength() > 0. &&
     this->GetActualMinimumAngle() > 0.)
    {
    doBuild = true;
    }
  else if(this->GetActualLength() != this->GetMeshedLength() ||
          this->GetActualMinimumAngle() != this->GetMeshedMinimumAngle())
    {
    doBuild = true;
    }
  else if( vtkPolyData* modelGeometry =
           vtkPolyData::SafeDownCast(this->ModelFace->GetGeometry()) )
    {
    if(this->GetModelEntityMesh() == NULL &&
       (this->GetActualLength() == 0. || this->GetActualMinimumAngle() == 0.) )
      {
      return true;
      }
    if(modelGeometry->GetMTime() >= this->GetModelEntityMesh()->GetMTime())
      { // if the model poly is newer than the mesh poly we need to remesh
      doBuild = true;
      }
    else
      { // check if any of the edge mesh poly's are newer than the face mesh poly
      edges = this->ModelFace->NewAdjacentModelEdgeIterator();
      for(edges->Begin();!edges->IsAtEnd();edges->Next())
        {
        vtkModelGeometricEntity* edge =
          vtkModelGeometricEntity::SafeDownCast(edges->GetCurrentItem());
        vtkCmbModelEdgeMesh* edgeMesh = vtkCmbModelEdgeMesh::SafeDownCast(
          this->GetMasterMesh()->GetModelEntityMesh(edge));
        if(edgeMesh->GetModelEntityMesh()->GetMTime() >
           this->GetModelEntityMesh()->GetMTime())
          {
          doBuild = true;
          break;
          }
        }
      edges->Delete();
      }
    }
  if(doBuild == false)
    {
    return true;
    }
  return this->BuildMesh(meshHigherDimensionalEntities);
}

//----------------------------------------------------------------------------
<<<<<<< HEAD
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

  if (this->FaceInfo)
    {
    delete this->FaceInfo;
    this->FaceInfo = NULL;
    }
  this->FaceInfo = new CmbModelFaceMeshPrivate::InternalFace();

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
    this->FaceInfo->addLoop(loop);
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
  //The current plan is that we are going to redo the entire storage of the
  //loop and face data. We will go to a light information object ( num holes,segs,points)
  //create the mesher interface with that information.
  //Than we will pass back to the Internal Face the pointers to the memory structs,
  //copy all the info directly into those pointers, and use those to calculate out the
  //bounds, hole inside etc.
  //we now get to construct the triangulate structs based on our mapping
  int numPoints = this->FaceInfo->numberOfPoints();
  int numSegs = this->FaceInfo->numberOfLineSegments();
  int numHoles = this->FaceInfo->numberOfHoles();
  CmbFaceMesherInterface ti(numPoints,numSegs,numHoles);

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

  this->FaceInfo->fillTriangleInterface(&ti);
  bool valid = ti.buildFaceMesh((long)this->ModelFace->GetUniquePersistentId());
  if ( valid )
    {
    valid = this->FaceInfo->RelateMeshToModel(mesh,this->ModelFace->GetUniquePersistentId());
    }
  return valid;
}

//----------------------------------------------------------------------------
double vtkCmbModelFaceMesh::GetActualLength()
{
  double actualLength =
    vtkCmbMesh::CombineMeshLengths(this->GetLength(),
                                   this->GetMasterMesh()->GetGlobalLength());
  return actualLength;
}

//----------------------------------------------------------------------------
double vtkCmbModelFaceMesh::GetActualMinimumAngle()
{
  double actualMinAngle =
    vtkCmbMesh::CombineMeshMinimumAngles(this->MinimumAngle,
                                         this->GetMasterMesh()->
                                         GetGlobalMinimumAngle());
  return actualMinAngle;
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
  os << indent << "MinimumAngle: " << this->MinimumAngle << "\n";
  os << indent << "MeshedMinimumAngle: " << this->MeshedMinimumAngle << "\n";
}
