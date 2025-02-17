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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"

#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"

#include "smtk/plugin/Registry.h"

#include "smtk/session/polygon/Registrar.h"
#include "smtk/session/polygon/Resource.h"

#include "smtk/session/polygon/json/jsonResource.h"

#include "smtk/operation/Manager.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
std::string writeRoot = SMTK_SCRATCH_DIR;
std::string filename("/scene/ArcSetsTest/innerLoopHard_contour.vtp");

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

void UniqueEntities(const smtk::model::EntityRef& root, std::set<smtk::model::EntityRef>& unique)
{
  smtk::model::EntityRefArray children =
    (root.isModel()
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

void ValidateModelTopology(smtk::model::Model model)
{
  std::size_t count[4] = { 0, 0, 0, 0 };
  ParseModelTopology(model, count);

  std::cout << count[3] << " volumes" << std::endl;
  test(count[3] == 0, "There should be no volumes");
  std::cout << count[2] << " faces" << std::endl;
  test(count[2] == 0, "There should be no faces");
  std::cout << count[1] << " edges" << std::endl;
  test(count[1] == 7, "There should be seven lines");
  std::cout << count[0] << " vertex groups" << std::endl;
  test(count[0] == 6, "There should be six vertex groups");
}
} // namespace

int UnitTestPolygonReadWrite(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  auto polygonRegistry = smtk::plugin::addToManagers<smtk::session::polygon::Registrar>(
    resourceManager, operationManager);
  auto operationRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an "import" operator
  smtk::operation::ImportResource::Ptr importOp =
    operationManager->create<smtk::operation::ImportResource>();

  test(importOp != nullptr, "No import operator");
  std::string readFilePath = dataRoot + filename;
  importOp->parameters()->findFile("filename")->setValue(readFilePath);
  std::cout << "Importing " << readFilePath << std::endl;

  smtk::operation::Operation::Result importOpResult = importOp->operate();
  test(
    importOpResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Import operator failed");

  // Retrieve the resulting polygon resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resourcesCreated"));
  smtk::session::polygon::Resource::Ptr polygonResource =
    std::dynamic_pointer_cast<smtk::session::polygon::Resource>(resourceItem->value());

  // Retrieve the resulting model
  {
    smtk::model::Models models = polygonResource->entitiesMatchingFlagsAs<smtk::model::Models>(
      smtk::model::MODEL_ENTITY, false);

    std::cout << "found " << models.size() << " models" << std::endl;
    if (models.empty())
      return 1;

    smtk::model::Model model = models[0];
    ValidateModelTopology(model);
  }

  std::string writeFilePath(writeRoot);
  writeFilePath += "/" + smtk::common::UUID::random().toString() + ".smtk";
  polygonResource->setLocation(writeFilePath);

  {
    smtk::operation::WriteResource::Ptr writeOp =
      operationManager->create<smtk::operation::WriteResource>();

    test(writeOp != nullptr, "No write operator");

    writeOp->parameters()->associate(polygonResource);

    smtk::operation::Operation::Result writeOpResult = writeOp->operate();
    test(
      writeOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Write operator failed");

    smtk::operation::ReadResource::Ptr readOp =
      operationManager->create<smtk::operation::ReadResource>();

    test(readOp != nullptr, "No read operator");

    readOp->parameters()->findFile("filename")->setValue(writeFilePath);

    smtk::operation::Operation::Result readOpResult = readOp->operate();
    test(
      readOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Read operator failed");

    smtk::session::polygon::Resource::Ptr polygonResource2 =
      smtk::dynamic_pointer_cast<smtk::session::polygon::Resource>(
        readOpResult->findResource("resourcesCreated")->value(0));

    cleanup(writeFilePath);

    smtk::model::Models models = polygonResource2->entitiesMatchingFlagsAs<smtk::model::Models>(
      smtk::model::MODEL_ENTITY, false);

    std::cout << "found " << models.size() << " models" << std::endl;
    if (models.empty())
      return 1;

    smtk::model::Model model2 = models[0];

    ValidateModelTopology(model2);
  }

  return 0;
}
