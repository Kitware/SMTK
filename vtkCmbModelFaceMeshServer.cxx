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
}

//----------------------------------------------------------------------------
vtkCmbModelFaceMeshServer::~vtkCmbModelFaceMeshServer()
{
}

//----------------------------------------------------------------------------
bool vtkCmbModelFaceMeshServer::BuildMesh(bool meshHigherDimensionalEntities)
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

  // rob -- put in the calls to triangle here

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

  double global = this->GetMasterMesh()->GetGlobalMaximumArea();
  double local = this->GetMaximumArea();
  if ( local == 0 || global < local )
    {
    local = global;
    }
  ti.setUseMaxArea(true);
  ti.setMaxArea(local);

  global = this->GetMasterMesh()->GetGlobalMinimumAngle();
  local = this->GetMinimumAngle();
  if ( local == 0 || global < local )
    {
    local = global;
    }
  ti.setUseMinAngle(true);
  ti.setMinAngle(local);

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
