//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBMeshGridRepresentationOperator.h"

#include "vtkAlgorithm.h"
#include "vtkCMBMeshGridRepresentationServer.h"
#include "vtkCMBMeshServer.h"
#include "vtkCMBMeshWrapper.h"
#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBMeshGridRepresentationOperator);

//----------------------------------------------------------------------------
vtkCMBMeshGridRepresentationOperator::vtkCMBMeshGridRepresentationOperator()
{
  this->GridFileName = 0;
  this->OperateSucceeded = 0;
  this->MeshIsAnalysisGrid = 0;
  this->MeshRepresentationSource = NULL;
}

//----------------------------------------------------------------------------
vtkCMBMeshGridRepresentationOperator:: ~vtkCMBMeshGridRepresentationOperator()
{
  this->SetGridFileName(0);
}

//----------------------------------------------------------------------------
void vtkCMBMeshGridRepresentationOperator::Operate(vtkCMBMeshWrapper* meshWrapper)
{
  this->OperateSucceeded = 0;
  vtkCMBMeshServer* mesh = meshWrapper->GetMesh();
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(mesh->GetModel());
  if (model && mesh)
    {
    if ( this->GridFileName != NULL && !this->MeshIsAnalysisGrid )
      {
      //create a new grid and write it out to file
      vtkCMBMeshGridRepresentationServer* gridRepresentation =
        vtkCMBMeshGridRepresentationServer::New();
      this->OperateSucceeded = this->MeshRepresentationSource ?
        gridRepresentation->Initialize(vtkPolyData::SafeDownCast(
            this->MeshRepresentationSource->GetOutputDataObject(0)), model) :
        gridRepresentation->Initialize(mesh);
      if (this->OperateSucceeded)
        {
        gridRepresentation->SetGridFileName(this->GridFileName);
        gridRepresentation->WriteMeshToFile();
        }
      gridRepresentation->Delete();
      }
    else if ( this->MeshIsAnalysisGrid )
      {
      vtkCMBMeshGridRepresentationServer *currentGrid =
        vtkCMBMeshGridRepresentationServer::SafeDownCast(
        model->GetAnalysisGridInfo());
      if (!currentGrid)
        {
        currentGrid = vtkCMBMeshGridRepresentationServer::New();
        model->SetAnalysisGridInfo(currentGrid);
        currentGrid->FastDelete();
        }

      this->OperateSucceeded = this->MeshRepresentationSource ?
        currentGrid->Initialize(vtkPolyData::SafeDownCast(
            this->MeshRepresentationSource->GetOutputDataObject(0)), model) :
        currentGrid->Initialize(mesh);
      if (this->OperateSucceeded && this->GridFileName != NULL)
        {
        //update the file name if we have one
        currentGrid->SetGridFileName(this->GridFileName);
        }

      //write the file to disk if the grid has a file name
      currentGrid->WriteMeshToFile();
      }
    }
  return;
}
//----------------------------------------------------------------------------
void vtkCMBMeshGridRepresentationOperator::SetMeshRepresentationInput(
  vtkAlgorithm* meshSource)
{
  this->MeshRepresentationSource = meshSource;
}

//----------------------------------------------------------------------------
void vtkCMBMeshGridRepresentationOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
