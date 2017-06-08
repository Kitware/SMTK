//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/CellEntity.h"
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

int UnitTestPolygonCreateEdgeFromVertices(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // In this test, 2 two-dimensional vertices are going to be created,
  // and an edge is going to be created from them
  const int numCoordsPerVertex = 2;
  const std::vector<double> points{ -1, -1, 1, 1 };
  const int numVertices = static_cast<int>(points.size()) / numCoordsPerVertex;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  smtk::model::SessionRef session = manager->createSession("polygon");
  smtk::model::OperatorPtr myOp = session.op("create model");
  smtk::model::OperatorResult res = myOp->operate();
  smtk::model::EntityRef myModel = res->findModelEntity("created")->value();

  // Create two vertices
  // Associate model with create vertices operation
  myOp = session.op("create vertices");
  test(myOp != nullptr, "No create vertices operator");
  test(myOp->specification()->associateEntity(myModel), "Could not associate model");

  // Specify the vertices
  std::cout << "Creating vertices at (-1, -1) and (1, 1)" << std::endl;
  smtk::attribute::IntItemPtr pointDimension = myOp->specification()->findInt("point dimension");
  test(pointDimension != nullptr, "Could not find point dimension");
  test(pointDimension->setValue(numCoordsPerVertex), "Could not set point dimension");
  smtk::attribute::GroupItem::Ptr pointsInfo = myOp->specification()->findGroup("2d points");
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
  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Create vertices operator failed");

  // Verify the vertices are correctly created
  smtk::model::Model modelCreated = static_cast<smtk::model::Model>(myModel);
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
  myOp = session.op("create edge from vertices");
  test(myOp != nullptr, "No create edge from vertices operator");
  for (const auto& v : verts)
    test(myOp->specification()->associateEntity(v), "Could not associate vertex");

  // Apply the create edge from vertices operation
  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
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

// This macro ensures the polygon session library is loaded into the executable
smtkComponentInitMacro(smtk_polygon_session)
