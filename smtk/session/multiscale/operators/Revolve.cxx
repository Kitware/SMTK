//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/multiscale/operators/Revolve.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/session/multiscale/Resource.h"
#include "smtk/session/multiscale/Revolve_xml.h"
#include "smtk/session/multiscale/Session.h"

#include "smtk/common/UUID.h"

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"
#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#include "smtk/mesh/core/Collection.h"

#include "vtkNew.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVolumeOfRevolutionFilter.h"

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

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
}

namespace smtk
{
namespace session
{
namespace multiscale
{

Revolve::Result Revolve::operateInternal()
{
  Result result;

  // Grab the datasets associated with the operator
  auto assocs = this->parameters()->associations();
  auto datasets = assocs->as<smtk::model::Models>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::Model(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  if (datasets.empty())
  {
    smtkErrorMacro(this->log(), "No models to revolve.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  smtk::model::Model dataset = datasets[0];

  smtk::session::multiscale::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::multiscale::Resource>(dataset.component()->resource());
  smtk::session::multiscale::Session::Ptr session = resource->session();

  if (!session)
  {
    smtkErrorMacro(this->log(), "No session associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // The collection for this model has the same UUID as the model, so we can
  // access it using the model's UUID
  smtk::mesh::CollectionPtr collection =
    session->meshManager()->findCollection(dataset.entity())->second;

  if (!collection->isValid())
  {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Convert all of the 2-dimensional mesh elements into a vtkUnstructuredGrid.
  // We don't want any of the 1- or 0-dimensional elements that were created via
  // shell operations because they would create artifacts when the slice is
  // revolved. Label the domain partitioning "ZoneIds" so that the partitioning
  // will be persistent through the revolution operation.
  vtkNew<vtkUnstructuredGrid> ug;
  smtk::extension::vtk::io::mesh::ExportVTKData exportVTKData;
  exportVTKData(collection->meshes(smtk::mesh::DimensionType(2)), ug.GetPointer(), "ZoneIds");

  // Create a VolumeOfRevolution filter
  vtkNew<vtkVolumeOfRevolutionFilter> revolve;
  revolve->SetInputData(ug.GetPointer());

  // Set the axis direction from the values held in the parameters
  double axisDirection[3];
  smtk::attribute::DoubleItemPtr axisDirectionItem =
    this->parameters()->findDouble("axis-direction");
  for (int i = 0; i < 3; i++)
    axisDirection[i] = axisDirectionItem->value(i);
  revolve->SetAxisDirection(axisDirection);

  // Set the axis position from the values held in the parameters
  double axisPosition[3];
  smtk::attribute::DoubleItemPtr axisPositionItem = this->parameters()->findDouble("axis-position");
  for (int i = 0; i < 3; i++)
    axisPosition[i] = axisPositionItem->value(i);
  revolve->SetAxisPosition(axisPosition);

  // Set the sweep angle and resolution from the values held in the
  // parameters
  revolve->SetSweepAngle(this->parameters()->findDouble("sweep-angle")->value());
  revolve->SetResolution(this->parameters()->findInt("resolution")->value());

  // Run the filter
  revolve->Update();

  // Convert the vtkUnstructuredGrid back into an smtk mesh, preserving the
  // domain partitioning.
  smtk::extension::vtk::io::mesh::ImportVTKData importVTKData;
  smtk::mesh::ManagerPtr meshManager = session->meshManager();
  collection =
    importVTKData(vtkUnstructuredGrid::SafeDownCast(revolve->GetOutput()), meshManager, "ZoneIds");

  if (!collection || !collection->isValid())
  {
    // The file was not correctly read.
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Assign its model manager to the one associated with this session
  collection->setModelResource(session->resource());
  collection->name("Revolved mesh");

  // Construct the topology
  session->addTopology(std::move(smtk::session::mesh::Topology(collection)));

  // Our collection already has a UUID, so here we create a model given the
  // model manager and UUID
  smtk::model::Model model = resource->insertModel(collection->entity(), 3, 3, "Revolved model");
  session->declareDanglingEntity(model);

  model.setSession(smtk::model::SessionRef(resource, session->sessionId()));

  // Associate the collection to our newly created model
  collection->associateToModel(model.entity());

  // Set the model's session to point to the current session
  model.setSession(smtk::model::SessionRef(session->resource(), session->sessionId()));

  // If we don't call "transcribe" ourselves, it never gets called.
  session->transcribe(model, smtk::model::SESSION_EVERYTHING, false);

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  auto resultModels = result->findComponent("model");
  resultModels->setValue(model.component());

  smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
  created->setNumberOfValues(1);
  created->setValue(model.component());
  created->setIsEnabled(true);

  result->findComponent("mesh_created")->setValue(model.component());

  return result;
}

const char* Revolve::xmlDescription() const
{
  return Revolve_xml;
}
}
}
}
