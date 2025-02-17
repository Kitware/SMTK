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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/session/polygon/Registrar.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/operators/ImportPPG.h"

#include <boost/filesystem.hpp>

#include <string>

namespace
{
const int OP_SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);
const std::string resourceFilename("import-ppg.smtk");

const std::string ppgText = "# Test2D geometry\n"
                            "\n"
                            "# Vertices 1-8 for the outer polygon\n"
                            "v  0.0  2.0   # first vertex 1\n"
                            "v  1.0  0.0\n"
                            "v  9.0  0.0\n"
                            "v  9.0  2.0\n"
                            "v  8.0  4.0\n"
                            "v  6.0  5.0\n"
                            "v  3.0  5.0\n"
                            "v  1.0  4.0\n"
                            "\n"
                            "# Vertices 9-12 for the inner polygon (hole)\n"
                            "v  7.0  1.0\n"
                            "v  8.0  1.0\n"
                            "v  8.0  2.0\n"
                            "v  7.0  2.0\n"
                            "\n"
                            "# Outer polygon\n"
                            "f 1 2 3 4 5 6 7 8  # first face\n"
                            "\n"
                            "# Inner polygon (hole)\n"
                            "h 9 10 11 12\n";
} // namespace

int UnitTestPolygonImportPPG(int /*argc*/, char* /*argv*/[])
{
  // Delete existing file
  boost::filesystem::path folderPath(SMTK_SCRATCH_DIR);
  boost::filesystem::path smtkPath = folderPath / resourceFilename;
  if (boost::filesystem::exists(smtkPath))
  {
    boost::filesystem::remove(smtkPath);
  }

  // Initialize managers
  smtk::resource::ManagerPtr resManager = smtk::resource::Manager::create();
  smtk::operation::ManagerPtr opManager = smtk::operation::Manager::create();

  auto polygonRegistry =
    smtk::plugin::addToManagers<smtk::session::polygon::Registrar>(resManager, opManager);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(opManager);

  opManager->registerResourceManager(resManager);

  // Create an "import" operation
  auto importOp = opManager->create<smtk::session::polygon::ImportPPG>();
  test(importOp != nullptr, "No import operator");
  importOp->parameters()->findString("string")->setIsEnabled(true);
  importOp->parameters()->findString("string")->setValue(ppgText);

  // Run the operation
  auto importResult = importOp->operate();
  int importOutcome = importResult->findInt("outcome")->value();
  smtkTest(
    importOutcome == OP_SUCCEEDED,
    "Import operation failed. Returned outcome " << std::to_string(importOutcome) << ". "
                                                 << importOp->log().convertToString());

  // Get the model resource
  auto resourceItem = importResult->findResource("resourcesCreated");
  auto resource = resourceItem->value();
  test(resource != nullptr, "Resource was not found.");
  auto modelResource = std::dynamic_pointer_cast<smtk::model::Resource>(resource);
  test(modelResource != nullptr, "Model resource not found.");

  // Write model resource to file
  std::cout << "Writing " << smtkPath << std::endl;
  auto writeOp = opManager->create<smtk::operation::WriteResource>();
  test(writeOp != nullptr, "No write operator");
  writeOp->parameters()->associate(resource);
  writeOp->parameters()->findFile("filename")->setIsEnabled(true);
  writeOp->parameters()->findFile("filename")->setValue(smtkPath.string());
  auto writeResult = writeOp->operate();
  int writeOutcome = writeResult->findInt("outcome")->value();
  smtkTest(
    writeOutcome == OP_SUCCEEDED,
    "Write operation failed. Returned outcome " << std::to_string(writeOutcome) << ". "
                                                << writeOp->log().convertToString());

  // Check entity counts
  auto vList = modelResource->entitiesMatchingFlags(smtk::model::VERTEX);
  smtkTest(vList.size() == 12, "Expected 12 model vertices not " << vList.size() << ".");
  auto eList = modelResource->entitiesMatchingFlags(smtk::model::EDGE);
  smtkTest(eList.size() == 12, "Expected 12 model edges not " << eList.size() << ".");
  auto fList = modelResource->entitiesMatchingFlags(smtk::model::FACE);
  smtkTest(fList.size() == 1, "Expected 1 model face not " << fList.size() << ".");

  return 0;
}
