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
#include "vtkCmbModelEdgeMeshServer.h"

#include "vtkCleanPolylines.h"
#include "vtkCmbMesh.h"
#include "vtkCmbModelFaceMesh.h"
#include "vtkCMBModelEdge.h"
#include "vtkCmbModelVertexMesh.h"
#include "vtkMeshModelEdgesFilter.h"
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkCmbModelEdgeMeshServer);
vtkCxxRevisionMacro(vtkCmbModelEdgeMeshServer, "");

//----------------------------------------------------------------------------
vtkCmbModelEdgeMeshServer::vtkCmbModelEdgeMeshServer()
{
  vtkPolyData* poly = vtkPolyData::New();
  this->SetModelEntityMesh(poly);
  poly->FastDelete();
}

//----------------------------------------------------------------------------
vtkCmbModelEdgeMeshServer::~vtkCmbModelEdgeMeshServer()
{
}

//----------------------------------------------------------------------------
bool vtkCmbModelEdgeMeshServer::BuildMesh(bool meshHigherDimensionalEntities)
{
  this->SetMeshedLength(this->GetActualLength());
  if(this->GetActualLength() <= 0.)
    {
    this->SetModelEntityMesh(NULL);
    return true;
    }

  // Clean polylines will create a "full strip", whereas vtkStripper
  // starts building a strip from a line and adds lines to only one end,
  // such that it is possible for (what could be) a single polyline to end
  // up as several (perhaps MANY) polylines.  Clean polylines also deals with
  // non-manifold points, but at this point, not expectign that to be an issue.
  vtkSmartPointer<vtkCleanPolylines> stripper =
    vtkSmartPointer<vtkCleanPolylines>::New();
  stripper->SetMinimumLineLength(0);
  stripper->UseRelativeLineLengthOff();
  vtkCMBModelEdge* cmbModelEdge = vtkCMBModelEdge::SafeDownCast(this->GetModelEdge());
  vtkPolyData* polyd = vtkPolyData::SafeDownCast(cmbModelEdge->GetGeometry());
  stripper->SetInput(vtkPolyData::SafeDownCast(vtkCMBModelEdge::SafeDownCast(this->GetModelEdge())->GetGeometry() ) );
  stripper->Update();

  if ( stripper->GetOutput()->GetNumberOfLines() != 1 )
    {
    vtkErrorMacro("Expecting one and exactly one edge / polyline: " <<
      stripper->GetOutput()->GetNumberOfLines() );
    return false;
    }

  vtkDoubleArray *targetCellLength = vtkDoubleArray::New();
  targetCellLength->SetNumberOfComponents( 1 );
  targetCellLength->SetNumberOfTuples( 1 );
  double length = this->GetActualLength();

  targetCellLength->SetValue( 0, length );
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

  // it would seem like we could just do this->SetModelEntityMesh(mesh);
  // but we can't.  i think this has to do with the polydataprovider.
  vtkPolyData* mesh = this->GetModelEntityMesh();
  if(!mesh)
    {
    mesh = vtkPolyData::New();
    this->SetModelEntityMesh(mesh);
    mesh->FastDelete();
    }
  mesh->ShallowCopy(meshEdgesFilter->GetOutput());
  meshEdgesFilter->Delete();
  cerr << "model edge " << this->GetModelEdge()->GetUniquePersistentId()
       << " mesh built with numpoints " << mesh->GetNumberOfPoints() << endl;

  // now we go and remesh any adjacent model face meshes that exist
  if(meshHigherDimensionalEntities)
    {
    vtkModelItemIterator* faces =
      this->GetModelEdge()->NewAdjacentModelFaceIterator();
    for(faces->Begin();!faces->IsAtEnd();faces->Next())
      {
      vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      vtkCmbModelFaceMesh* faceMesh = vtkCmbModelFaceMesh::SafeDownCast(
        this->GetMasterMesh()->GetModelEntityMesh(face));
      faceMesh->BuildModelEntityMesh(true);
      }
    faces->Delete();
    }

  return true;
}

//----------------------------------------------------------------------------
void vtkCmbModelEdgeMeshServer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
