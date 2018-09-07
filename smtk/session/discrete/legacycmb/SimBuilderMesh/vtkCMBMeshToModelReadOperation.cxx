//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshToModelReadOperation.h"

#include "vtkCMBMeshToModelReader.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBMeshToModelReadOperation);

vtkCMBMeshToModelReadOperation::vtkCMBMeshToModelReadOperation()
{
  this->FileName = 0;
  this->OperateSucceeded = 0;
}

vtkCMBMeshToModelReadOperation::~vtkCMBMeshToModelReadOperation()
{
  this->SetFileName(0);
}

void vtkCMBMeshToModelReadOperation::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if (!ModelWrapper || !ModelWrapper->GetModel())
  {
    vtkErrorMacro("Must have a valid CMB model.");
    return;
  }

  vtkDebugMacro("Reading a m2m file into a CMB model.");
  this->OperateSucceeded = 0;
  if (!this->GetFileName())
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

void vtkCMBMeshToModelReadOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
