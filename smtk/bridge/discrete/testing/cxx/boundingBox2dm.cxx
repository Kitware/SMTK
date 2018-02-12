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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/operators/ImportOperation.h"

#include "smtk/common/UUID.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Vertex.h"

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
  smtk::bridge::discrete::ImportOperation::Ptr importOp =
    smtk::bridge::discrete::ImportOperation::create();
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

  smtk::model::Manager::Ptr resource =
    std::static_pointer_cast<smtk::model::Manager>(model->resource());
  smtk::model::Model modelSimple2dm = model->referenceAs<smtk::model::Model>();

  if (!modelSimple2dm.isValid())
  {
    std::cerr << "Importing 2dm file failed!\n";
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
