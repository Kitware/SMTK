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
  this->Defs = vtkSmartPointer<vtkTable>::New();
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
  os << indent << "AttributeResource: " << this->AttributeResource << "\n";
  os << indent << "IncludePathToFile: " << this->IncludePathToFile << "\n";
}

smtk::resource::ResourcePtr vtkSMTKAttributeReader::GetResource() const
{
  return std::dynamic_pointer_cast<smtk::resource::Resource>(this->GetSMTKResource());
}

smtk::attribute::ResourcePtr vtkSMTKAttributeReader::GetSMTKResource() const
{
  return this->AttributeResource;
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkSMTKAttributeReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* outInfo)
{
  vtkMultiBlockDataSet* entitySource = vtkMultiBlockDataSet::GetData(outInfo, 0);

  if (!this->FileName || !this->FileName[0])
  {
    // No filename is not really an error... we should just have an empty output.
    static bool once = false;
    if (!once)
    {
      once = true;
      //vtkWarningMacro("No filename specified. This is your only warning.");
    }
    return 1;
  }

  // if (this->GetMTime() > this->ModelSource->GetMTime())
  {
    // Something changed. Probably the FileName.
    this->LoadFile();
  }

  entitySource->SetNumberOfBlocks(1);
  // entitySource->SetBlock(0, this->Defs);
  return 1;
}

bool vtkSMTKAttributeReader::LoadFile()
{
  if (this->AttributeResource && this->Wrapper)
  {
    this->Wrapper->GetResourceManager()->remove(this->AttributeResource);
  }
  auto rsrc = smtk::attribute::Resource::create();
  this->AttributeResource = rsrc;
  rsrc->setLocation(this->FileName);

  if (this->Wrapper)
  {
    this->Wrapper->GetResourceManager()->add(this->AttributeResource);
  }

  smtk::io::AttributeReader rdr;
  if (rdr.read(rsrc, this->FileName, this->IncludePathToFile, smtk::io::Logger::instance()))
  {
    vtkErrorMacro("Errors encountered: " << smtk::io::Logger::instance().convertToString() << "\n");
    this->AttributeResource = nullptr;
    vtkErrorMacro("Could not read \"" << this->FileName << "\"");
    return false;
  }

  return true;
}
