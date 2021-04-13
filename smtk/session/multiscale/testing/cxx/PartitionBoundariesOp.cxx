//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/operators/Import.h"

#include "smtk/session/multiscale/Registrar.h"
#include "smtk/session/multiscale/Session.h"
#include "smtk/session/multiscale/operators/PartitionBoundaries.h"
#include "smtk/session/multiscale/operators/Revolve.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

#include "smtk/resource/Manager.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <cassert>

using namespace smtk::model;

namespace
{

std::string afrlRoot = std::string(AFRL_DIR);
std::string dataRoot = SMTK_DATA_DIR;

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
} // namespace

int PartitionBoundariesOp(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  if (afrlRoot.empty())
  {
    std::cerr << "AFRL directory not defined\n";
    return 1;
  }

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register multiscale resources to the resource manager
  {
    smtk::session::multiscale::Registrar::registerTo(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register mesh and multiscale operators to the operation manager
  {
    smtk::session::multiscale::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an import operator
  smtk::operation::Operation::Ptr importOp = smtk::session::mesh::Import::create();
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(dataRoot);
  importFilePath += "/mesh/2d/ImportFromDEFORM.h5m";

  importOp->parameters()->findFile("filename")->setValue(importFilePath);

  smtk::operation::Operation::Result importOpResult = importOp->operate();

  if (
    importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::operation::Operation::Ptr revolveOp =
    operationManager->create<smtk::session::multiscale::Revolve>();
  if (!revolveOp)
  {
    std::cerr << "No revolve operator\n";
    return 1;
  }

  revolveOp->parameters()->associateEntity(model);
  revolveOp->parameters()->findDouble("sweep-angle")->setValue(30.);
  revolveOp->parameters()->findInt("resolution")->setValue(15);
  revolveOp->parameters()->findDouble("axis-direction")->setValue(0, 0.);
  revolveOp->parameters()->findDouble("axis-direction")->setValue(1, 1.);
  revolveOp->parameters()->findDouble("axis-direction")->setValue(2, 0.);
  revolveOp->parameters()->findDouble("axis-position")->setValue(0, -0.02);
  revolveOp->parameters()->findDouble("axis-position")->setValue(1, 0.);
  revolveOp->parameters()->findDouble("axis-position")->setValue(2, 0.);

  smtk::operation::Operation::Result revolveOpResult = revolveOp->operate();
  if (
    revolveOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Revolve operator failed\n";
    return 1;
  }

  // Retrieve the resulting model
  componentItem = std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
    revolveOpResult->findComponent("created"));

  // Access the generated model
  model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::operation::Operation::Ptr partitionBoundariesOp =
    operationManager->create<smtk::session::multiscale::PartitionBoundaries>();
  if (!partitionBoundariesOp)
  {
    std::cerr << "No partition boundaries operator\n";
    return 1;
  }

  partitionBoundariesOp->parameters()->associateEntity(model);
  partitionBoundariesOp->parameters()->findDouble("origin")->setValue(0, -0.02);
  partitionBoundariesOp->parameters()->findDouble("origin")->setValue(1, 0.);
  partitionBoundariesOp->parameters()->findDouble("origin")->setValue(2, 0.);
  partitionBoundariesOp->parameters()->findDouble("radius")->setValue(1.2);

  smtk::operation::Operation::Result partitionBoundariesOpResult = partitionBoundariesOp->operate();

  if (
    partitionBoundariesOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "partition boundaries operator failed\n";
    return 1;
  }

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr created =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("created"));

  // assert(created->numberOfValues() == 7);

  // for (auto it = created->begin(); it != created->end(); ++it)
  // {
  //   smtk::model::Entity::Ptr c = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
  //   smtk::model::EntityRef r = c->referenceAs<smtk::model::EntityRef>();
  //   assert(r.isValid());
  //   assert(r.isVertex());
  // }

  return 0;
}
