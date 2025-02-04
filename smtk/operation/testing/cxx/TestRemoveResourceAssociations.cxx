//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/operators/Import.h"

#include "smtk/common/UUIDGenerator.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Resource.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/RemoveResource.h"
#include "smtk/operation/operators/WriteResource.h"

#include "smtk/plugin/Registry.h"

#ifdef VTK_SUPPORT
#include "smtk/session/vtk/Registrar.h"
#endif

#include "smtk/common/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

// Test if resources can still be removed if they have associations.
// The test loads an SMTK attribute
// resource plus (if vtk support is built) an SMTK vtk model resource.

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& location)
{
  //first verify the file exists
  ::boost::filesystem::path path(location);
  printf("Removing: %s\n", location.c_str());
  if (::boost::filesystem::exists(path))
  {
    printf("Removing: %s...Success\n", location.c_str());
    //remove the file_path if it exists.
    ::boost::filesystem::remove_all(path);
  }
}

} // namespace

int TestRemoveResourceAssociations(int /*unused*/, char** const /*unused*/)
{
  // Set the file path
  std::string resourceDirectory = write_root + "/TestRemoveResourceAssociations";
  cleanup(resourceDirectory);
  ::boost::filesystem::create_directories(::boost::filesystem::path(resourceDirectory));
  std::string resourceFileLocation = resourceDirectory + "/foo.smtk";

  auto managers = smtk::common::Managers::create();
  // Construct smtk managers
  auto resourceRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(managers);
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(managers);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(managers);
  // access smtk managers
  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();

  // Initialize operations
  auto attributeOpRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);
  auto operationOpRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(resourceManager, operationManager);
#ifdef VTK_SUPPORT
  auto vtkRegistry =
    smtk::plugin::addToManagers<smtk::session::vtk::Registrar>(resourceManager, operationManager);
#endif
  smtk::attribute::ResourcePtr attrResource;
  smtk::model::ResourcePtr modelResource;
  std::size_t numberOfResources = 0;

  // Input data files
  std::vector<std::string> importPaths = { "/attribute/attribute_collection/elasticity.sbt" };
  {
    // Create an import operator
    smtk::operation::Operation::Ptr importAnyOp =
      operationManager->create<smtk::operation::ImportResource>();
    smtk::operation::Operation::Ptr importSBTOp =
      operationManager->create<smtk::attribute::Import>();

    smtkTest(!!importSBTOp, "No sbt import operator");
    smtkTest(!!importAnyOp, "No any import operator");

#ifdef VTK_SUPPORT
    importPaths.emplace_back("/model/3d/genesis/pillbox4.gen");
#endif
    for (auto& path : importPaths)
    {
      std::string importFilePath(data_root);
      importFilePath += path;
      std::string ext = boost::filesystem::path(importFilePath).extension().string();

      smtk::operation::Operation::Result importOpResult;
      if (ext == ".sbt")
      {
        importSBTOp->parameters()->findFile("filename")->setValue(importFilePath);
        importOpResult = importSBTOp->operate();
        auto resourceItem = importOpResult->findResource("resourcesCreated");
        attrResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceItem->value());
      }
      else
      {
        importAnyOp->parameters()->findFile("filename")->setValue(importFilePath);
        importOpResult = importAnyOp->operate();
        auto resourceItem = importOpResult->findResource("resourcesCreated");
        modelResource = std::dynamic_pointer_cast<smtk::model::Resource>(resourceItem->value());
      }
      ++numberOfResources;
    }
  }

  smtkTest(
    resourceManager->size() == numberOfResources,
    "Should have two resources: " << resourceManager->size());

  auto constraintAttr = attrResource->createAttribute("con0", "constraint");
  smtkTest(!!constraintAttr, "Cannot create contraint");

  smtk::operation::Operation::Ptr removeOp =
    operationManager->create<smtk::operation::RemoveResource>();
  if (modelResource)
  {
    {
      // model resource must be written to disk to re-load successfully
      // Create a write operator
      smtk::operation::WriteResource::Ptr writeOp =
        operationManager->create<smtk::operation::WriteResource>();

      smtkTest(!!writeOp, "No write operator");

      writeOp->parameters()->associate(modelResource);
      writeOp->parameters()->findFile("filename")->setIsEnabled(true);
      writeOp->parameters()->findFile("filename")->setValue(resourceFileLocation);

      smtkTest(writeOp->ableToOperate(), "Write operation unable to operate");

      auto writeOpResult = writeOp->operate();
      smtkTest(
        writeOpResult->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
        "Write resource operation failed.");
    }
    auto allFaces =
      modelResource->entitiesMatchingFlagsAs<smtk::model::EntityRefArray>(smtk::model::FACE);
    smtkTest(!allFaces.empty(), "Cannot retrieve model faces");
    // associate a model face with a reference item.
    smtkTest(constraintAttr->associate(allFaces[0].component()), "Association of face failed");
    smtkTest(
      constraintAttr->associatedObjects()->numberOfValues() == 1, "Face not in associatedObjects");

    // test model resource is reloaded ("ping pong" reload) after it is removed, due to the association
    removeOp->parameters()->associate(modelResource);
    auto removeResult = removeOp->operate();
    smtkTest(
      removeResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Remove resource operation 1 failed.");
    // trigger link checking the same way qtAssociation2ColumnWidget does
    constraintAttr->associatedObjects()->removeInvalidValues();

    smtkTest(
      resourceManager->size() == 2, "Should still have two resources " << resourceManager->size());
  }

  // confirm we can remove an attribute resource with a link to a model resource
  removeOp->parameters()->removeAllAssociations();
  removeOp->parameters()->associate(attrResource);
  auto removeResult = removeOp->operate();
  smtkTest(
    removeResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Remove resource operation 2 failed.");
  smtkTest(
    resourceManager->size() == numberOfResources - 1,
    "Should have removed attribute resource " << resourceManager->size());

  // reload and re-associate the attribute resource, and see if we can force-remove the model
  if (modelResource)
  {
    smtk::operation::Operation::Ptr importSBTOp =
      operationManager->create<smtk::attribute::Import>();
    std::string importFilePath(data_root);
    importFilePath += importPaths[0];
    importSBTOp->parameters()->findFile("filename")->setValue(importFilePath);
    auto importOpResult = importSBTOp->operate();
    auto resourceItem = importOpResult->findResource("resourcesCreated");
    attrResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceItem->value());
    constraintAttr = attrResource->createAttribute("con0", "constraint");
    smtkTest(!!constraintAttr, "Cannot create contraint");
    auto allFaces =
      modelResource->entitiesMatchingFlagsAs<smtk::model::EntityRefArray>(smtk::model::FACE);
    smtkTest(constraintAttr->associate(allFaces[0].component()), "Association of face failed");

    removeOp->parameters()->removeAllAssociations();
    removeOp->parameters()->associate(modelResource);

    // remove links to the model resource - now it won't re-load.
    removeOp->parameters()->findVoid("removeAssociations")->setIsEnabled(true);

    auto res = removeOp->operate();
    smtkTest(
      res->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Remove resource and links operation failed.");
    // trigger link checking the same way qtAssociation2ColumnWidget does
    constraintAttr->associatedObjects()->removeInvalidValues();
    smtkTest(
      resourceManager->size() == 1,
      "Should have removed model resource " << resourceManager->size());
  }
#ifdef NDEBUG
  cleanup(resourceDirectory);
#endif

  return 0;
}
