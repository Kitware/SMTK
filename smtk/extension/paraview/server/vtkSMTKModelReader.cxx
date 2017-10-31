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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"

#include "smtk/resource/Manager.h"

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
  this->FileName = nullptr;
  this->ResourceObserver = nullptr;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(vtkModelMultiBlockSource::NUMBER_OF_OUTPUT_PORTS);

  // Ensure this object's MTime > this->ModelSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKModelReader::~vtkSMTKModelReader()
{
  //std::cout << "Delete reader " << this << "\n";
  this->SetFileName(nullptr);
}

void vtkSMTKModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "ModelSource: " << this->ModelSource << "\n";
  os << indent << "ResourceObserver: " << (this->ResourceObserver ? "Y" : "N") << "\n";
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
  std::cout << "  Reading " << this->FileName << "\n";
  auto oldMgr = this->ModelSource->GetModelManager();
  if (oldMgr && this->ResourceObserver)
  {
    this->ResourceObserver(oldMgr, false);
  }
  auto mgr = model::Manager::create();
  this->ModelSource->SetModelManager(mgr);
  mgr->setLocation(this->FileName);

  auto ssn = mgr->createSession("native");
  auto rdr = ssn.op("load smtk model"); // smtk::model::LoadSMTKModel::create();
  if (!rdr)
  {
    return false;
  }

  auto filenameItem = rdr->specification()->findFile("filename");
  filenameItem->setNumberOfValues(1);
  filenameItem->setValue(0, this->FileName);
  auto res = rdr->operate();

  if (res->findInt("outcome")->value() != operation::Operator::OPERATION_SUCCEEDED)
  {
    vtkErrorMacro("Could not read \"" << this->FileName << "\"");
    return false;
  }

  auto cre = res->findModelEntity("created");
  if (cre->numberOfValues() > 0)
  {
    this->ModelSource->SetModelEntityID(cre->value().entity().toString().c_str());
  }

  if (this->ResourceObserver)
  {
    this->ResourceObserver(mgr, true);
  }

  return true;
}
