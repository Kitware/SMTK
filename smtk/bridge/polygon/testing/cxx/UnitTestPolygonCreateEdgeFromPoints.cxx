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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"
#include <complex>

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/operation/Manager.h"

namespace
{
static const double tolerance = 1.e-5;
}

int UnitTestPolygonCreateEdgeFromPoints(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // In this test, 3 two-dimensional points are going to be created
  const int numCoordsPerPoint = 2;
  const std::vector<double> points{ -0.5, -0.5, 0.5, -0.5, 0.5, 0.5 };
  const int numPoints = static_cast<int>(points.size()) / numCoordsPerPoint;

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
    operationManager->registerOperation<smtk::bridge::polygon::CreateModel>(
      "smtk::bridge::polygon::CreateModel");
    operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromPoints>(
      "smtk::bridge::polygon::CreateEdgeFromPoints");
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

  // Create a "create edge from points" operator
  smtk::bridge::polygon::CreateEdgeFromPoints::Ptr createEdgeFromPointsOp =
    operationManager->create<smtk::bridge::polygon::CreateEdgeFromPoints>();

  createEdgeFromPointsOp->parameters()->associateEntity(model->referenceAs<smtk::model::Model>());

  // test(createEdgeFromPointsOp->parameters()->associateEntity(
  //        model->referenceAs<smtk::model::Model>()), "Could not associate model");

  // Specify the points
  std::cout << "Creating edge using (-0.5, -0.5), (0.5, -0.5) and (0.5, 0.5)" << std::endl;

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
      test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(point)->setValue(
             j, points[i * numCoordsPerPoint + j]),
        "Setting points failed");
    }
  }

  // Apply the operation
  smtk::operation::Operation::Result res = createEdgeFromPointsOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Create edge from points operator failed");

  // Check the created edge and vertices
  smtk::model::Model createdModel = model->referenceAs<smtk::model::Model>();
  smtk::model::Edges edges;
  smtk::model::Vertices verts;
  for (auto& cell : createdModel.cells())
  {
    if (smtk::model::isEdge(cell.entityFlags()))
    {
      smtk::model::Edge e = static_cast<smtk::model::Edge>(cell);
      edges.push_back(e);
      smtk::model::Vertices vertsOnEdge = e.vertices();
      verts.insert(verts.end(), vertsOnEdge.begin(), vertsOnEdge.end());
    }
  }
  test(edges.size() == 1, "Incorrect number of edges");
  test(verts.size() == edges.size() * numCoordsPerPoint, "Incorrect number of vertices");
  test(std::abs(verts[0].coordinates()[0] + 0.5) < tolerance, "Incorrect coordinates of vertex");
  test(std::abs(verts[0].coordinates()[1] + 0.5) < tolerance, "Incorrect coordinates of vertex");
  test(std::abs(verts[1].coordinates()[0] - 0.5) < tolerance, "Incorrect coordinates of vertex");
  test(std::abs(verts[1].coordinates()[1] - 0.5) < tolerance, "Incorrect coordinates of vertex");

  return 0;
}
