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
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"
#include <complex>
#include <set>

int UnitTestPolygonDemoteVertex(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Here is what is going to happen: first create 4 vertices,
  // then create 4 edges from the vertices with the same start and end vertices,
  // then create 1 face from the edges. At this point there should be 1 face,
  // 4 edges and 4 vertices in the model. Now demote 1 vertex, there should be
  // 1 face, 3 edges and 3 vertices remained

  const int numCoordsPerVertex = 2;
  const std::vector<double> points{ 0., 0., 1., 0., 1., 1., 0., 1. };
  const int numVertices = static_cast<int>(points.size()) / numCoordsPerVertex;
  const int numEdges = 4;
  const int numFaces = 1;
  const std::vector<int> endVert{ 1, 2, 3, 0 }; // a lookup table for the end vertex of an edge

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  smtk::model::SessionRef session = manager->createSession("polygon");
  smtk::model::OperatorPtr myOp = session.op("create model");
  smtk::model::OperatorResult res = myOp->operate();
  smtk::model::EntityRef myModel = res->findModelEntity("created")->value();

  // Create and verify vertices
  myOp = session.op("create vertices");
  test(myOp != nullptr, "No create vertices operator");
  test(myOp->specification()->associateEntity(myModel), "Could not associate model");

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

  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Create vertices operator failed");

  smtk::model::Model modelCreated = static_cast<smtk::model::Model>(myModel);
  smtk::model::Vertices verts;
  for (auto& cell : modelCreated.cells())
  {
    if (smtk::model::isVertex(cell.entityFlags()))
    {
      smtk::model::Vertex vert = static_cast<smtk::model::Vertex>(cell);
      verts.push_back(vert);
      std::cout << "Created vertex at: (" << vert.coordinates()[0] << ", " << vert.coordinates()[1]
                << ")\n";
    }
  }
  test(static_cast<int>(verts.size()) == numVertices, "Missing vertices");

  // Create and verify edges using vertices 0-1, 1-2, 2-3, 3-0
  for (int i = 0; i < numEdges; ++i)
  {
    myOp = session.op("create edge from vertices");
    test(myOp != nullptr, "No create edge from vertices operator");
    test(myOp->specification()->associateEntity(verts[i]), "Could not associate vertex");
    test(myOp->specification()->associateEntity(verts[endVert[i]]), "Could not associate vertex");
    res = myOp->operate();
    test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
      "Create edge from vertices operator failed");
  }

  smtk::model::Edges edges;
  for (auto& cell : modelCreated.cells())
  {
    if (smtk::model::isEdge(cell.entityFlags()))
    {
      edges.push_back(static_cast<smtk::model::Edge>(cell));
    }
  }
  test(static_cast<int>(edges.size()) == numEdges, "Incorrect number of edges");

  // Create and verify face
  myOp = session.op("create faces from edges");
  test(myOp != nullptr, "No create faces from edges operator");
  for (int i = 0; i < numEdges; ++i)
  {
    test(myOp->specification()->associateEntity(edges[i]), "Could not associate edge");
  }
  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Create faces from edges operator failed");

  int numberOfFaces = 0;
  for (auto& cell : modelCreated.cells())
  {
    numberOfFaces += (smtk::model::isFace(cell.entityFlags()) ? 1 : 0);
  }
  test(numberOfFaces == numFaces, "Incorrect number of faces");

  std::cout << "Before demoting vertex, there are " << numFaces << " faces, " << numEdges
            << " edges, " << numVertices << " vertices in the model" << std::endl;

  // Demote the last vertex
  myOp = session.op("demote vertex");
  test(myOp != nullptr, "No demote vertex operator");
  test(
    myOp->specification()->associateEntity(verts[numVertices - 1]), "Could not associate vertex");
  res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Demote vertex operator failed");

  // Check the state of the model after operation
  std::set<smtk::model::Face> remainingFaces;
  std::set<smtk::model::Edge> remainingEdges;
  std::set<smtk::model::Vertex> remainingVertices;
  for (auto& cell : modelCreated.cells())
  {
    if (smtk::model::isFace(cell.entityFlags()))
    {
      smtk::model::Face f = static_cast<smtk::model::Face>(cell);
      remainingFaces.insert(f);
      smtk::model::Edges edgesOnFace = f.edges();
      for (const auto& e : edgesOnFace)
      {
        remainingEdges.insert(e);
        smtk::model::Vertices vertices = e.vertices();
        remainingVertices.insert(vertices.begin(), vertices.end());
      }
    }
    else if (smtk::model::isEdge(cell.entityFlags()))
    {
      std::cerr << "There should not be any dangling endge" << std::endl;
      return 1;
    }
    else if (smtk::model::isVertex(cell.entityFlags()))
    {
      std::cerr << "There should not be any dangling vertex" << std::endl;
      return 1;
    }
  }

  std::cout << "After demoting vertex, there are " << remainingFaces.size() << " faces, "
            << remainingEdges.size() << " edges, " << remainingVertices.size()
            << " vertices in the model" << std::endl;

  test(static_cast<int>(remainingFaces.size()) == numFaces,
    "Incorrect number of faces after demoting vertex");
  test(static_cast<int>(remainingEdges.size()) == numEdges - 1,
    "Incorrect number of edges after demoting vertex");
  test(static_cast<int>(remainingVertices.size()) == numVertices - 1,
    "Incorrect number of vertices after demoting vertex");

  return 0;
}

// This macro ensures the polygon session library is loaded into the executable
smtkComponentInitMacro(smtk_polygon_session)
