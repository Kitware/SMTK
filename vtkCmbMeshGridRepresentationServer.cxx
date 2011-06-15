/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

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
#include "vtkCmbMeshGridRepresentationServer.h"

#include <iostream>
#include <string>
#include "vtkCMBModel.h"
#include "vtkCMBModelEdge.h"
#include "vtkCmbModelEntityMesh.h"
#include "vtkCMBModelEntityGroup.h"
#include "vtkCMBModelFace.h"
#include "vtkCMBModelGeometricEntity.h"
#include "vtkCMBNodalGroup.h"
#include "vtkCmbMeshServer.h"
#include "vtkModelItemIterator.h"

#include <vtkAppendPolyData.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCmbMeshGridRepresentationServer);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationServer, "");

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::vtkCmbMeshGridRepresentationServer()
{
  this->Model = NULL;
  this->MeshServer = NULL;
}

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationServer::~vtkCmbMeshGridRepresentationServer()
{
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetNodalGroupAnalysisGridPointIds(
  vtkCMBModel* model, vtkIdType nodalGroupId, vtkIdList* pointIds)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetFloatingEdgeAnalysisGridPointIds(
  vtkCMBModel* model, vtkIdType floatingEdgeId, vtkIdList* pointIds)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetModelEdgeAnalysisPoints(
  vtkCMBModel* model, vtkIdType boundaryGroupId, vtkIdTypeArray* edgePoints)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::GetBoundaryGroupAnalysisFacets(
  vtkCMBModel* model, vtkIdType boundaryGroupId,
  vtkIdList* cellIds, vtkIdList* cellSides)
{
  return false;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::IsModelConsistent(vtkCMBModel* model)
{
  return true;
}

//----------------------------------------------------------------------------
bool vtkCmbMeshGridRepresentationServer::Initialize(
  vtkCMBModel* model, vtkCmbMeshServer *meshServer)
{
  //instead of doing this logic now, instead store a weak pointer to the model and mesh server
  //and than call BuildRepresentation when we are asked about any information

  this->Model = model;
  this->MeshServer = meshServer;
}


bool vtkCmbMeshGridRepresentationServer::BuildRepresentation()
{
  //generate a single polydata that is the combintation of all the
  //face meshes
  std::vector<vtkPolyData*> faceMeshes;
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(this->Model->NewIterator(vtkModelFaceType));

  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelEntityMesh *faceEntityMesh = this->MeshServer->GetModelEntityMesh(face);
    if ( faceEntityMesh )
      {
      vtkPolyData *faceMesh = faceEntityMesh->GetModelEntityMesh();
      if ( faceMesh )
        {
        //append this mesh together
        faceMeshes.push_back(faceMesh);
        }
      }
    }

  if ( faceMeshes.size() == 0 )
    {
    return false;
    }

  //create the single polydata now

  return true;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
