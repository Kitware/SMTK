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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/bridge/openfoam/Session.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

namespace
{
std::string scratch_dir = SMTK_SCRATCH_DIR;
}

int SetWorkingDirectoryOp(int argc, char* argv[])
{
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  smtk::model::StringList sessions = manager->sessionTypeNames();
  for (smtk::model::StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::openfoam::Session::Ptr session = smtk::bridge::openfoam::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  smtk::model::StringList opnames = session->operatorNames();
  for (smtk::model::StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  {
    smtk::model::OperatorPtr setWorkingDirectoryOp = session->op("set working directory");
    if (!setWorkingDirectoryOp)
    {
      std::cerr << "No set working directory operator\n";
      return 1;
    }

    setWorkingDirectoryOp->specification()
      ->findDirectory("working directory")
      ->setValue(scratch_dir += "/" + smtk::common::UUID::random().toString());

    smtk::model::OperatorResult setWorkingDirectoryOpResult = setWorkingDirectoryOp->operate();
    if (setWorkingDirectoryOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "set working directory operator failed\n";
      return 1;
    }

    session->removeWorkingDirectory();
  }

  return 0;
}

smtkPythonInitMacro(set_working_directory, smtk.bridge.openfoam.set_working_directory, true);
