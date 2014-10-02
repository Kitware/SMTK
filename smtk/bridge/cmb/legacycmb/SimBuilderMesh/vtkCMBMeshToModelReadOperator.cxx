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

#include "vtkCMBMeshToModelReadOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkCMBMeshToModelReader.h"

vtkStandardNewMacro(vtkCMBMeshToModelReadOperator);

vtkCMBMeshToModelReadOperator::vtkCMBMeshToModelReadOperator()
{
  this->FileName = 0;
  this->OperateSucceeded = 0;
}

vtkCMBMeshToModelReadOperator:: ~vtkCMBMeshToModelReadOperator()
{
  this->SetFileName(0);
}

void vtkCMBMeshToModelReadOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper || !ModelWrapper->GetModel())
    {
    vtkErrorMacro("Must have a valid CMB model.");
    return;
    }

  vtkDebugMacro("Reading a m2m file into a CMB model.");
  this->OperateSucceeded = 0;
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set m2m file name.");
    return;
    }

  vtkNew<vtkCMBMeshToModelReader> reader;
  reader->SetFileName(this->GetFileName());
  reader->SetModelWrapper(ModelWrapper);
  reader->Update();

  this->OperateSucceeded = reader->IsReadSuccessful();
}

void vtkCMBMeshToModelReadOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
