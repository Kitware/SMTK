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

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
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
  //std::cout << "Create reader " << this << "\n";
  this->FileName = nullptr;
  this->ResourceObserver = nullptr;
  this->Defs = vtkSmartPointer<vtkTable>::New();
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  // Ensure this object's MTime > this->ModelSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKAttributeReader::~vtkSMTKAttributeReader()
{
  //std::cout << "Delete reader " << this << "\n";
  this->SetFileName(nullptr);
}

void vtkSMTKAttributeReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "AttributeResource: " << this->AttributeResource << "\n";
  os << indent << "ResourceObserver: " << (this->ResourceObserver ? "Y" : "N") << "\n";
}

smtk::attribute::CollectionPtr vtkSMTKAttributeReader::GetSMTKResource() const
{
  return this->AttributeResource;
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkSMTKAttributeReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* outInfo)
{
  vtkMultiBlockDataSet* entitySource = vtkMultiBlockDataSet::GetData(outInfo, 0);

  /*
   std::cout
     << "    Reader    " << this
     << " has file " << (this->FileName && this->FileName[0] ? "Y" : "N") << "\n";
   */
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
  entitySource->SetBlock(0, this->Defs);
  return 1;
}

bool vtkSMTKAttributeReader::LoadFile()
{
  if (this->AttributeResource && this->ResourceObserver)
  {
    this->ResourceObserver(this->AttributeResource, false);
  }
  auto mgr = smtk::attribute::Collection::create();
  this->AttributeResource = mgr;
  mgr->setLocation(this->FileName);

  smtk::io::AttributeReader rdr;
  if (!rdr.read(
        this->AttributeResource, this->FileName, this->IncludePath, smtk::io::Logger::instance()))
  {
    this->AttributeResource = nullptr;
    vtkErrorMacro("Could not read \"" << this->FileName << "\"");
    return false;
  }

  if (this->ResourceObserver)
  {
    this->ResourceObserver(this->AttributeResource, true);
  }

  return true;
}
