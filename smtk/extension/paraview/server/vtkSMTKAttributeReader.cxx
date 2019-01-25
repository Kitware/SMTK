//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKAttributeReader.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/Paths.h"

#include "smtk/resource/Manager.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkTable.h"

using namespace smtk;

vtkStandardNewMacro(vtkSMTKAttributeReader);

vtkSMTKAttributeReader::vtkSMTKAttributeReader()
{
  this->FileName = nullptr;
  this->IncludePathToFile = true;
  this->SetNumberOfOutputPorts(1);

  // Ensure this object's MTime > this->ModelSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKAttributeReader::~vtkSMTKAttributeReader()
{
  this->SetFileName(nullptr);
}

void vtkSMTKAttributeReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "IncludePathToFile: " << this->IncludePathToFile << "\n";
}

smtk::resource::ResourcePtr vtkSMTKAttributeReader::GenerateResource() const
{
  if (this->Resource && this->Wrapper)
  {
    this->Wrapper->GetResourceManager()->remove(this->Resource);
  }
  auto resource = smtk::attribute::Resource::create();
  auto name = smtk::common::Paths::stem(this->FileName);
  resource->setName(name);

  if (this->Wrapper)
  {
    this->Wrapper->GetResourceManager()->add(resource);
  }

  smtk::io::AttributeReader rdr;
  if (rdr.read(resource, this->FileName, this->IncludePathToFile, smtk::io::Logger::instance()))
  {
    vtkErrorMacro("Errors encountered: " << smtk::io::Logger::instance().convertToString() << "\n");
    vtkErrorMacro("Could not read \"" << this->FileName << "\"");
  }

  return resource;
}
