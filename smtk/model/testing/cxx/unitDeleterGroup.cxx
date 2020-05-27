//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/DeleterGroup.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

int unitDeleterGroup(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  smtk::model::Registrar::registerTo(resourceManager);
  smtk::model::Registrar::registerTo(operationManager);

  smtk::operation::DeleterGroup deleters(operationManager);

  auto resource = resourceManager->create<smtk::model::Resource>();
  auto model = resource->addModel(3, 3, "test model");
  auto face = resource->addFace();
  model.addCell(face);
  smtkTest(!model.cells().empty(), "Face creation did not succeed.");

  auto op = deleters.matchingOperation(*face.component());
  auto deleter = operationManager->create(op);
  smtkTest(!!deleter, "Could not find or create the model deletion operation.");
  smtkTest(deleter->parameters()->associate(face.component()), "Could not associate face.");
  auto result = deleter->operate();
  auto outcome =
    static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
  smtkTest(outcome == smtk::operation::Operation::Outcome::SUCCEEDED, "Deletion failed.");
  smtkTest(model.cells().empty(), "Deletion did not actually succeed.");

  return 0;
}
