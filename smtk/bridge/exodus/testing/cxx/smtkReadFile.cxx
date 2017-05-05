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
#include "smtk/attribute/System.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <string>

// This macro ensures that exodus session is initialized properly on each platform
smtkComponentInitMacro(smtk_exodus_session)

  int main(int argc, char* argv[])
{
  // basic check info
  if (argc < 2)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }
  std::string modelPath(argv[1]);

  // Create model manager and exodus session
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();
  auto session = manager->createSession("exodus");

  // Load model file
  smtk::model::OperatorPtr readOp = session.op("load smtk model");
  test(!!readOp, "No operator import-smtk-model.");

  readOp->specification()->findFile("filename")->setValue(modelPath);
  std::cout << "Importing " << modelPath << "\n";
  smtk::model::OperatorResult opresult = readOp->operate();
  test(opresult->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "Read operator failed");
  smtk::model::EntityRef entRef = opresult->findModelEntity("created")->value();
  smtk::model::Model model(entRef);

  // Check model validity
  test(model.isValid(), "Invalid model");

  // Check model geometry style
  test(model.geometryStyle() == smtk::model::DISCRETE, "Incorrect geometry style (not discrete)");

  test(model.session() == session, "Model should have parent session.");
  test(session.models<std::set<smtk::model::Model> >().find(model)->isValid(),
    "Session should own model.");

  session.close();

  //std::cout << "finis" << std::endl;
  return 0;
}
