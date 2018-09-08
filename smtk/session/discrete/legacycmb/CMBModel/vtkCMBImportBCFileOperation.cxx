//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBImportBCFileOperation.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelBCGridRepresentation.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBImportBCFileOperation);

vtkCMBImportBCFileOperation::vtkCMBImportBCFileOperation()
{
  this->FileName = 0;
  this->OperateSucceeded = 0;
}

vtkCMBImportBCFileOperation::~vtkCMBImportBCFileOperation()
{
  this->SetFileName(0);
}

void vtkCMBImportBCFileOperation::Operate(vtkDiscreteModelWrapper* modelWrapper)
{
  vtkDebugMacro("Reading a CMB file into a CMB model.");
  this->OperateSucceeded = 0;
  if (!this->GetFileName())
  {
    vtkWarningMacro("Must set file name.");
    return;
  }

  if (!modelWrapper)
  {
    vtkErrorMacro("Passed in a null model.");
    return;
  }
  vtkDiscreteModel* model = modelWrapper->GetModel();

  vtkSmartPointer<vtkModelBCGridRepresentation> gridRepresentation =
    vtkSmartPointer<vtkModelBCGridRepresentation>::New();

  this->OperateSucceeded = gridRepresentation->Initialize(this->GetFileName(), model);
  if (this->OperateSucceeded)
  {
    model->SetAnalysisGridInfo(gridRepresentation);
  }

  return;
}

void vtkCMBImportBCFileOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
