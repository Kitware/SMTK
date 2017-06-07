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

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  smtk::model::SessionRef session = manager->createSession("polygon");
  smtk::model::OperatorPtr myOp = session.op("create model");
  smtk::model::OperatorResult res = myOp->operate();
  smtk::model::EntityRef myModel = res->findModelEntity("created")->value();

  // Associate model with operation
  myOp = session.op("create edge from points");
  test(myOp != nullptr, "No create edge from points operator");
  test(myOp->specification()->associateEntity(myModel), "Could not associate model");

  // Specify the points
  std::cout << "Creating edge using (-0.5, -0.5), (0.5, -0.5) and (0.5, 0.5)" << std::endl;
  smtk::attribute::IntItemPtr pointGeometry = myOp->specification()->findInt("pointGeometry");
  test(pointGeometry != nullptr, "Could not find pointGeometry");
  test(pointGeometry->setValue(numCoordsPerPoint), "Could not set pointGeometry");
  smtk::attribute::GroupItem::Ptr pointsInfo = myOp->specification()->findGroup("2DPoints");
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
  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Create edge from points operator failed");

  // Check the created edge and vertices
  smtk::model::Model modelCreated = static_cast<smtk::model::Model>(myModel);
  smtk::model::Edges edges;
  smtk::model::Vertices verts;
  for (auto& cell : modelCreated.cells())
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

// This macro ensures the polygon session library is loaded into the executable
smtkComponentInitMacro(smtk_polygon_session)
