//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/Vertex.h"

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

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  smtk::model::SessionRef session = manager->createSession("polygon");
  smtk::model::OperatorPtr myOp = session.op("import");
  test(myOp != nullptr, "No import operator");
  readFilePath += filename;
  myOp->specification()->findFile("filename")->setValue(readFilePath);
  std::cout << "Importing " << readFilePath << std::endl;
  smtk::model::OperatorResult res = myOp->operate();
  test(res->findInt("outcome")->value() == smtk::operation::Operator::OPERATION_SUCCEEDED,
    "Import operator failed");
  smtk::model::Model modelCreated =
    static_cast<smtk::model::Model>(res->findModelEntity("created")->value());
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
