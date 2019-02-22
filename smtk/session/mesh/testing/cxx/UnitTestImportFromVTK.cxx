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
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"

#include "smtk/operation/Manager.h"

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
}

int UnitTestImportFromVTK(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  std::vector<std::string> files(
    { "/mesh/2d/ImportFromDEFORM.vtu", "/../thirdparty/delaunay/data/chesapeake-0.001-100.vtp" });

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::session::mesh::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import operator to the operation manager
  {
    operationManager->registerOperation<smtk::session::mesh::Import>("smtk::session::mesh::Import");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  for (auto file : files)
  {
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
      importFilePath += file;
      importOp->parameters()->findFile("filename")->setValue(importFilePath);

      // Test for success
      if (importOp->ableToOperate() == false)
      {
        std::cerr << "Import operator unable to operate\n";
        return 1;
      }

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
  }

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK)
