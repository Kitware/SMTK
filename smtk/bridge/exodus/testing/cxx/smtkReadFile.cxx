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
  smtk::model::OperatorPtr readOp = session.op("import smtk model");
  if (!readOp)
  {
    std::cerr << "No operator import-smtk-model\n";
    return 1;
  }
  readOp->specification()->findFile("filename")->setValue(modelPath);
  std::cout << "Importing " << modelPath << "\n";
  smtk::model::OperatorResult opresult = readOp->operate();
  if (
    opresult->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Read operator failed\n";
    return 1;
  }
  smtk::model::EntityRef entRef = opresult->findModelEntity("created")->value();
  smtk::model::Model model(entRef);

  // Check model validity
  if (!model.isValid())
  {
    std::cerr << "Invalid model!\n";
    return 1;
  }

  // Check model geometry style
  if (model.geometryStyle() != smtk::model::DISCRETE)
    {
    std::cerr << "Incorrect geometry style (not discrete)";
    return 1;
    }

  //std::cout << "finis" << std::endl;
  return 0;
}
