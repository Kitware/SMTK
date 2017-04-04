//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelFaceMeshServer.h"

#include "vtkCMBMesh.h"
#include <vtkModelEdge.h>
#include <vtkModelEdgeUse.h>
#include <vtkModelFace.h>
#include <vtkModelFaceUse.h>
#include <vtkModelItemIterator.h>
#include <vtkModelLoopUse.h>
#include <vtkModelUserName.h>
#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

#include "cmbFaceMeshHelper.h"
#include "cmbFaceMesherInterface.h"

vtkStandardNewMacro(vtkCMBModelFaceMeshServer);

vtkCMBModelFaceMeshServer::vtkCMBModelFaceMeshServer()
{
  this->ZValue = -1;
  this->FaceInfo = NULL;
  vtkPolyData* poly = vtkPolyData::New();
  this->SetModelEntityMesh(poly);
  poly->FastDelete();
}

vtkCMBModelFaceMeshServer::~vtkCMBModelFaceMeshServer()
{
  if (this->FaceInfo)
    {
    delete this->FaceInfo;
    this->FaceInfo = NULL;
    }
}

bool vtkCMBModelFaceMeshServer::SetLocalLength(double length)
{
  if(length == this->GetLength())
    {
    return true;
    }
  this->SetLength(length);
  return true;
}

bool vtkCMBModelFaceMeshServer::SetLocalMinimumAngle(double minAngle)
{
  if(minAngle == this->GetMinimumAngle())
    {
    return true;
    }
  this->SetMinimumAngle(minAngle);
  return true;
}

bool vtkCMBModelFaceMeshServer::BuildMesh(bool /*meshHigherDimensionalEntities*/)
{
  this->FaceMesherFailed = 0;

  double length = this->GetActualLength();
  double angle = this->GetActualMinimumAngle();
  if(length <= 0. || angle <= 0.)
    {
    this->SetModelEntityMesh(NULL);
    this->SetMeshedLength(0.);
    this->SetMeshedMinimumAngle(0.);
    return true;
    }
  if (this->FaceInfo)
    {
    delete this->FaceInfo;
    this->FaceInfo = NULL;
    }
  this->FaceInfo = new CmbFaceMesherClasses::ModelFaceRep();

  bool valid = this->CreateMeshInfo();
  vtkPolyData* mesh = vtkPolyData::New();
  valid = valid && this->Triangulate(mesh,length, angle);

  // it would seem like we could just do this->SetModelEntityMesh(mesh);
  // but we can't.  i think this has to do with the polydataprovider.
  vtkPolyData* faceMesh = this->GetModelEntityMesh();
  if(!faceMesh)
    {
    faceMesh = vtkPolyData::New();
    this->SetModelEntityMesh(faceMesh);
    faceMesh->FastDelete();
    }


  faceMesh->ShallowCopy(mesh);
  mesh->Delete();
  if(!valid && (faceMesh->GetNumberOfCells() == 0 ||
                faceMesh->GetNumberOfPoints() == 0) )
    {
    //we can presume at this point the face mesher didn't work properly
    this->FaceMesherFailed = 1;
    }

  this->SetMeshedLength(length);
  this->SetMeshedMinimumAngle(angle);
  return valid;
}

bool vtkCMBModelFaceMeshServer::CreateMeshInfo()
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
    CmbFaceMesherClasses::ModelLoopRep loop(loopId,loopId != START_LOOP_ID);
    vtkModelItemIterator* edgeUses = vtkModelLoopUse::SafeDownCast(
        liter->GetCurrentItem())->NewModelEdgeUseIterator();
    for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
      {
      //add each edge to the loop
      vtkModelEdge* modelEdge = vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem())->GetModelEdge();

      //we now have to walk the mesh lines cell data to get all the verts for this edge
      vtkIdType edgeId =modelEdge->GetUniquePersistentId();
      vtkPolyData *mesh = this->GetMasterMesh()->
        GetModelEntityMesh(modelEdge)->GetModelEntityMesh();
      if(!mesh)
        {
        vtkErrorMacro("Missing mesh.");
        }

      CmbFaceMesherClasses::ModelEdgeRep edge(edgeId);
      edge.setMeshPoints(mesh);
      int numVerts = modelEdge->GetNumberOfModelVertexUses();
      for(int i=0;i<numVerts;++i)
        {
        vtkModelVertex* vertex = modelEdge->GetAdjacentModelVertex(i);
        if(vertex)
          {
          double point[3];
          vertex->GetPoint(point);
          edge.addModelVert(vertex->GetUniquePersistentId(),point);
          }
        }
      loop.addEdge(edge);
      }
    edgeUses->Delete();
    this->FaceInfo->addLoop(loop);
    }
  if ( liter )
    {
    liter->Delete();
    }

  this->DetermineZValueOfFace();
  return true;
}

bool vtkCMBModelFaceMeshServer::Triangulate(vtkPolyData *mesh,
                                            double length, double angle)
{
  //The current plan is that we are going to redo the entire storage of the
  //loop and face data. We will go to a light information object ( num holes,segs,points)
  //create the mesher interface with that information.
  //Than we will pass back to the Internal Face the pointers to the memory structs,
  //copy all the info directly into those pointers, and use those to calculate out the
  //bounds, hole inside etc.
  //we now get to construct the triangulate structs based on our mapping
  int numPoints = this->FaceInfo->numberOfVertices();
  int numSegs = this->FaceInfo->numberOfEdges();
  int numHoles = this->FaceInfo->numberOfHoles();
  cmbFaceMesherInterface ti(numPoints,numSegs,numHoles);

  double maxArea = 0.5 * length * length;
  ti.setUseMaxArea(true);
  ti.setMaxArea(maxArea);

  ti.setUseMinAngle(true);
  ti.setMinAngle(angle);

  ti.setOutputMesh(mesh);

  this->FaceInfo->fillTriangleInterface(&ti);
  vtkModelFace* modelFace =
    vtkModelFace::SafeDownCast(this->GetModelGeometricEntity());
  bool valid = ti.buildFaceMesh(static_cast<long>(modelFace->GetUniquePersistentId()), this->ZValue);
  if ( valid )
    {
    valid = this->FaceInfo->RelateMeshToModel(mesh,modelFace->GetUniquePersistentId());
    }
  return valid;
}

void vtkCMBModelFaceMeshServer::DetermineZValueOfFace()
{
  vtkModelFace* modelFace = vtkModelFace::SafeDownCast(this->GetModelGeometricEntity());
  vtkModelItemIterator *liter = modelFace->GetModelFaceUse(0)->NewLoopUseIterator();
  liter->Begin();

  vtkModelItemIterator* edgeUses = vtkModelLoopUse::SafeDownCast(
  liter->GetCurrentItem())->NewModelEdgeUseIterator();

  edgeUses->Begin();

  //get the first model edge
  vtkModelEdge* modelEdge = vtkModelEdgeUse::SafeDownCast(
    edgeUses->GetCurrentItem())->GetModelEdge();
  vtkModelVertex* vertex = modelEdge->GetAdjacentModelVertex(0);
  if(vertex)
    {
    double point[3];
    vertex->GetPoint(point);
    this->ZValue = point[2];
    }

  edgeUses->Delete();
  if ( liter )
    {
    liter->Delete();
    }
}

void vtkCMBModelFaceMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
