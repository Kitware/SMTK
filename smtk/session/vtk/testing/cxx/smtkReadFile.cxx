//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Model.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/operators/LegacyRead.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  // basic check info
  if (argc < 2)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }
  std::string modelPath(argv[1]);

  smtk::session::vtk::Resource::Ptr resource;
  smtk::model::Entity::Ptr modelEntity;

  {
    // Create a read operator
    smtk::session::vtk::LegacyRead::Ptr readOp = smtk::session::vtk::LegacyRead::create();
    if (!readOp)
    {
      std::cerr << "No read operator\n";
      return 1;
    }

    // Set the file path
    std::cout << "readOp parameters: " << readOp->parameters() << std::endl;
    readOp->parameters()->findFile("filename")->setValue(modelPath);

    // Execute the operation
    smtk::operation::Operation::Result readOpResult = readOp->operate();

    // Retrieve the resulting resource
    smtk::attribute::ResourceItemPtr resourceItem =
      std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
        readOpResult->findResource("resourcesCreated"));

    resource = std::dynamic_pointer_cast<smtk::session::vtk::Resource>(resourceItem->value());

    if (!resource)
    {
      std::cerr << "No resource\n";
      return 1;
    }

    // Retrieve the resulting model
    smtk::model::Models models =
      resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

    std::cout << "found " << models.size() << " models" << std::endl;
    if (models.empty())
      return 1;

    modelEntity = models[0].entityRecord();

    // Test for success
    if (
      readOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Read operator failed\n";
      return 1;
    }
  }

  auto model = modelEntity->referenceAs<smtk::model::Model>();

  // Check model validity
  test(model.isValid(), "Invalid model");

  // Check model geometry style
  test(model.geometryStyle() == smtk::model::DISCRETE, "Incorrect geometry style (not discrete)");

  return 0;
}
