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

#include "vtkCleanPolylines.h"
#include "vtkCmbMesh.h"
#include "vtkCMBModelEdge.h"
#include "vtkCmbModelVertexMesh.h"
#include "vtkMeshModelEdgesFilter.h"
#include <vtkModelEdge.h>
#include <vtkModelVertex.h>

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
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
  mesh->Initialize();
  mesh->Allocate();

  // Clean polylines will create a "full strip", whereas vtkStripper
  // starts building a strip from a line and adds lines to only one end,
  // such that it is possible for (what could be) a single polyline to end
  // up as several (perhaps MANY) polylines.  Clean polylines also deals with
  // non-manifold points, but at this point, not expectign that to be an issue.
  vtkCleanPolylines *stripper = vtkCleanPolylines::New();
  stripper->SetMinimumLineLength(0);
  stripper->UseRelativeLineLengthOff();
  stripper->SetInput( vtkPolyData::SafeDownCast(
    vtkCMBModelEdge::SafeDownCast(this->ModelEdge)->GetGeometry() ) );
  stripper->Update();

  if ( stripper->GetOutput()->GetNumberOfLines() != 1 )
    {
    vtkErrorMacro("Expecting one and exactly one edge / polyline: " <<
      stripper->GetOutput()->GetNumberOfLines() );
    stripper->Delete();
    return false;
    }

  vtkDoubleArray *targetCellLength = vtkDoubleArray::New();
  targetCellLength->SetNumberOfComponents( 1 );
  targetCellLength->SetNumberOfTuples( 1 );
  targetCellLength->SetValue( 0, this->GetModelEntityMeshSize() );
  targetCellLength->SetName( "TargetSegmentLength" );

  vtkPolyData *polyLinePD = vtkPolyData::New();
  polyLinePD->ShallowCopy( stripper->GetOutput() );
  polyLinePD->GetCellData()->AddArray( targetCellLength );
  targetCellLength->FastDelete();

  vtkMeshModelEdgesFilter *meshEdgesFilter = vtkMeshModelEdgesFilter::New();
  meshEdgesFilter->UseLengthAlongEdgeOff();
  meshEdgesFilter->SetInput( polyLinePD );
  polyLinePD->FastDelete();
  meshEdgesFilter->SetTargetSegmentLengthCellArrayName(
    targetCellLength->GetName() );
  meshEdgesFilter->Update();

  mesh->SetPoints( meshEdgesFilter->GetOutput()->GetPoints() );
  mesh->SetLines( meshEdgesFilter->GetOutput()->GetLines() );

  stripper->Delete();
  meshEdgesFilter->Delete();
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

