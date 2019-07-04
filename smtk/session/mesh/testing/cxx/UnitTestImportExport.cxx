//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/Export.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/common/UUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/operation/Manager.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <fstream>

using namespace smtk::model;

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
std::string writeRoot = SMTK_SCRATCH_DIR;

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

int UnitTestImportExport(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::session::mesh::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import and write operators to the operation manager
  {
    operationManager->registerOperation<smtk::session::mesh::Export>("smtk::session::mesh::Export");
    operationManager->registerOperation<smtk::session::mesh::Import>("smtk::session::mesh::Import");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  {
    // Create an import operator
    smtk::session::mesh::Import::Ptr importOp =
      operationManager->create<smtk::session::mesh::Import>();
    if (!importOp)
    {
      std::cerr << "No import operator\n";
      return 1;
    }

    // Set the file path
    std::string importFilePath(dataRoot);
    importFilePath += "/model/3d/exodus/SimpleReactorCore/SimpleReactorCore.exo";
    importOp->parameters()->findFile("filename")->setValue(importFilePath);

    // Execute the operation
    smtk::operation::Operation::Result importOpResult = importOp->operate();

    // Retrieve the resulting model
    smtk::attribute::ComponentItemPtr componentItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        importOpResult->findComponent("model"));

    // Access the generated model
    model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

    // Test for success
    if (importOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      return 1;
    }
  }

  {
    // Create an export operator
    smtk::operation::Operation::Ptr exportOp =
      operationManager->create<smtk::session::mesh::Export>();
    if (!exportOp)
    {
      std::cerr << "No export operator\n";
      return 1;
    }

    // Set the file path
    std::string exportFilePath(writeRoot);
    exportFilePath += "/" + smtk::common::UUID::random().toString() + ".exo";
    exportOp->parameters()->findFile("filename")->setValue(exportFilePath);

    // Set the entity association
    exportOp->parameters()->associate(model->resource());

    // Execute the operation
    smtk::operation::Operation::Result exportOpResult = exportOp->operate();

    // Test for success
    if (exportOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Export operator failed\n";
      return 1;
    }

    cleanup(exportFilePath);
  }

  return 0;
}
