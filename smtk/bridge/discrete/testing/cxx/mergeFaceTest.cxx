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

#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/operators/MergeOperation.h"
#include "smtk/bridge/discrete/operators/ReadOperation.h"

#include "smtk/common/UUID.h"

#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << "Usage: mergeFaceTest <path/to/discreteFile.cmb>" << std::endl;
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

  // Create a read operator
  smtk::bridge::discrete::ReadOperation::Ptr readOp =
    smtk::bridge::discrete::ReadOperation::create();
  if (!readOp)
  {
    std::cerr << "No read operator\n";
    return 1;
  }

  // Set the file path
  readOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  // Execute the operation
  smtk::operation::Operation::Result readOpResult = readOp->operate();

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(readOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  // Test for success
  if (readOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Read operator failed\n";
    return 1;
  }

  smtk::model::Manager::Ptr resource =
    std::static_pointer_cast<smtk::model::Manager>(model->resource());
  smtk::model::Model modelCmb = model->referenceAs<smtk::model::Model>();

  int noVolumes = 0;
  int noFaces = 0;
  if (!modelCmb.isValid())
  {
    std::cerr << "Read operator produced an invalid model\n";
    return 1;
  }

  Faces faces;
  for (const auto& cell : modelCmb.cells())
  {
    if (isVolume(cell.entityFlags()))
    {
      Volume vol = static_cast<Volume>(cell);
      noVolumes++;
      faces = vol.faces();
      noFaces += static_cast<int>(faces.size());
    }
  }
  std::cout << "Number of volumes in the model: " << noVolumes << std::endl;
  std::cout << "Number of faces in the model: " << noFaces << std::endl;
  for (decltype(faces.size()) i = 0; i < faces.size(); ++i)
  {
    smtk::common::UUID id(faces[i].entity());
    std::cout << "UUID of Face " << i << ": " << id << std::endl;
  }
  test(noVolumes == 1, "Expecting 1 volume.");
  test(noFaces == 6, "Expecting 6 faces.");
  // Merge faces 3, 4, 5, 6
  smtk::bridge::discrete::MergeOperation::Ptr mergeOp =
    smtk::bridge::discrete::MergeOperation::create();
  test(mergeOp != nullptr, "No merge face operator.");
  auto modelPtr = mergeOp->parameters()->findModelEntity("model");
  test(modelPtr != nullptr && modelPtr->setValue(modelCmb), "Could not associate model");
  modelPtr = mergeOp->parameters()->findModelEntity("source cell");
  test(modelPtr != nullptr && modelPtr->appendValue(faces[2]), "Could not set source cell");
  test(modelPtr != nullptr && modelPtr->appendValue(faces[3]), "Could not set source cell");
  test(modelPtr != nullptr && modelPtr->appendValue(faces[4]), "Could not set source cell");
  modelPtr = mergeOp->parameters()->findModelEntity("target cell");
  test(modelPtr != nullptr && modelPtr->appendValue(faces[5]), "Could not set target cell");
  auto result = mergeOp->operate();
  test(result->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Merge face failed");
  // Check the number of faces after merge face operation
  test(modelCmb.cells().size() == 1 && isVolume(modelCmb.cells()[0].entityFlags()),
    "Expecting 1 volume");
  Volume v = static_cast<Volume>(modelCmb.cells()[0]);
  noFaces = static_cast<int>(v.faces().size());
  std::cout << "Number of faces in the model after merge face operation: " << noFaces << std::endl;
  test(noFaces == 3, "Expecting 3 faces after merge face operation.");

  return 0;
}
