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
#include <set>

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/bridge/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"
#include "smtk/bridge/polygon/operators/CreateVertices.h"
#include "smtk/bridge/polygon/operators/DemoteVertex.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/operation/Manager.h"

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
    operationManager->registerOperation<smtk::bridge::polygon::CreateVertices>(
      "smtk::bridge::polygon::CreateVertices");
    operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromVertices>(
      "smtk::bridge::polygon::CreateEdgeFromVertices");
    operationManager->registerOperation<smtk::bridge::polygon::CreateFacesFromEdges>(
      "smtk::bridge::polygon::CreateFacesFromEdges");
    operationManager->registerOperation<smtk::bridge::polygon::DemoteVertex>(
      "smtk::bridge::polygon::DemoteVertex");
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

  // Create a "create vertices" operator
  smtk::bridge::polygon::CreateVertices::Ptr createVerticesOp =
    operationManager->create<smtk::bridge::polygon::CreateVertices>();

  createVerticesOp->parameters()->associateEntity(model->referenceAs<smtk::model::Model>());

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

  smtk::operation::Operation::Result res = createVerticesOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Create vertices operator failed");

  smtk::model::Model modelCreated = model->referenceAs<smtk::model::Model>();
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
    smtk::bridge::polygon::CreateEdgeFromVertices::Ptr createEdgeFromVerticesOp =
      operationManager->create<smtk::bridge::polygon::CreateEdgeFromVertices>();
    test(createEdgeFromVerticesOp != nullptr, "No create edge from vertices operator");
    test(createEdgeFromVerticesOp->parameters()->associateEntity(verts[i]),
      "Could not associate vertex");
    test(createEdgeFromVerticesOp->parameters()->associateEntity(verts[endVert[i]]),
      "Could not associate vertex");
    res = createEdgeFromVerticesOp->operate();
    test(res->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
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
  smtk::bridge::polygon::CreateFacesFromEdges::Ptr createFacesFromEdgesOp =
    operationManager->create<smtk::bridge::polygon::CreateFacesFromEdges>();
  test(createFacesFromEdgesOp != nullptr, "No create faces from edges operator");
  for (int i = 0; i < numEdges; ++i)
  {
    test(
      createFacesFromEdgesOp->parameters()->associateEntity(edges[i]), "Could not associate edge");
  }
  res = createFacesFromEdgesOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
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
  smtk::bridge::polygon::DemoteVertex::Ptr demoteVertexOp =
    operationManager->create<smtk::bridge::polygon::DemoteVertex>();
  test(demoteVertexOp != nullptr, "No demote vertex operator");
  test(demoteVertexOp->parameters()->associateEntity(verts[numVertices - 1]),
    "Could not associate vertex");
  res = demoteVertexOp->operate();
  test(res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
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
      std::cerr << "There should not be any dangling edge" << std::endl;
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
