//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelOmicronMeshInputWriterBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkCMBModelOmicronMeshInputWriter.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBModelOmicronMeshInputWriterBase);

//----------------------------------------------------------------------------
vtkCMBModelOmicronMeshInputWriterBase::vtkCMBModelOmicronMeshInputWriterBase()
{
  this->OperateSucceeded = 0;
  this->FileName = 0;
  this->GeometryFileName = 0;
  this->TetGenOptions = 0;
  this->VolumeConstraint = 0.001;
}

//----------------------------------------------------------------------------
vtkCMBModelOmicronMeshInputWriterBase::~vtkCMBModelOmicronMeshInputWriterBase()
{
  this->SetFileName(0);
  this->SetGeometryFileName(0);
  this->SetTetGenOptions(0);
}

//----------------------------------------------------------------------------
void vtkCMBModelOmicronMeshInputWriterBase::Operate(vtkDiscreteModelWrapper* modelWrapper)
{
  vtkDebugMacro("Writing a CMB file.");
  this->OperateSucceeded = 0;
  if (!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return;
    }
  if (!this->GeometryFileName)
    {
    vtkWarningMacro("Must specify GeometryFileName!\n");
    return;
    }

  if (!this->TetGenOptions)
    {
    vtkWarningMacro("Must specify TetGenOptions!\n");
    return;
    }

  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }

  vtkSmartPointer<vtkCMBModelOmicronMeshInputWriter> writer =
    vtkSmartPointer<vtkCMBModelOmicronMeshInputWriter>::New();
  this->OperateSucceeded = writer->Write(modelWrapper->GetModel(), this);
}

//----------------------------------------------------------------------------
void vtkCMBModelOmicronMeshInputWriterBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "GeometryFileName: " << this->GeometryFileName << endl;
  os << indent << "TetGenOptions: " << this->TetGenOptions << endl;
  os << indent << "VolumeConstraint: " << this->VolumeConstraint << endl;
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
