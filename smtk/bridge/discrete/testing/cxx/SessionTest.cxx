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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/operators/ReadOperator.h"
#include "smtk/bridge/discrete/operators/SplitFaceOperator.h"

#include "smtk/io/SaveJSON.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Tessellation.h"

#include "smtk/operation/Manager.h"

#include <fstream>

static int maxIndent = 10;

using namespace smtk::model;

int main(int argc, char* argv[])
{
  if (argc < 2)
    return 1;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the resource to the resource manager
  {
    resourceManager->registerResource<smtk::bridge::discrete::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register the operators to the operation manager
  {
    operationManager->registerOperator<smtk::bridge::discrete::ReadOperator>(
      "smtk::bridge::discrete::ReadOperator");
    operationManager->registerOperator<smtk::bridge::discrete::SplitFaceOperator>(
      "smtk::bridge::discrete::SplitFaceOperator");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  {
    // Create a read operator
    smtk::bridge::discrete::ReadOperator::Ptr readOp =
      operationManager->create<smtk::bridge::discrete::ReadOperator>();
    if (!readOp)
    {
      std::cerr << "No read operator\n";
      return 1;
    }

    // Set the file path
    readOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

    // Execute the operation
    smtk::operation::NewOp::Result readOpResult = readOp->operate();

    // Retrieve the resulting model
    smtk::attribute::ComponentItemPtr componentItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        readOpResult->findComponent("model"));

    // Access the generated model
    model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

    // Test for success
    if (readOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
    {
      std::cerr << "Read operator failed\n";
      return 1;
    }
  }

  smtk::bridge::discrete::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::bridge::discrete::Resource>(model->resource());
  resource->assignDefaultNames(); // should force transcription of every entity, but doesn't yet.

  // Test a model operator (if some argument beyond filename is given)
  if (argc > 2)
  {
    // Find a face with more than 2 triangles
    smtk::model::Faces allFaces;
    smtk::model::EntityRef::EntityRefsFromUUIDs(
      allFaces, resource, resource->entitiesMatchingFlags(smtk::model::FACE));
    smtk::model::Face f;
    for (smtk::model::Faces::iterator it = allFaces.begin(); it != allFaces.end(); ++it)
    {
      f = *it;
      const smtk::model::Tessellation* tess = f.hasTessellation();
      if (tess && tess->conn().size() > 8)
        break;
    }
    if (f.isValid() && f.hasTessellation()->conn().size() > 8)
    {
      std::cout << "Attempting face split\n";

      // Create a split face operator
      smtk::bridge::discrete::SplitFaceOperator::Ptr splitFaceOp =
        operationManager->create<smtk::bridge::discrete::SplitFaceOperator>();
      if (!splitFaceOp)
      {
        std::cerr << "No split face operator\n";
        return 1;
      }

      auto faceToSplit = splitFaceOp->parameters()->findModelEntity("face to split");
      faceToSplit->setNumberOfValues(1);
      faceToSplit->setValue(f);
      splitFaceOp->parameters()->findModelEntity("model")->setValue(
        *resource->entitiesMatchingFlagsAs<Models>(smtk::model::MODEL_ENTITY).begin());
      splitFaceOp->parameters()->findDouble("feature angle")->setValue(15.0);
      smtk::bridge::discrete::SplitFaceOperator::Result result = splitFaceOp->operate();
      std::cout << "  Face is " << f.name() << " (" << f.entity() << ")\n";
      std::cout << "  " << (result->findInt("outcome")->value() ==
                                 smtk::operation::Operator::OPERATION_SUCCEEDED
                               ? "OK"
                               : "Failed")
                << "\n";
    }
    else if (f.isValid())
    {
      std::cout << "No faces to split\n";
    }

    smtk::model::EntityRefArray exports;
    exports.push_back(model);
    resource->discreteSession()->ExportEntitiesToFileOfNameAndType(
      exports, "sessionTest.cmb", "cmb");
    std::cout << "  done\n";
  }

  std::string json = smtk::io::SaveJSON::fromModelManager(resource);
  if (!json.empty())
  {
    std::ofstream jsonFile("sessionTest.json");
    jsonFile << json;
    jsonFile.close();
  }

  return 0;
}
