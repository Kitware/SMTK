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
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"
#include <complex>

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/session/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/session/polygon/operators/CreateModel.h"

#include "smtk/operation/Manager.h"

int UnitTestPolygonCreateFacesFromEdges(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // In this test, 1 face is going to be created from a unit square
  const int numCoordsPerPoint = 2;
  const std::vector<double> points{ -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5 };
  const int numPoints = static_cast<int>(points.size()) / numCoordsPerPoint;

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
    operationManager->registerOperation<smtk::session::polygon::CreateEdgeFromPoints>(
      "smtk::session::polygon::CreateEdgeFromPoints");
    operationManager->registerOperation<smtk::session::polygon::CreateFacesFromEdges>(
      "smtk::session::polygon::CreateFacesFromEdges");
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

  // Create a "create edge from points" operator
  smtk::session::polygon::CreateEdgeFromPoints::Ptr createEdgeFromPointsOp =
    operationManager->create<smtk::session::polygon::CreateEdgeFromPoints>();

  createEdgeFromPointsOp->parameters()->associateEntity(model->referenceAs<smtk::model::Model>());

  // Specify the points
  std::cout << "Creating a unit square centering at the origin" << std::endl;
  smtk::attribute::IntItemPtr pointGeometry =
    createEdgeFromPointsOp->parameters()->findInt("pointGeometry");
  test(pointGeometry != nullptr, "Could not find pointGeometry");
  test(pointGeometry->setValue(numCoordsPerPoint), "Could not set pointGeometry");
  smtk::attribute::GroupItem::Ptr pointsInfo =
    createEdgeFromPointsOp->parameters()->findGroup("2DPoints");
  test(pointsInfo->setNumberOfGroups(numPoints), "Could not set number of points");
  for (int i = 0; i < numPoints; ++i)
  {
    smtk::attribute::ItemPtr point = pointsInfo->find(i, "points");
    test(point != nullptr, "Could not find point");
    for (int j = 0; j < numCoordsPerPoint; ++j)
    {
      test(
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(point)->setValue(
          j, points[i * numCoordsPerPoint + j]),
        "Setting points failed");
    }
  }

  // Apply the create edge from ponits operation
  smtk::operation::Operation::Result res = createEdgeFromPointsOp->operate();
  test(
    res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Create edge from points operator failed");

  // Check the created edge and vertices
  smtk::model::Model modelCreated = model->referenceAs<smtk::model::Model>();
  smtk::model::Edges edges;
  for (auto& cell : modelCreated.cells())
  {
    if (smtk::model::isEdge(cell.entityFlags()))
    {
      smtk::model::Edge e = static_cast<smtk::model::Edge>(cell);
      edges.push_back(e);
      smtk::model::Vertices vertsOnEdge = e.vertices();
      test(static_cast<int>(vertsOnEdge.size()) == 0, "Closed edge should not have vertices");
    }
  }
  test(static_cast<int>(edges.size()) == 1, "Incorrect number of edges");

  // Create a face from the loop

  // Create a "create face from edges" operator
  smtk::session::polygon::CreateFacesFromEdges::Ptr createFacesFromEdgesOp =
    operationManager->create<smtk::session::polygon::CreateFacesFromEdges>();

  test(createFacesFromEdgesOp != nullptr, "No create faces from edges operator");
  test(createFacesFromEdgesOp->parameters()->associateEntity(edges[0]), "Could not associate edge");
  res = createFacesFromEdgesOp->operate();
  test(
    res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Create faces from edges operator failed");

  // Verify that face has been created
  int numberOfFaces = 0;
  for (auto& cell : modelCreated.cells())
  {
    numberOfFaces += (smtk::model::isFace(cell.entityFlags()) ? 1 : 0);
  }
  test(numberOfFaces == 1, "Incorrect number of faces");

  return 0;
}
