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
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/operators/Import.h"

#include "smtk/operation/Manager.h"

namespace
{
std::string readFilePath(SMTK_DATA_DIR);
std::string filename("/model/2d/map/simple.map");
const int numEdgesExpected = 5;
const int numVerticesExpected = 5;
}

int UnitTestPolygonImport(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::session::polygon::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import and write operators to the operation manager
  {
    operationManager->registerOperation<smtk::session::polygon::Import>(
      "smtk::session::polygon::Import");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  // Create an "import" operator
  smtk::session::polygon::Import::Ptr importOp =
    operationManager->create<smtk::session::polygon::Import>();

  test(importOp != nullptr, "No import operator");
  readFilePath += filename;
  importOp->parameters()->findFile("filename")->setValue(readFilePath);
  std::cout << "Importing " << readFilePath << std::endl;

  smtk::operation::Operation::Result importOpResult = importOp->operate();
  test(importOpResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Import operator failed");
  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Model modelCreated =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  std::set<smtk::model::Edge> edges;
  std::set<smtk::model::Vertex> vertices;
  for (auto& cell : modelCreated.cells())
  {
    if (smtk::model::isEdge(cell.entityFlags()))
    {
      smtk::model::Edge e = static_cast<smtk::model::Edge>(cell);
      edges.insert(e);
      smtk::model::Vertices vertsOnEdge = e.vertices();
      vertices.insert(vertsOnEdge.begin(), vertsOnEdge.end());
    }
  }
  std::cout << "The imported model has " << edges.size() << " edges, and " << vertices.size()
            << " vertices" << std::endl;
  test(static_cast<int>(edges.size()) == numEdgesExpected, "Incorrect number of edges");
  test(static_cast<int>(vertices.size()) == numVerticesExpected, "Incorrect number of vertices");

  return 0;
}
