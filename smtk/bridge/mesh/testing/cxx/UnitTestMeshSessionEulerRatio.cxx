//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/Session.h"

#include "smtk/common/UUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include <cmath>

using namespace smtk::model;

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
}

int UnitTestMeshSessionEulerRatio(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::SessionRef session = manager->createSession("mesh");

  std::cout << "Available cmb operators\n";
  StringList opnames = session.operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::Model model;

  {
    smtk::model::OperatorPtr readOp = session.op("read");
    if (!readOp)
    {
      std::cerr << "No read operator\n";
      return 1;
    }

    std::string readFilePath(dataRoot);
    readFilePath += "/mesh/3d/cube.exo";

    readOp->specification()->findFile("filename")->setValue(readFilePath);

    smtk::model::OperatorResult readOpResult = readOp->operate();

    model = readOpResult->findModelEntity("model")->value();

    if (readOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "Read operator failed\n";
      return 1;
    }
  }

  {
    smtk::model::OperatorPtr eulerOp = session.op("euler characteristic ratio");
    if (!eulerOp)
    {
      std::cerr << "No \"euler characteristic ratio\" operator\n";
      return 1;
    }

    eulerOp->specification()->associateEntity(model);

    smtk::model::OperatorResult eulerOpResult = eulerOp->operate();
    if (eulerOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "\"Euler characteristic ratio\" operator failed\n";
      return 1;
    }

    if (std::abs(eulerOpResult->findDouble("value")->value() - 2.) > 1.e-10)
    {
      std::cerr << "Unexpected Euler ratio\n";
      return 1;
    }
  }

  return 0;
}

// This macro ensures the mesh session library is loaded into the executable
smtkComponentInitMacro(smtk_mesh_session)
