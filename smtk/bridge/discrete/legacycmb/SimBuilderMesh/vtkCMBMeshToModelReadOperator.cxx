//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBMeshToModelReadOperator.h"

#include "vtkCMBMeshToModelReader.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

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
