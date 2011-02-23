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
#include "vtkCmbModelFaceMeshServer.h"

#include "vtkCmbMesh.h"
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

vtkStandardNewMacro(vtkCmbModelFaceMeshServer);
vtkCxxRevisionMacro(vtkCmbModelFaceMeshServer, "");

//----------------------------------------------------------------------------
vtkCmbModelFaceMeshServer::vtkCmbModelFaceMeshServer()
{
  this->FaceInfo = NULL;
  vtkPolyData* poly = vtkPolyData::New();
  this->SetModelEntityMesh(poly);
  poly->FastDelete();
}

//----------------------------------------------------------------------------
vtkCmbModelFaceMeshServer::~vtkCmbModelFaceMeshServer()
{
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshServer::BuildMesh(bool meshHigherDimensionalEntities)
{
  if(this->GetActualMaximumArea() <= 0. ||
     this->GetActualMinimumAngle() <= 0.)
    {
    this->SetModelEntityMesh(NULL);
    this->SetMeshedMaximumArea(0.);
    this->SetMeshedMinimumAngle(0.);
    return false;
    }
  vtkPolyData* mesh = this->GetModelEntityMesh();
  if(mesh)
    {
    mesh->Reset();
    }
  else
    {
    mesh = vtkPolyData::New();
    this->SetModelEntityMesh(mesh);
    mesh->FastDelete();
    }

  if (this->FaceInfo)
    {
    delete this->FaceInfo;
    this->FaceInfo = NULL;
    }
  this->FaceInfo = new CmbModelFaceMeshPrivate::InternalFace();

  bool valid = true;
  valid = valid && this->CreateMeshInfo();
  valid = valid && this->Triangulate(mesh);
  this->SetMeshedMaximumArea(this->GetActualMaximumArea());
  this->SetMeshedMinimumAngle(this->GetActualMinimumAngle());
  return valid;
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshServer::CreateMeshInfo()
{
  //we need to walk the topology once to determine the following:
  // number of loops that are holes
  // number of total line segements across all loops
  // number of unique points used in all line segments
  // we need all this information so we can properly construct the triangle memory
  vtkModelFace* modelFace = vtkModelFace::SafeDownCast(this->GetModelGeometricEntity());
  vtkModelItemIterator *liter = modelFace->GetModelFaceUse(0)->NewLoopUseIterator();
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
        if(!mesh)
          {
          vtkErrorMacro("stupid missing mesh.");
          }

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
bool vtkCmbModelFaceMeshServer::Triangulate(vtkPolyData *mesh)
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

  ti.setUseMaxArea(true);
  ti.setMaxArea(this->GetActualMaximumArea());

  ti.setUseMinAngle(true);
  ti.setMinAngle(this->GetActualMinimumAngle());

  ti.setOutputMesh(mesh);

  this->FaceInfo->fillTriangleInterface(&ti);
  vtkModelFace* modelFace =
    vtkModelFace::SafeDownCast(this->GetModelGeometricEntity());
  bool valid = ti.buildFaceMesh((long)modelFace->GetUniquePersistentId());
  if ( valid )
    {
    valid = this->FaceInfo->RelateMeshToModel(mesh,modelFace->GetUniquePersistentId());
    }
  return valid;
}

//----------------------------------------------------------------------------
void vtkCmbModelFaceMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
