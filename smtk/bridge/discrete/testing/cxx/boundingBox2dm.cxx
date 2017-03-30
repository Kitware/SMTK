//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Vertex.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/UUID.h"

#include "smtk/bridge/discrete/Session.h"

using namespace smtk::model;

int main(int argc, char* argv[])
{
  // basic check info
  if (argc == 1)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }

  // Create a model manager
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  // Identify available sessions
  std::cout << "Available sessions\n";
  typedef smtk::model::StringList StringList;
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  // Create a new discrete session
  smtk::bridge::discrete::Session::Ptr session = smtk::bridge::discrete::Session::create();
  manager->registerSession(session);

  // Identify available operators
  std::cout << "Available cmb operators in discrete session\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
  {
    std::cout << "  " << *it << "\n";
  }
  std::cout << "\n";

  // read the data
  smtk::model::OperatorPtr readOp = session->op("import");
  if (!readOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  // read the file
  readOp->specification()->findFile("filename")->setValue(std::string(argv[1]));
  std::cout << "Importing " << argv[1] << "\n";
  smtk::model::OperatorResult opresult = readOp->operate();
  if (opresult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Read operator failed\n";
    return 1;
  }

  smtk::model::Model modelSimple2dm = opresult->findModelEntity("created")->value();

  if (!modelSimple2dm.isValid())
  {
    std::cerr << "Reading simple 2dm file failed!\n";
    return 1;
  }

  // get edge/face info
  EntityRefs groups = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  std::cout << "Before creation, group size is: " << groups.size() << endl;
  EntityRefs edges = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
  std::cout << "Edges inside the model is:\n";
  for (EntityRefs::iterator it = edges.begin(); it != edges.end(); ++it)
  {
    std::cout << "  " << it->name() << " \n";
    if (!it->hasFloatProperty(SMTK_BOUNDING_BOX_PROP))
    {
      std::cerr << "edge has no bounding box!\n";
      return 1;
    }
  }
  std::cout << std::endl;
  test(edges.size() == 10, "Expecting 10 edges");

  return 0;
}
