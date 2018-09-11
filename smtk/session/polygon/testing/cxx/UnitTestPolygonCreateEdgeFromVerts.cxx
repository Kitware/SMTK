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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"
#include <complex>

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/session/polygon/operators/CreateModel.h"
#include "smtk/session/polygon/operators/CreateVertices.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/operation/Manager.h"

namespace
{
static const double tolerance = 1.e-5;
}

int UnitTestPolygonCreateEdgeFromVerts(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // In this test, 2 two-dimensional vertices are going to be created,
  // and an edge is going to be created from them
  const int numCoordsPerVertex = 2;
  const std::vector<double> points{ -1, -1, 1, 1 };
  const int numVertices = static_cast<int>(points.size()) / numCoordsPerVertex;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::session::polygon::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register operators to the operation manager
  {
    operationManager->registerOperation<smtk::session::polygon::CreateModel>(
      "smtk::session::polygon::CreateModel");
    operationManager->registerOperation<smtk::session::polygon::CreateVertices>(
      "smtk::session::polygon::CreateVertices");
    operationManager->registerOperation<smtk::session::polygon::CreateEdgeFromVertices>(
      "smtk::session::polygon::CreateEdgeFromVertices");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an "create model" operator
  smtk::session::polygon::CreateModel::Ptr createOp =
    operationManager->create<smtk::session::polygon::CreateModel>();

  // Apply the operation and check the result
  smtk::operation::Operation::Result createOpResult = createOp->operate();

  // Retrieve the resulting model item
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      createOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  // Create a "create vertices" operator
  smtk::session::polygon::CreateVertices::Ptr createVerticesOp =
    operationManager->create<smtk::session::polygon::CreateVertices>();

  createVerticesOp->parameters()->associateEntity(model->referenceAs<smtk::model::Model>());

  // Specify the vertices
  std::cout << "Creating vertices at (-1, -1) and (1, 1)" << std::endl;
  smtk::attribute::IntItemPtr pointDimension =
    createVerticesOp->parameters()->findInt("point dimension");
  test(pointDimension != nullptr, "Could not find point dimension");
  test(pointDimension->setValue(numCoordsPerVertex), "Could not set point dimension");
  smtk::attribute::GroupItem::Ptr pointsInfo =
    createVerticesOp->parameters()->findGroup("2d points");
  test(pointsInfo->setNumberOfGroups(numVertices), "Could not set number of points");
  for (int i = 0; i < numVertices; ++i)
  {
    smtk::attribute::ItemPtr point = pointsInfo->find(i, "points");
    test(point != nullptr, "Could not find point");
    for (int j = 0; j < numCoordsPerVertex; ++j)
    {
      test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(point)->setValue(
             j, points[i * numCoordsPerVertex + j]),
        "Setting points failed");
    }
  }

  // Apply the create vertices operation
  smtk::operation::Operation::Result res = createVerticesOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Create vertices operator failed");

  // Verify the vertices are correctly created
  smtk::model::Model modelCreated = model->referenceAs<smtk::model::Model>();
  smtk::model::Vertices verts;
  for (auto& cell : modelCreated.cells())
  {
    if (smtk::model::isVertex(cell.entityFlags()))
    {
      smtk::model::Vertex vert = static_cast<smtk::model::Vertex>(cell);
      verts.push_back(vert);
      std::cout << "Found vertex at: (" << vert.coordinates()[0] << ", " << vert.coordinates()[1]
                << ")\n";
    }
  }
  test(static_cast<int>(verts.size()) == numVertices, "Missing vertices");
  for (int i = 0; i < numVertices; ++i)
  {
    for (int j = 0; j < numCoordsPerVertex; ++j)
    {
      test(std::abs(verts[i].coordinates()[j] - points[i * numCoordsPerVertex + j]) < tolerance,
        "Incorrect vertex coordinates");
    }
  }

  // Create an edge from the vertices
  // Associate model with create edge operaton
  std::cout << "Creating edge from vertices" << std::endl;

  // Create a "create edge from vertices" operator
  smtk::session::polygon::CreateEdgeFromVertices::Ptr createEdgeFromVerticesOp =
    operationManager->create<smtk::session::polygon::CreateEdgeFromVertices>();

  test(createEdgeFromVerticesOp != nullptr, "No create edge from vertices operator");
  for (const auto& v : verts)
    test(createEdgeFromVerticesOp->parameters()->associateEntity(v), "Could not associate vertex");

  // Apply the create edge from vertices operation
  res = createEdgeFromVerticesOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Create edge from vertices operator failed");

  // Vertify that the edge is created
  int numberOfEdges = 0;
  for (auto& cell : modelCreated.cells())
  {
    numberOfEdges += (smtk::model::isEdge(cell.entityFlags()) ? 1 : 0);
  }
  test(numberOfEdges == 1, "Incorrect number of edges");

  return 0;
}
