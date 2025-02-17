//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResourceImporter.h"

#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/groups/ImporterGroup.h"

#include "smtk/resource/Manager.h"

#include "smtk/attribute/ComponentItem.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include <cassert>

using namespace smtk;

vtkStandardNewMacro(vtkSMTKResourceImporter);

vtkSMTKResourceImporter::vtkSMTKResourceImporter()
{
  this->FileName = nullptr;
  this->ResourceName = nullptr;

  // Ensure this object's MTime > this->ResourceSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKResourceImporter::~vtkSMTKResourceImporter()
{
  this->SetFileName(nullptr);
  this->SetResourceName(nullptr);
}

void vtkSMTKResourceImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}

smtk::resource::ResourcePtr vtkSMTKResourceImporter::GenerateResource() const
{
  if (!this->FileName)
  {
    return smtk::resource::ResourcePtr();
  }

  smtk::resource::Manager::Ptr rsrcMgr;
  if (this->Wrapper != nullptr)
  {
    rsrcMgr = this->Wrapper->GetResourceManager();
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

  // Access the importer group associated with this operation manager.
  smtk::operation::ImporterGroup importerGroup(operMgr);

  // Collect all of the import operation ids associated with our resource. If no
  // resource is associated with this importer, then just use all importers.
  std::string resourceName(this->ResourceName);
  std::set<smtk::operation::Operation::Index> ops = resourceName.empty()
    ? importerGroup.operationsForFileName(this->FileName)
    : importerGroup.operationsForResourceAndFileName(resourceName, this->FileName);

  if (ops.empty())
  {
    return smtk::resource::ResourcePtr();
  }

  // If there is more than one operation that can import this file type, we
  // just use the first one.
  //
  // TODO: handle the case where multiple importers can import a file type
  smtk::operation::Operation::Index opIdx = *ops.begin();

  auto oper = operMgr->create(opIdx);
  if (!oper)
  {
    return smtk::resource::ResourcePtr();
  }

  // Access the local operation's file item. Since importers have no restriction
  // on the name of their file item, we record the name during operation
  // registration and access it through the importer Group API.
  smtk::attribute::FileItem::Ptr importerFileItem =
    oper->parameters()->findFile(importerGroup.fileItemNameForOperation(opIdx));

  importerFileItem->setValue(this->FileName);

  auto result = oper->operate();
  if (
    result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }

  return result->findResource("resourcesCreated")->value(0);
}
