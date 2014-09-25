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
