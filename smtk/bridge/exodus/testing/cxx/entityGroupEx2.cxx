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

#include "smtk/bridge/exodus/Session.h"

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
  smtk::bridge::exodus::Session::Ptr session = smtk::bridge::exodus::Session::create();
  manager->registerSession(session);

  // Identify available operators
  std::cout << "Available cmb operators in exodus session\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
  {
    std::cout << "  " << *it << "\n";
  }
  std::cout << "\n";

  // read the data
  smtk::model::OperatorPtr readOp = session->op("read");
  if (!readOp)
  {
    std::cerr << "No read operator\n";
    return 1;
  }

  // read file
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
    std::cerr << "Reading ex2 file failed!\n";
    return 1;
  }

  // get item info
  EntityRefs groups = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  int count = 0;
  EntityRef item1, item2, item3;
  std::cout << "Before creation, group size is: " << groups.size() << endl;
  for (EntityRefs::iterator it = groups.begin(); it != groups.end(); ++it)
  {
    if (count == 0)
      item1 = *it;
    if (count == 1)
      item2 = *it;
    if (count == 2)
      item3 = *it;
    count++;
  }

  // create entity group operator
  std::cout << "Create the entity group operator\n";
  smtk::model::OperatorPtr egOp = session->op("entity group");
  if (!egOp)
  {
    std::cerr << "No entity group operator!\n";
    return 1;
  }

  // Check the optypeItem value
  std::cout << "optypeItem current value is: "
            << egOp->specification()->findString("Operation")->value() << std::endl;

  egOp->specification()->findModelEntity("model")->setValue(modelSimple2dm);
  egOp->specification()->findString("group name")->setValue("test groups");
  egOp->specification()->findVoid("Edge");

  egOp->specification()->findModelEntity("cell to add")->setNumberOfValues(2);
  egOp->specification()->findModelEntity("cell to add")->setValue(item1);
  egOp->specification()->findModelEntity("cell to add")->appendValue(item2);

  smtk::model::OperatorResult egResult = egOp->operate();
  if (egResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Entity group operator failed!\n";
    return 1;
  }
  // print out the elements in the group
  groups = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  std::cout << "After creation, group size is: " << groups.size() << endl;
  std::cout << "The items in groups is:\n";
  for (EntityRefs::iterator it = groups.begin(); it != groups.end(); ++it)
  {
    std::cout << "  group name: " << it->name() << "\n";
  }
  std::cout << std::endl;

  // Modify group
  std::cout << "Start Modify group\n";
  egOp->specification()->findString("Operation")->setValue("Modify");
  std::cout << "optypeItem current value is: "
            << egOp->specification()->findString("Operation")->value() << std::endl;
  smtk::model::Group modifyGroup = *(groups.begin());
  egOp->specification()->findModelEntity("modify cell group")->setValue(modifyGroup);
  egOp->specification()->findModelEntity("cell to remove")->appendValue(item1);
  egOp->specification()->findModelEntity("cell to add")->setNumberOfValues(1);
  egOp->specification()->findModelEntity("cell to add")->setValue(item3);

  egResult = egOp->operate();
  if (egResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Entity group operator failed!\n";
    return 1;
  }

  groups = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  std::cout << "After Modify, group size is: " << groups.size() << endl;
  std::cout << "The items in groups is:\n";
  for (EntityRefs::iterator it = groups.begin(); it != groups.end(); ++it)
  {
    std::cout << "  " << it->name() << "\n";
  }
  std::cout << std::endl;

  // Remove group
  egOp->specification()->findString("Operation")->setValue("Remove");
  // Check the optypeItem value
  std::cout << "optypeItem current value is: "
            << egOp->specification()->findString("Operation")->value() << std::endl;
  groups = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  modifyGroup = *(groups.begin());
  egOp->specification()->findModelEntity("remove cell group")->setNumberOfValues(1);
  egOp->specification()->findModelEntity("remove cell group")->setValue(modifyGroup);

  egResult = egOp->operate();
  test(egResult->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Entity group operator failed!");

  // Print out the elements in the group
  groups = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  std::cout << "After removal, the number of groups is: " << groups.size() << endl;
  std::cout << "The groups are:\n";
  for (EntityRefs::iterator it = groups.begin(); it != groups.end(); ++it)
  {
    std::cout << "  " << it->name() << "\n";
  }
  std::cout << std::endl;
  test(groups.size() == 3, "Expecting 3 groups.");
  return 0;
}
