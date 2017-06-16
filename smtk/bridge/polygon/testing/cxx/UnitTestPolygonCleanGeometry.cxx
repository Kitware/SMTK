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
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"
#include <complex>
#include <set>

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

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  smtk::model::SessionRef session = manager->createSession("polygon");
  smtk::model::OperatorPtr myOp = session.op("create model");
  smtk::model::OperatorResult res = myOp->operate();
  smtk::model::EntityRef myModel = res->findModelEntity("created")->value();

  // Create two intersecting edges
  std::cout << "Creating two intersecting edges" << std::endl;
  // Associate model with operation
  myOp = session.op("create edge from points");
  test(myOp != nullptr, "No create edge from points operator");
  test(myOp->specification()->associateEntity(myModel), "Could not associate model");
  for (int i = 0; i < numEdges; ++i)
  {
    // Specify the points
    smtk::attribute::IntItemPtr pointGeometry = myOp->specification()->findInt("pointGeometry");
    test(pointGeometry != nullptr, "Could not find pointGeometry");
    test(pointGeometry->setValue(numCoordsPerPoint), "Could not set pointGeometry");
    smtk::attribute::GroupItem::Ptr pointsInfo = myOp->specification()->findGroup("2DPoints");
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
    res = myOp->operate();
    test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
      "Create edge from points operator failed");
  }

  smtk::model::Model modelCreated = static_cast<smtk::model::Model>(myModel);
  std::set<smtk::model::Edge> edges;
  std::set<smtk::model::Vertex> vertices;
  findEdgesAndVertices(myModel, edges, vertices);
  std::cout << "Before clean geometry operation, number of edges: " << edges.size()
            << ", number of vertices: " << vertices.size() << std::endl;
  test(static_cast<int>(edges.size()) == numEdges, "Incorrect number of edges");
  test(static_cast<int>(vertices.size()) == numEdges * numPointsPerEdge,
    "Incorrect number of vertices");

  // Clean geometry
  myOp = session.op("clean geometry");
  test(myOp != nullptr, "No clean geometry operator");
  for (const auto& e : edges)
  {
    test(myOp->specification()->associateEntity(e), "Could not associate model");
  }

  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Clean geometry operator failed");

  // Verify the result
  findEdgesAndVertices(myModel, edges, vertices);
  std::cout << "After clean geometry operation, number of edges: " << edges.size()
            << ", number of vertices: " << vertices.size() << std::endl;
  test(static_cast<int>(edges.size()) == numEdges * 2, "Incorrect number of edges");
  test(static_cast<int>(vertices.size()) == numEdges * numPointsPerEdge + 1,
    "Incorrect number of vertices");

  return 0;
}

// This macro ensures the polygon session library is loaded into the executable
smtkComponentInitMacro(smtk_polygon_session)
