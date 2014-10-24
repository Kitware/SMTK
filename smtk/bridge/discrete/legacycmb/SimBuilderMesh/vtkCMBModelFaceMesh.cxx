//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelFaceMesh.h"

#include "vtkCMBMesh.h"
#include "vtkCMBModelEdgeMesh.h"

#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>


//----------------------------------------------------------------------------
vtkCMBModelFaceMesh::vtkCMBModelFaceMesh()
{
  this->ModelFace = NULL;
  this->MinimumAngle = 0.;
  this->MeshedMinimumAngle = 0.;
}

//----------------------------------------------------------------------------
vtkCMBModelFaceMesh::~vtkCMBModelFaceMesh()
{
}

//----------------------------------------------------------------------------
vtkModelGeometricEntity* vtkCMBModelFaceMesh::GetModelGeometricEntity()
{
  return this->ModelFace;
}

//----------------------------------------------------------------------------
void vtkCMBModelFaceMesh::Initialize(vtkCMBMesh* masterMesh, vtkModelFace* face)
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
bool vtkCMBModelFaceMesh::BuildModelEntityMesh(
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
    vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
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
        vtkCMBModelEdgeMesh* edgeMesh = vtkCMBModelEdgeMesh::SafeDownCast(
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
double vtkCMBModelFaceMesh::GetActualLength()
{
  double actualLength =
    vtkCMBMesh::CombineMeshLengths(this->GetLength(),
                                   this->GetMasterMesh()->GetGlobalLength());
  return actualLength;
}

//----------------------------------------------------------------------------
double vtkCMBModelFaceMesh::GetActualMinimumAngle()
{
  double actualMinAngle =
    vtkCMBMesh::CombineMeshMinimumAngles(this->MinimumAngle,
                                         this->GetMasterMesh()->
                                         GetGlobalMinimumAngle());
  return actualMinAngle;
}

//----------------------------------------------------------------------------
void vtkCMBModelFaceMesh::PrintSelf(ostream& os, vtkIndent indent)
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
