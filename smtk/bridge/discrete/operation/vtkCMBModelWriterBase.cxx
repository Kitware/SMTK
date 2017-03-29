//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelWriterBase.h"

#include "smtk/bridge/discrete/Session.h"

#include "vtkCMBModelWriterV2.h"
#include "vtkCMBModelWriterV4.h"
#include "vtkCMBModelWriterV5.h"
#include "vtkCMBParserBase.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBModelWriterBase);

vtkCMBModelWriterBase::vtkCMBModelWriterBase()
{
  this->FileName = 0;
  this->Version = this->GetCurrentVersion();
  this->OperateSucceeded = 0;
}

vtkCMBModelWriterBase:: ~vtkCMBModelWriterBase()
{
  this->SetFileName(0);
}

void vtkCMBModelWriterBase::Operate(vtkDiscreteModelWrapper* ModelWrapper,
                                    smtk::bridge::discrete::Session* session)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return;
    }
  this->Write(ModelWrapper->GetModel(), session);
}

void vtkCMBModelWriterBase::Write(vtkDiscreteModel* model,
                                  smtk::bridge::discrete::Session* session)
{
  vtkDebugMacro("Writing a CMB file.");
  this->OperateSucceeded = 0;
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return;
    }

  // Note that all writers are currently derived from the
  // V2 version
  vtkCMBModelWriterV2 *writer=0;

  if(!model)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }
  if(this->Version == 2 || this->Version == 3)
    {
    writer = vtkCMBModelWriterV2::New();
    }
  else if(this->Version == 4)
    {
    writer = vtkCMBModelWriterV4::New();
    }
   else if(this->Version == 5)
    {
    writer = vtkCMBModelWriterV5::New();
    }
  else
    {
    vtkWarningMacro("Writing version " << this->Version << " not supported.");
    }

  if (writer)
    {
    writer->SetFileName(this->GetFileName());
    this->OperateSucceeded = writer->Write(model, session);
    writer->Delete();
    }
  vtkDebugMacro("Finished writing a CMB file.");
  return;
}

int vtkCMBModelWriterBase::GetCurrentVersion()
{
  return 5;
}

void vtkCMBModelWriterBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "Version: " << this->Version << endl;
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
