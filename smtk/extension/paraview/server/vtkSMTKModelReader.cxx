//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"

#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/SessionRef.h"

#include "smtk/operation/LoadResource.h"

#include "smtk/resource/Manager.h"

#include "smtk/attribute/ComponentItem.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

using namespace smtk;

vtkStandardNewMacro(vtkSMTKModelReader);

vtkSMTKModelReader::vtkSMTKModelReader()
{
  //std::cout << "Create reader " << this << "\n";
  this->SetNumberOfOutputPorts(vtkModelMultiBlockSource::NUMBER_OF_OUTPUT_PORTS);

  // Ensure this object's MTime > this->ModelSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKModelReader::~vtkSMTKModelReader()
{
  //std::cout << "Delete reader " << this << "\n";
  this->DropResource();
  this->SetWrapper(nullptr);
  this->SetFileName(nullptr);
}

void vtkSMTKModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelSource: " << this->ModelSource << "\n";
}

smtk::resource::ResourcePtr vtkSMTKModelReader::GetResource() const
{
  return std::dynamic_pointer_cast<smtk::resource::Resource>(this->GetSMTKResource());
}

smtk::model::ManagerPtr vtkSMTKModelReader::GetSMTKResource() const
{
  return this->ModelSource->GetModelManager();
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkSMTKModelReader::RequestData(vtkInformation* vtkNotUsed(request),
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

bool vtkSMTKModelReader::LoadFile()
{
  if (!this->FileName)
  {
    return false;
  }

  smtk::resource::Manager::Ptr rsrcMgr = this->Wrapper
    ? this->Wrapper->GetResourceManager()
    : smtk::environment::ResourceManager::instance();

  smtk::operation::Manager::Ptr operMgr = this->Wrapper
    ? this->Wrapper->GetOperationManager()
    : smtk::environment::OperationManager::instance();
  if (!operMgr)
  {
    return false;
  }

  auto oper = operMgr->create<smtk::operation::LoadResource>();
  if (!oper)
  {
    return false;
  }

  // oper->setResourceManager(rsrcMgr); // TJ: uncomment me to get further.
  oper->parameters()->findFile("filename")->setValue(this->FileName);

  auto result = oper->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    return false;
  }

  auto rsrc = result->findResource("resource")->value(0);
  auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Manager>(rsrc);
  if (!modelRsrc)
  {
    return false;
  }

  // If we have a resouce manager...
  if (rsrcMgr)
  {
    // ... remove the previous resource if we had one.
    auto oldRsrc = this->ModelSource->GetModelManager();
    if (oldRsrc)
    {
      rsrcMgr->remove(oldRsrc);
    }
    // Add the resource we just read.
    rsrcMgr->add(rsrc);
  }

  // Tell our multiblock source to generate VTK polydata for model/mesh entities.
  this->ModelSource->SetModelManager(modelRsrc);

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
