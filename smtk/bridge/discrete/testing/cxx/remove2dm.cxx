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

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/operators/ImportOperation.h"
#include "smtk/bridge/discrete/operators/RemoveModel.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Manager.h"

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }

  std::ifstream file;
  file.open(argv[1]);
  if (!file.good())
  {
    std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
    return 1;
  }
  file.close();

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

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model->resource());
  smtk::model::Model model2dm = model->referenceAs<smtk::model::Model>();

  if (!model2dm.isValid())
  {
    std::cerr << "Reading 2dm file failed!\n";
    return 1;
  }

  // Remove model
  smtk::bridge::discrete::RemoveModel::Ptr removeOp = smtk::bridge::discrete::RemoveModel::create();
  if (!removeOp)
  {
    std::cerr << "No remove operator\n";
    return 1;
  }

  test(removeOp != nullptr, "No remove model operator.");
  test(removeOp->parameters()->associateEntity(model2dm), "Could not associate model.");
  auto result = removeOp->operate();
  test(result->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "close model failed.");
  // Make sure the model is removed
  test(smtk::model::SessionRef(resource, resource->discreteSession()->sessionId())
         .models<Models>()
         .empty(),
    "Expecting no models after close.");

  return 0;
}
