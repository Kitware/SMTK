//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelReadOperation.h"

#include "smtk/session/discrete/Session.h"
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

vtkStandardNewMacro(vtkCMBModelReadOperation);

vtkCMBModelReadOperation::vtkCMBModelReadOperation()
{
  this->FileName = nullptr;
  this->OperateSucceeded = 0;
}

vtkCMBModelReadOperation::~vtkCMBModelReadOperation()
{
  this->SetFileName(nullptr);
}

const char* vtkCMBModelReadOperation::GetCMBFileVersionString()
{
  return CMBFileVersionString;
}

void vtkCMBModelReadOperation::Operate(
  vtkDiscreteModelWrapper* ModelWrapper, smtk::session::discrete::Session* session)
{
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return;
  }
  vtkDiscreteModel* Model = ModelWrapper->GetModel();
  this->Read(Model, session);
  if (this->OperateSucceeded)
  {
    ModelWrapper->InitializeWithModelGeometry();
  }
}

void vtkCMBModelReadOperation::Read(
  vtkDiscreteModel* Model, smtk::session::discrete::Session* session)
{
  vtkDebugMacro("Reading a CMB file into a CMB model.");
  this->OperateSucceeded = 0;
  if (!this->GetFileName())
  {
    vtkWarningMacro("Must set file name.");
    return;
  }

  vtkNew<vtkCMBModelReader> reader;
  reader->SetFileName(this->GetFileName());
  reader->Update();
  vtkPolyData* MasterPoly = reader->GetOutput();

  vtkCMBParserBase* parser = this->NewParser(MasterPoly);
  if (!parser)
  {
    vtkErrorMacro("File version not supported.");
    return;
  }

  this->OperateSucceeded = parser->Parse(MasterPoly, Model, session);
  Model->SetFileName(this->GetFileName());
  parser->Delete();

  return;
}

vtkCMBParserBase* vtkCMBModelReadOperation::NewParser(vtkPolyData* MasterPoly)
{
  vtkIntArray* version =
    vtkIntArray::SafeDownCast(MasterPoly->GetFieldData()->GetArray(CMBFileVersionString));

  if (!version)
  {
    vtkErrorMacro("No Version in CMB file, unsupported.");
    return nullptr;
  }
  else if (version->GetValue(0) == 1)
  {
    vtkErrorMacro("Version 1 CMB file no longer supported.");
    return nullptr;
  }
  else if (version->GetValue(0) == 2 || version->GetValue(0) == 3)
  {
    return vtkCMBParserV2::New();
  }
  else if (version->GetValue(0) == 4)
  {
    return vtkCMBParserV4::New();
  }
  else if (version->GetValue(0) == 5)
  {
    return vtkCMBParserV5::New();
  }
  else
  {
    vtkErrorMacro("Unsupported file version " << version->GetValue(0));
  }

  return nullptr;
}

void vtkCMBModelReadOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
