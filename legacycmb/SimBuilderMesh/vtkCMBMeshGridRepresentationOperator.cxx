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

#include "vtkCMBMeshGridRepresentationOperator.h"

#include "vtkAlgorithm.h"
#include "vtkDiscreteModel.h"
#include "vtkCMBMeshGridRepresentationServer.h"
#include "vtkCMBMeshServer.h"
#include "vtkCMBMeshWrapper.h"
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
