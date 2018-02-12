//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PythonAutoInit.h"

#include "smtk/bridge/multiscale/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/mesh/RegisterSession.h"
#include "smtk/bridge/mesh/operators/ImportOperator.h"
#include "smtk/bridge/multiscale/RegisterSession.h"
#include "smtk/bridge/multiscale/Session.h"
#include "smtk/bridge/multiscale/operators/Revolve.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

namespace
{

std::string afrlRoot = std::string(AFRL_DIR);
std::string dataRoot = SMTK_DATA_DIR;
}

int RevolveOp(int argc, char* argv[])
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
    smtk::bridge::multiscale::registerResources(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register mesh and multiscale operators to the operation manager
  {
    smtk::bridge::mesh::registerOperations(operationManager);
    smtk::bridge::multiscale::registerOperations(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an import operator
  smtk::operation::NewOp::Ptr importOp =
    operationManager->create<smtk::bridge::mesh::ImportOperator>();
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(dataRoot);
  importFilePath += "/mesh/2d/ImportFromDEFORM.h5m";

  importOp->parameters()->findFile("filename")->setValue(importFilePath);

  smtk::operation::NewOp::Result importOpResult = importOp->operate();

  if (importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
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

  smtk::operation::NewOp::Ptr revolveOp =
    operationManager->create<smtk::bridge::multiscale::Revolve>();
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

  smtk::operation::NewOp::Result revolveOpResult = revolveOp->operate();
  if (revolveOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "Revolve operator failed\n";
    return 1;
  }

  return 0;
}
