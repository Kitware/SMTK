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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"

#include "smtk/operation/Manager.h"

namespace
{
std::string dataRoot = SMTK_DATA_DIR;

void UniqueEntities(const smtk::model::EntityRef& root, std::set<smtk::model::EntityRef>& unique)
{
  smtk::model::EntityRefArray children = (root.isModel()
      ? root.as<smtk::model::Model>().cellsAs<smtk::model::EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<smtk::model::CellEntity>().boundingCellsAs<smtk::model::EntityRefArray>()
            : (root.isGroup() ? root.as<smtk::model::Group>().members<smtk::model::EntityRefArray>()
                              : smtk::model::EntityRefArray())));

  for (smtk::model::EntityRefArray::const_iterator it = children.begin(); it != children.end();
       ++it)
  {
    if (unique.find(*it) == unique.end())
    {
      unique.insert(*it);
      UniqueEntities(*it, unique);
    }
  }
}

void ParseModelTopology(smtk::model::Model model, std::size_t* count)
{
  std::set<smtk::model::EntityRef> unique;
  UniqueEntities(model, unique);

  for (auto&& entity : unique)
  {
    if (entity.dimension() >= 0 && entity.dimension() <= 3)
    {
      count[entity.dimension()]++;
      float r = static_cast<float>(entity.dimension()) / 3;
      float b = static_cast<float>(1.) - r;
      const_cast<smtk::model::EntityRef&>(entity).setColor(
        (r < 1. ? r : 1.), 0., (b < 1. ? b : 1.), 1.);
    }
  }
}
}

int UnitTestImportIntoResource(int argc, char* argv[])
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

  // Register import operator to the operation manager
  {
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
    importFilePath += "/model/3d/genesis/gun-1fourth.gen";
    importOp->parameters()->findFile("filename")->setValue(importFilePath);
    importOp->parameters()->findVoid("construct hierarchy")->setIsEnabled(false);

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
    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 1, "There should be one volume");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 5, "There should be five faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 0, "There should be no lines");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 0, "There should be no vertex groups");
  }

  {
    smtk::session::mesh::Import::Ptr importOp =
      operationManager->create<smtk::session::mesh::Import>();
    importOp->parameters()->associate(model->resource());
    std::string importFilePath(dataRoot);
    importFilePath += "/model/3d/exodus/disk_out_ref.ex2";
    importOp->parameters()->findFile("filename")->setValue(importFilePath);

    smtk::operation::Operation::Result importOpResult = importOp->operate();

    // Test for success
    if (importOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      return 1;
    }
  }

  {
    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 2, "There should be two volumes");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 12, "There should be twelve faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 0, "There should be no lines");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 3, "There should be three vertex groups");
  }

  return 0;
}
