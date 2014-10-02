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

#include "vtkCMBImportBCFileOperator.h"

#include "vtkModelBCGridRepresentation.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBImportBCFileOperator);

vtkCMBImportBCFileOperator::vtkCMBImportBCFileOperator()
{
  this->FileName = 0;
  this->OperateSucceeded = 0;
}

vtkCMBImportBCFileOperator:: ~vtkCMBImportBCFileOperator()
{
  this->SetFileName(0);
}

void vtkCMBImportBCFileOperator::Operate(vtkDiscreteModelWrapper* modelWrapper)
{
  vtkDebugMacro("Reading a CMB file into a CMB model.");
  this->OperateSucceeded = 0;
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return;
    }

  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }
  vtkDiscreteModel* model = modelWrapper->GetModel();

  vtkSmartPointer<vtkModelBCGridRepresentation> gridRepresentation =
    vtkSmartPointer<vtkModelBCGridRepresentation>::New();

  this->OperateSucceeded =
       gridRepresentation->Initialize(this->GetFileName(), model);
  if( this->OperateSucceeded )
    {
    model->SetAnalysisGridInfo(gridRepresentation);
    }

  return;
}

void vtkCMBImportBCFileOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}
