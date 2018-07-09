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
#include "smtk/model/Edge.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"
#include <complex>
#include <set>

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/CleanGeometry.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/bridge/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/operation/Manager.h"

void findEdgesAndVertices(const smtk::model::Model& model, std::set<smtk::model::Edge>& edges,
  std::set<smtk::model::Vertex>& vertices)
{
  edges.clear();
  vertices.clear();
  for (auto& cell : model.cells())
  {
    if (smtk::model::isEdge(cell.entityFlags()))
    {
      smtk::model::Edge e = static_cast<smtk::model::Edge>(cell);
      edges.insert(e);
      smtk::model::Vertices vertsOnEdge = e.vertices();
      vertices.insert(vertsOnEdge.begin(), vertsOnEdge.end());
    }
  }
}

int UnitTestPolygonCleanGeometry(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // In this test, two intersecting edges will be created from points
  // then clean geometry will be called.
  // 4 edges and 5 vertices are expected after the operation.
  const int numCoordsPerPoint = 2;
  const int numEdges = 2;
  const std::vector<double> points{ -1., -1., 1., 1., 1., -1., -1., 1. };
  const int numPointsPerEdge = static_cast<int>(points.size()) / numCoordsPerPoint / numEdges;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::bridge::polygon::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register operators to the operation manager
  {
    operationManager->registerOperation<smtk::bridge::polygon::CleanGeometry>(
      "smtk::bridge::polygon::CleanGeometry");
    operationManager->registerOperation<smtk::bridge::polygon::CreateModel>(
      "smtk::bridge::polygon::CreateModel");
    operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromPoints>(
      "smtk::bridge::polygon::CreateEdgeFromPoints");
    operationManager->registerOperation<smtk::bridge::polygon::CreateFacesFromEdges>(
      "smtk::bridge::polygon::CreateFacesFromEdges");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an "create model" operator
  smtk::bridge::polygon::CreateModel::Ptr createOp =
    operationManager->create<smtk::bridge::polygon::CreateModel>();

  // Apply the operation and check the result
  smtk::operation::Operation::Result createOpResult = createOp->operate();

  // Retrieve the resulting model item
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      createOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  // Create two intersecting edges
  std::cout << "Creating two intersecting edges" << std::endl;

  // Create a "create edge from points" operator
  smtk::bridge::polygon::CreateEdgeFromPoints::Ptr createEdgeFromPointsOp =
    operationManager->create<smtk::bridge::polygon::CreateEdgeFromPoints>();

  createEdgeFromPointsOp->parameters()->associateEntity(model->referenceAs<smtk::model::Model>());

  for (int i = 0; i < numEdges; ++i)
  {
    // Specify the points
    smtk::attribute::IntItemPtr pointGeometry =
      createEdgeFromPointsOp->parameters()->findInt("pointGeometry");
    test(pointGeometry != nullptr, "Could not find pointGeometry");
    test(pointGeometry->setValue(numCoordsPerPoint), "Could not set pointGeometry");
    smtk::attribute::GroupItem::Ptr pointsInfo =
      createEdgeFromPointsOp->parameters()->findGroup("2DPoints");
    test(pointsInfo->setNumberOfGroups(numPointsPerEdge), "Could not set number of points");
    for (int j = 0; j < numPointsPerEdge; ++j)
    {
      smtk::attribute::ItemPtr point = pointsInfo->find(j, "points");
      test(point != nullptr, "Could not find point");
      for (int k = 0; k < numCoordsPerPoint; ++k)
      {
        test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(point)->setValue(
               k, points[(i * numPointsPerEdge + j) * numCoordsPerPoint + k]),
          "Setting points failed");
      }
    }
    // Apply the operation
    smtk::operation::Operation::Result res = createEdgeFromPointsOp->operate();
    test(res->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Create edge from points operator failed");
  }

  smtk::model::Model modelCreated = model->referenceAs<smtk::model::Model>();
  std::set<smtk::model::Edge> edges;
  std::set<smtk::model::Vertex> vertices;
  findEdgesAndVertices(modelCreated, edges, vertices);
  std::cout << "Before clean geometry operation, number of edges: " << edges.size()
            << ", number of vertices: " << vertices.size() << std::endl;
  test(static_cast<int>(edges.size()) == numEdges, "Incorrect number of edges");
  test(static_cast<int>(vertices.size()) == numEdges * numPointsPerEdge,
    "Incorrect number of vertices");

  // Clean geometry
  smtk::bridge::polygon::CleanGeometry::Ptr cleanGeometryOp =
    operationManager->create<smtk::bridge::polygon::CleanGeometry>();
  test(cleanGeometryOp != nullptr, "No clean geometry operator");
  for (const auto& e : edges)
  {
    test(cleanGeometryOp->parameters()->associateEntity(e), "Could not associate model");
  }

  smtk::operation::Operation::Result res = cleanGeometryOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Clean geometry operator failed");

  // Verify the result
  findEdgesAndVertices(modelCreated, edges, vertices);
  std::cout << "After clean geometry operation, number of edges: " << edges.size()
            << ", number of vertices: " << vertices.size() << std::endl;
  test(static_cast<int>(edges.size()) == numEdges * 2, "Incorrect number of edges");
  test(static_cast<int>(vertices.size()) == numEdges * numPointsPerEdge + 1,
    "Incorrect number of vertices");

  return 0;
}
