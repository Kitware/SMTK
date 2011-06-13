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

#include "vtkCmbMeshGridRepresentationServer.h"
#include "vtkCMBModel.h"
#include "vtkCMBModelWrapper.h"
#include "vtkCMBMeshWrapper.h"
#include "vtkCmbMeshServer.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCmbMeshGridRepresentationOperator);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationOperator, "");

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationOperator::vtkCmbMeshGridRepresentationOperator()
{
  this->OperateSucceeded = 0;
}

//----------------------------------------------------------------------------
vtkCmbMeshGridRepresentationOperator:: ~vtkCmbMeshGridRepresentationOperator()
{
}

//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationOperator::Operate(vtkCMBMeshWrapper* meshWrapper)
{

  vtkCmbMeshServer* mesh = meshWrapper->GetMesh();
  vtkModel* model = mesh->GetModel();

  vtkSmartPointer<vtkCmbMeshGridRepresentationServer> gridRepresentation =
    vtkSmartPointer<vtkCmbMeshGridRepresentationServer>::New();

  if(this->OperateSucceeded = true
     //gridRepresentation->Initialize(model,mesh)
    {
    model->SetAnalysisGridInfo(gridRepresentation);
    }

  return;
}
//----------------------------------------------------------------------------
void vtkCmbMeshGridRepresentationOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
