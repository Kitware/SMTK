//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelReadOperator.h"

#include "smtk/bridge/discrete/Session.h"
#include "vtkCMBModelReader.h"
#include "vtkCMBParserV2.h"
#include "vtkCMBParserV4.h"
#include "vtkCMBParserV5.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

namespace
{
  char CMBFileVersionString[] = "version";
}

vtkStandardNewMacro(vtkCMBModelReadOperator);

vtkCMBModelReadOperator::vtkCMBModelReadOperator()
{
  this->FileName = 0;
  this->OperateSucceeded = 0;
}

vtkCMBModelReadOperator:: ~vtkCMBModelReadOperator()
{
  this->SetFileName(0);
}

const char* vtkCMBModelReadOperator::GetCMBFileVersionString()
{
  return CMBFileVersionString;
}

void vtkCMBModelReadOperator::Operate(vtkDiscreteModelWrapper* ModelWrapper,
                                      smtk::bridge::discrete::Session* session)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return;
    }
  vtkDiscreteModel* Model = ModelWrapper->GetModel();
  this->Read(Model, session);
  if(this->OperateSucceeded)
    {
    ModelWrapper->InitializeWithModelGeometry();
    }
}

void vtkCMBModelReadOperator::Read(vtkDiscreteModel* Model,
                                   smtk::bridge::discrete::Session* session)
{
  vtkDebugMacro("Reading a CMB file into a CMB model.");
  this->OperateSucceeded = 0;
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return;
    }

  vtkNew<vtkCMBModelReader> reader;
  reader->SetFileName(this->GetFileName());
  reader->Update();
  vtkPolyData* MasterPoly = reader->GetOutput();

  vtkCMBParserBase* parser = this->NewParser(MasterPoly);
  if(!parser)
    {
    vtkErrorMacro("File version not supported.");
    return;
    }

  this->OperateSucceeded = parser->Parse(MasterPoly, Model, session);
  Model->SetFileName(this->GetFileName());
  parser->Delete();

  return;
}

vtkCMBParserBase* vtkCMBModelReadOperator::NewParser(vtkPolyData* MasterPoly)
{
  vtkIntArray* version = vtkIntArray::SafeDownCast(
    MasterPoly->GetFieldData()->GetArray(CMBFileVersionString));

  if(!version || version->GetValue(0) == 1)
    {
    vtkErrorMacro("Version 1 CMB file no longer supported.");
    return NULL;
    }
  else if(version->GetValue(0) == 2 || version->GetValue(0) == 3)
    {
    return vtkCMBParserV2::New();
    }
  else if(version->GetValue(0) == 4)
    {
    return vtkCMBParserV4::New();
    }
  else if(version->GetValue(0) == 5)
    {
    return vtkCMBParserV5::New();
    }
  else
    {
    vtkErrorMacro("Unsupported file version " << version->GetValue(0));
    }

  return 0;
}

void vtkCMBModelReadOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}
