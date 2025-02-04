//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResourceReader.h"

#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/resource/Manager.h"

#include "smtk/attribute/ComponentItem.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

using namespace smtk;

vtkStandardNewMacro(vtkSMTKResourceReader);

vtkSMTKResourceReader::vtkSMTKResourceReader()
{
  this->FileName = nullptr;

  this->Modified();
}

vtkSMTKResourceReader::~vtkSMTKResourceReader()
{
  this->SetFileName(nullptr);
}

void vtkSMTKResourceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}

smtk::resource::ResourcePtr vtkSMTKResourceReader::GenerateResource() const
{
  if (!this->FileName)
  {
    return smtk::resource::ResourcePtr();
  }

  smtk::operation::Manager::Ptr operMgr;
  if (this->Wrapper != nullptr)
  {
    operMgr = this->Wrapper->GetOperationManager();
  }

  if (!operMgr)
  {
    return smtk::resource::ResourcePtr();
  }

  auto oper = operMgr->create<smtk::operation::ReadResource>();
  if (!oper)
  {
    return smtk::resource::ResourcePtr();
  }

  oper->parameters()->findFile("filename")->setValue(this->FileName);

  auto result = oper->operate();
  if (
    result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }

  return result->findResource("resourcesCreated")->value(0);
}
