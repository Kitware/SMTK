//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKModelImporter.h"

#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"

#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"

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

vtkStandardNewMacro(vtkSMTKModelImporter);

vtkSMTKModelImporter::vtkSMTKModelImporter()
{
  this->FileName = nullptr;
  this->ResourceName = nullptr;
  this->SetNumberOfOutputPorts(vtkModelMultiBlockSource::NUMBER_OF_OUTPUT_PORTS);

  // Ensure this object's MTime > this->ModelSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKModelImporter::~vtkSMTKModelImporter()
{
  this->SetFileName(nullptr);
  this->SetResourceName(nullptr);
}

void vtkSMTKModelImporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "ModelSource: " << this->ModelSource << "\n";
}

vtkModelMultiBlockSource* vtkSMTKModelImporter::GetConverter() const
{
  return this->ModelSource.GetPointer();
}

smtk::resource::ResourcePtr vtkSMTKModelImporter::GetResource() const
{
  return std::static_pointer_cast<smtk::resource::Resource>(this->ModelSource->GetModelResource());
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkSMTKModelImporter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* outInfo)
{
  vtkMultiBlockDataSet* entitySource = vtkMultiBlockDataSet::GetData(
    outInfo, static_cast<int>(vtkModelMultiBlockSource::MODEL_ENTITY_PORT));

  vtkMultiBlockDataSet* instanceSource = vtkMultiBlockDataSet::GetData(
    outInfo, static_cast<int>(vtkModelMultiBlockSource::PROTOTYPE_PORT));

  if (!entitySource || !instanceSource)
  {
    vtkErrorMacro("No output dataset");
    return 0;
  }

  vtkMultiBlockDataSet* instancePlacement = vtkMultiBlockDataSet::GetData(
    outInfo, static_cast<int>(vtkModelMultiBlockSource::INSTANCE_PORT));

  if (!instancePlacement)
  {
    vtkErrorMacro("No output instance-placement dataset");
    return 0;
  }

  /*
   std::cout
     << "    Importer    " << this
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

  if (this->GetMTime() > this->ModelSource->GetMTime())
  {
    // Something changed. Probably the FileName.
    if (this->LoadFile())
    {
      this->ModelSource->Update();
    }
  }

  entitySource->ShallowCopy(this->ModelSource->GetOutputDataObject(
    static_cast<int>(vtkModelMultiBlockSource::MODEL_ENTITY_PORT)));
  instancePlacement->ShallowCopy(this->ModelSource->GetOutputDataObject(
    static_cast<int>(vtkModelMultiBlockSource::INSTANCE_PORT)));
  instanceSource->ShallowCopy(this->ModelSource->GetOutputDataObject(
    static_cast<int>(vtkModelMultiBlockSource::PROTOTYPE_PORT)));

  return 1;
}

bool vtkSMTKModelImporter::LoadFile()
{
  if (!this->FileName)
  {
    return false;
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
    return false;
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
    return false;
  }

  // If there is more than one operation that can import this file type, we
  // just use the first one.
  //
  // TODO: handle the case where multiple importers can import a file type
  smtk::operation::Operation::Index opIdx = *ops.begin();

  auto oper = operMgr->create(opIdx);
  if (!oper)
  {
    return false;
  }

  // Access the local operation's file item. Since importers have no restriction
  // on the name of their file item, we record the name during operation
  // registration and access it through the importer Group API.
  smtk::attribute::FileItem::Ptr importerFileItem =
    oper->parameters()->findFile(importerGroup.fileItemNameForOperation(opIdx));

  importerFileItem->setValue(this->FileName);

  auto result = oper->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    return false;
  }

  auto rsrc = result->findResource("resource")->value(0);
  auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Resource>(rsrc);
  if (!modelRsrc)
  {
    vtkWarningMacro("Cannot access resource from succesful import.");
    return false;
  }

  // If we have a resouce manager...
  if (rsrcMgr)
  {
    // ... remove the previous resource if we had one.
    auto oldRsrc = this->ModelSource->GetModelResource();
    if (oldRsrc)
    {
      rsrcMgr->remove(oldRsrc);
    }
    // Add the resource we just imported.
    rsrcMgr->add(rsrc);
  }

  // Tell our multiblock source to generate VTK polydata for model/mesh entities.
  this->ModelSource->SetModelResource(modelRsrc);

  // Also, find the first model and tell the multiblock source to render only it.
  // TODO: Either we need a separate representation for each model (which is
  //       IMNSHO a bad idea) or we need to change the multiblock source to work
  //       without a model. It currently generates tessellations but not all the
  //       metadata required for color-by modes.
  auto models =
    modelRsrc->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);
  if (!models.empty())
  {
    this->ModelSource->SetModelEntityID(models.begin()->entity().toString().c_str());
  }

  return true;
}
