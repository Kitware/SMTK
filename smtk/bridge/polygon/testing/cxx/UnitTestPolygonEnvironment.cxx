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
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/bridge/polygon/Resource.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/model/Model.h"

#include "smtk/environment/Environment.h"
#include "smtk/operation/Operation.h"

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  auto readOp =
    smtk::environment::OperationManager::instance()->create("smtk::operation::ReadResource");

  test(readOp != nullptr, "No read operator");

  std::string filename = dataRoot + "/model/2d/smtk/epic-trex-drummer.smtk";
  readOp->parameters()->findFile("filename")->setValue(filename);

  smtk::operation::Operation::Result readOpResult = readOp->operate();
  test(readOpResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Read operator failed");

  smtk::bridge::polygon::Resource::Ptr polygonResource =
    smtk::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(
      readOpResult->findResource("resource")->value());

  smtk::model::Models models =
    polygonResource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  if (models.size() < 1)
    return 1;

  return 0;
}
