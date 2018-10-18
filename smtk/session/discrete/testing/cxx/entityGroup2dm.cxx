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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/UUID.h"

#include "smtk/session/discrete/Resource.h"
#include "smtk/session/discrete/Session.h"
#include "smtk/session/discrete/operators/EntityGroupOperation.h"
#include "smtk/session/discrete/operators/ImportOperation.h"

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"

#include "smtk/operation/Manager.h"

using namespace smtk::model;

int main(int argc, char* argv[])
{
  // basic check info
  if (argc == 1)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }

  // Create an import operator
  auto importOp = smtk::session::discrete::ImportOperation::create();
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  // Set the file path
  importOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  // Execute the operation
  smtk::operation::Operation::Result importOpResult = importOp->operate();

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  // Test for success
  if (importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  smtk::model::Resource::Ptr resource =
    std::static_pointer_cast<smtk::model::Resource>(model->resource());
  smtk::model::Model modelSimple2dm = model->referenceAs<smtk::model::Model>();

  if (!modelSimple2dm.isValid())
  {
    std::cerr << "Reading simple 2dm file failed!\n";
    return 1;
  }

  // get edge/face info
  EntityRefs groups = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  std::cout << "Before creation, group size is: " << groups.size() << endl;
  EntityRefs edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
  std::cout << "Edges inside the model is:\n";
  for (EntityRefs::iterator it = edges.begin(); it != edges.end(); ++it)
  {
    std::cout << "  " << it->name() << " \n";
  }
  std::cout << std::endl;
  test(edges.size() == 10, "Expecting 10 edges");

  Edges edgelist = resource->findEntitiesByPropertyAs<Edges>("name", "Edge1");
  test(!edgelist.empty() && edgelist.begin()->name() == "Edge1");
  Edge edge1 = edgelist[0];
  Edges edgelist2 = resource->findEntitiesByPropertyAs<Edges>("name", "Edge2");
  Edge edge2 = edgelist2[0];

  // create entity group operator
  std::cout << "Create the entity group operator\n";
  auto egOp = smtk::session::discrete::EntityGroupOperation::create();
  if (!egOp)
  {
    std::cerr << "No entity group operator!\n";
    return 1;
  }

  // Check the optypeItem value
  std::cout << "optypeItem current value is: "
            << egOp->parameters()->findString("Operation")->value() << std::endl;

  egOp->parameters()->associate(modelSimple2dm.component());
  egOp->parameters()->findString("group name")->setValue("test edges");

  auto cellToAdd = egOp->parameters()->findComponent("cell to add");
  cellToAdd->setNumberOfValues(2);
  cellToAdd->setValue(0, edge1.component());
  cellToAdd->setValue(1, edge2.component());

  smtk::operation::Operation::Result egResult = egOp->operate();
  if (egResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Entity group operator failed!\n";
    return 1;
  }
  // print out the elements in the group
  groups = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::GROUP_ENTITY);
  std::cout << "After creation, group size is: " << groups.size() << endl;
  test(groups.size() == 1, "Expecting 1 group");

  std::cout << "The items in groups is:\n";
  for (EntityRefs::iterator it = groups.begin(); it != groups.end(); ++it)
  {
    std::cout << "  group name: " << it->name() << "\n";
    smtk::model::Group group = *it;
    EntityRefs entries = group.members<EntityRefs>();
    std::cout << "  item in the group is:\n";
    for (EntityRefs::iterator it2 = entries.begin(); it2 != entries.end(); ++it2)
    {
      if (it2->isValid())
      {
        std::string name = it2->name();
        std::cout << "    " << name << std::endl;
        //test ((name != "Edge1" && name != "Edge2"),"Edges are not as expeceted!");
      }
    }
    test(entries.size() == 2, "Expecting 2 item in the group");
  }

  return 0;
}
