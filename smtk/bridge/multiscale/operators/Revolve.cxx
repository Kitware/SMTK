//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/multiscale/operators/Revolve.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/bridge/multiscale/Session.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/vtk/io/ExportVTKData.h"
#include "smtk/extension/vtk/io/ImportVTKData.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "vtkNew.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVolumeOfRevolutionFilter.h"

#include "vtkIdTypeArray.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLUnstructuredGridWriter.h"

#include <sstream>

#include <boost/cstdint.hpp>
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

using namespace smtk::model;
using namespace smtk::common;
using namespace boost::filesystem;

namespace
{
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup( const std::string& file_path )
{
  //first verify the file exists
  ::boost::filesystem::path path( file_path );
  if( ::boost::filesystem::is_regular_file( path ) )
    {
    //remove the file_path if it exists.
    ::boost::filesystem::remove( path );
    }
}
}

namespace smtk {
  namespace bridge {
    namespace multiscale {

smtk::model::OperatorResult Revolve::operateInternal()
{
  smtk::model::OperatorResult result;

  // Grab the datasets associated with the operator
  smtk::model::Models datasets =
    this->specification()->associatedModelEntities<smtk::model::Models>();
  if (datasets.empty())
    {
    smtkErrorMacro(this->log(), "No models to revolve.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  smtk::model::Model dataset = datasets[0];

  // The collection for this model has the same UUID as the model, so we can
  // access it using the model's UUID
  smtk::mesh::CollectionPtr collection =
    this->activeSession()->meshManager()
    ->findCollection( dataset.entity() )->second;

  if ( !collection->isValid() )
    {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Convert all of the 2-dimensional mesh elements into a vtkUnstructuredGrid.
  // We don't want any of the 1- or 0-dimensional elements that were created via
  // shell operations because they would create artifacts when the slice is
  // revolved. Label the domain partitioning "ZoneIds" so that the partitioning
  // will be persistent through the revolution operation.
  vtkNew<vtkUnstructuredGrid> ug;
  smtk::extension::vtk::io::ExportVTKData exportVTKData;
  exportVTKData(collection->meshes(smtk::mesh::DimensionType(2)),
                ug.GetPointer(), "ZoneIds");

  // Create a VolumeOfRevolution filter
  vtkNew<vtkVolumeOfRevolutionFilter> revolve;
  revolve->SetInputData(ug.GetPointer());

  // Set the axis direction from the values held in the specification
  double axisDirection[3];
  smtk::attribute::DoubleItemPtr axisDirectionItem =
    this->specification()->findDouble("axis-direction");
  for (int i=0; i<3; i++)
    axisDirection[i] = axisDirectionItem->value(i);
  revolve->SetAxisDirection(axisDirection);

  // Set the axis position from the values held in the specification
  double axisPosition[3];
  smtk::attribute::DoubleItemPtr axisPositionItem =
    this->specification()->findDouble("axis-position");
  for (int i=0; i<3; i++)
    axisPosition[i] = axisPositionItem->value(i);
  revolve->SetAxisPosition(axisPosition);

  // Set the sweep angle and resolution from the values held in the
  // specification
  revolve->SetSweepAngle(
    this->specification()->findDouble("sweep-angle")->value());
  revolve->SetResolution(
    this->specification()->findInt("resolution")->value());

  // Run the filter
  revolve->Update();

  // This hack is in place because vtkVolumeOfRevolutionFilter has a bug in it
  // where the output unstructured grid has bad values for its cell locations.
  // Once the VTK bundled in the CMB superbuild catches up, this hack should go
  // away.
  bool hack = true;

  if (hack)
  {
  std::stringstream s;
  s << write_root << "/" << smtk::common::UUID::random().toString() << ".vtu";
  std::string fileName = s.str();

  vtkNew<vtkXMLUnstructuredGridWriter> writer;
  writer->SetFileName(fileName.c_str());
  writer->SetInputConnection(revolve->GetOutputPort());
  writer->Write();

  vtkNew<vtkXMLUnstructuredGridReader> reader;
  reader->SetFileName(fileName.c_str());
  reader->Update();

  cleanup(fileName);

  smtk::extension::vtk::io::ImportVTKData importVTKData;
  smtk::mesh::ManagerPtr meshManager = this->activeSession()->meshManager();
  collection =
    importVTKData(vtkUnstructuredGrid::SafeDownCast(reader->GetOutput()),
                  meshManager, "ZoneIds");
  }
  else
  {
  // Convert the vtkUnstructuredGrid back into an smtk mesh, preserving the
  // domain partitioning.
  smtk::extension::vtk::io::ImportVTKData importVTKData;
  smtk::mesh::ManagerPtr meshManager = this->activeSession()->meshManager();
  collection =
    importVTKData(vtkUnstructuredGrid::SafeDownCast(revolve->GetOutput()),
                  meshManager, "ZoneIds");
  }

  if (!collection || !collection->isValid())
    {
    // The file was not correctly read.
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Assign its model manager to the one associated with this session
  collection->setModelManager(this->activeSession()->manager());
  collection->name("result(revolve)");

  // Construct the topology
  this->activeSession()->addTopology(
    std::move(smtk::bridge::mesh::Topology(collection)));

  // Our collection already has a UUID, so here we create a model given the
  // model manager and UUID
  smtk::model::Model model =
    smtk::model::EntityRef(this->activeSession()->manager(),
                           collection->entity());

  // Associate the collection to our newly created model
  collection->associateToModel(model.entity());

  // Set the model's session to point to the current session
  model.setSession(smtk::model::SessionRef(this->activeSession()->manager(),
                                           this->activeSession()->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  this->activeSession()->transcribe(
    model, smtk::model::SESSION_EVERYTHING, false);

  result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("model");
  resultModels->setValue(model);

  smtk::attribute::ModelEntityItem::Ptr created =
    result->findModelEntity("created");
  created->setNumberOfValues(1);
  created->setValue(model);
  created->setIsEnabled(true);

  result->findModelEntity("mesh_created")->setValue(model);

  return result;
}

    } // namespace multiscale
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/multiscale/Exports.h"
#include "smtk/bridge/multiscale/Revolve_xml.h"

smtkImplementsModelOperator(
  SMTKMULTISCALESESSION_EXPORT,
  smtk::bridge::multiscale::Revolve,
  multiscale_revolve,
  "revolve",
  Revolve_xml,
  smtk::bridge::multiscale::Session);
