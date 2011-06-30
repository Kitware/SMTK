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

#include "vtkCmbMeshGridRepresentationOperator.h"

#include "vtkCMBModel.h"
#include "vtkCmbMeshGridRepresentationServer.h"
#include "vtkCmbMeshServer.h"
#include "vtkCmbMeshWrapper.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCmbMeshGridRepresentationOperator);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationOperator, "");

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationOperator::vtkCmbMeshGridRepresentationOperator()
{
  this->GridFileName = 0;
  this->OperateSucceeded = 0;
  this->MeshIsAnalysisGrid = 0;
}

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationOperator:: ~vtkCmbMeshGridRepresentationOperator()
{
  this->SetGridFileName(0);
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationOperator::Operate(vtkCmbMeshWrapper* meshWrapper)
{
  this->OperateSucceeded = 0;
  vtkCmbMeshServer* mesh = meshWrapper->GetMesh();
  vtkCMBModel* model = vtkCMBModel::SafeDownCast(mesh->GetModel());
  if (model && mesh)
    {
    if ( this->GridFileName != NULL && !this->MeshIsAnalysisGrid )
      {
      //create a new grid and write it out to file
      vtkCmbMeshGridRepresentationServer* gridRepresentation =
        vtkCmbMeshGridRepresentationServer::New();
      gridRepresentation->SetGridFileName(this->GridFileName);
      this->OperateSucceeded = gridRepresentation->Initialize(mesh);
      if (this->OperateSucceeded)
        {
        gridRepresentation->WriteToFile();
        }
      gridRepresentation->Delete();
      }
    else if ( this->MeshIsAnalysisGrid )
      {
      vtkCmbMeshGridRepresentationServer *currentGrid =
        vtkCmbMeshGridRepresentationServer::SafeDownCast(
        model->GetAnalysisGridInfo());
      if (!currentGrid)
        {
        currentGrid = vtkCmbMeshGridRepresentationServer::New();
        model->SetAnalysisGridInfo(currentGrid);
        }

      this->OperateSucceeded = currentGrid->Initialize(mesh);
      if (this->OperateSucceeded && this->GridFileName != NULL)
        {
        currentGrid->SetGridFileName(this->GridFileName);
        currentGrid->WriteToFile();
        }
      }
    }
  return;
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
