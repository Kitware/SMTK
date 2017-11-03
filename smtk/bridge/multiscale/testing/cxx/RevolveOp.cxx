//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PythonAutoInit.h"

#include "smtk/bridge/multiscale/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

namespace
{

std::string afrlRoot = std::string(AFRL_DIR);
std::string dataRoot = SMTK_DATA_DIR;
}

int RevolveOp(int argc, char* argv[])
{
  if (afrlRoot.empty())
  {
    std::cerr << "AFRL directory not defined\n";
    return 1;
  }

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  smtk::model::StringList sessions = manager->sessionTypeNames();
  for (smtk::model::StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::multiscale::Session::Ptr session = smtk::bridge::multiscale::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  smtk::model::StringList opnames = session->operatorNames();
  for (smtk::model::StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::OperatorPtr importOp = session->op("import");
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(dataRoot);
  importFilePath += "/mesh/2d/ImportFromDEFORM.h5m";

  importOp->specification()->findFile("filename")->setValue(importFilePath);

  smtk::model::OperatorResult importOpResult = importOp->operate();

  if (importOpResult->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  smtk::model::Model model = importOpResult->findModelEntity("model")->value();

  if (!model.isValid())
  {
    std::cerr << "Import operator returned an invalid model\n";
    return 1;
  }

  smtk::model::OperatorPtr revolveOp = session->op("revolve");
  if (!revolveOp)
  {
    std::cerr << "No revolve operator\n";
    return 1;
  }

  revolveOp->specification()->associateEntity(model);
  revolveOp->specification()->findDouble("sweep-angle")->setValue(30.);
  revolveOp->specification()->findInt("resolution")->setValue(15);
  revolveOp->specification()->findDouble("axis-direction")->setValue(0, 0.);
  revolveOp->specification()->findDouble("axis-direction")->setValue(1, 1.);
  revolveOp->specification()->findDouble("axis-direction")->setValue(2, 0.);
  revolveOp->specification()->findDouble("axis-position")->setValue(0, -0.02);
  revolveOp->specification()->findDouble("axis-position")->setValue(1, 0.);
  revolveOp->specification()->findDouble("axis-position")->setValue(2, 0.);

  smtk::model::OperatorResult revolveOpResult = revolveOp->operate();
  if (revolveOpResult->findInt("outcome")->value() !=
    smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    std::cerr << "Revolve operator failed\n";
    return 1;
  }

  return 0;
}
