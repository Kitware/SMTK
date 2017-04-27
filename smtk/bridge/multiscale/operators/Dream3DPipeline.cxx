//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/multiscale/operators/Dream3DPipeline.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/bridge/multiscale/Session.h"
#include "smtk/extension/vtk/io/ImportVTKData.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPythonInterpreter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXdmfReader.h"

#include "boost/filesystem.hpp"

#include "smtk/io/ExportMesh.h"

#define STRINGIFY(s) TOSTRING(s)
#define TOSTRING(s) #s

using namespace smtk::model;
using namespace smtk::common;
using namespace boost::filesystem;

namespace smtk
{
namespace bridge
{
namespace multiscale
{

smtk::model::OperatorResult Dream3DPipeline::operateInternal()
{
  smtk::model::OperatorResult result;

  std::stringstream preamble;
  preamble << "import os\n";
  preamble << "os.environ[\"AFRL_DIR\"] = \"" << STRINGIFY(AFRL_DIR) << "\"\n";
  preamble << this->specToArgList(this->specification());

  std::stringstream script;
  script << STRINGIFY(AFRL_DIR) << "/CMBPreprocessingScripts/Dream3DPipeline.py";

  result = this->executePythonScript(preamble.str(), script.str());

  if (result->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    return result;
  }

  std::string inputfile = this->specification()->findFile("output-file")->value();
  inputfile = inputfile.substr(0, inputfile.find_last_of(".")) + ".xdmf";

  vtkNew<vtkXdmfReader> xdmfReader;
  xdmfReader->SetFileName(inputfile.c_str());
  xdmfReader->Update();

  vtkMultiBlockDataSet* multiBlockDataSet =
    vtkMultiBlockDataSet::SafeDownCast(xdmfReader->GetOutputDataObject(0));

  bool valid = false;
  vtkCompositeDataIterator* it = multiBlockDataSet->NewIterator();
  for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
  {
    if (strcmp(it->GetCurrentMetaData()->Get(vtkCompositeDataSet::NAME()), "VolumeDataContainer") ==
      0)
    {
      valid = true;
      break;
    }
  }

  if (!valid)
  {
    result = this->createResult(smtk::model::OPERATION_FAILED);
    return result;
  }

  vtkUnstructuredGrid* volumeDataContainer =
    vtkUnstructuredGrid::SafeDownCast(it->GetCurrentDataObject());

  if (!volumeDataContainer)
  {
    result = this->createResult(smtk::model::OPERATION_FAILED);
    return result;
  }

  smtk::extension::vtk::io::ImportVTKData importVTKData;
  smtk::mesh::ManagerPtr meshManager = this->activeSession()->meshManager();
  smtk::mesh::CollectionPtr collection = importVTKData(volumeDataContainer, meshManager, "ZoneIds");

  if (!collection || !collection->isValid())
  {
    // The file was not correctly read.
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  // Assign its model manager to the one associated with this session
  collection->setModelManager(this->activeSession()->manager());
  collection->name("result(dream3d)");

  // Construct the topology
  this->activeSession()->addTopology(std::move(smtk::bridge::mesh::Topology(collection)));

  // Our collections will already have a UUID, so here we create a model given
  // the model manager and uuid
  smtk::model::Model model =
    smtk::model::EntityRef(this->activeSession()->manager(), collection->entity());

  collection->associateToModel(model.entity());

  // Set the model's session to point to the current session
  model.setSession(
    smtk::model::SessionRef(this->activeSession()->manager(), this->activeSession()->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  this->activeSession()->transcribe(model, smtk::model::SESSION_EVERYTHING, false);

  result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItem::Ptr resultModels = result->findModelEntity("model");
  resultModels->setValue(model);

  smtk::attribute::ModelEntityItem::Ptr created = result->findModelEntity("created");
  created->setNumberOfValues(1);
  created->setValue(model);
  created->setIsEnabled(true);

  result->findModelEntity("mesh_created")->setValue(model);

  return result;
}

} // namespace multiscale
} //namespace bridge
} // namespace smtk

#undef STRINGIFY
#undef TOSTRING

#include "smtk/bridge/multiscale/Dream3DPipeline_xml.h"
#include "smtk/bridge/multiscale/Exports.h"

smtkImplementsModelOperator(SMTKMULTISCALESESSION_EXPORT, smtk::bridge::multiscale::Dream3DPipeline,
  multiscale_dream3d, "dream3d", Dream3DPipeline_xml, smtk::bridge::multiscale::Session);
