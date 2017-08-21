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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <chrono>
#include <fstream>

using namespace smtk::model;

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
std::string writeRoot = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
}

int UnitTestMeshSessionReadWrite(int argc, char* argv[])
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
    smtk::model::OperatorPtr importOp = session.op("import");
    if (!importOp)
    {
      std::cerr << "No import operator\n";
      return 1;
    }

    std::string importFilePath(dataRoot);
    importFilePath += "/model/3d/exodus/SimpleReactorCore/SimpleReactorCore.exo";

    importOp->specification()->findFile("filename")->setValue(importFilePath);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    smtk::model::OperatorResult importOpResult = importOp->operate();

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

    model = importOpResult->findModelEntity("model")->value();

    if (importOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "Import operator failed\n";
      return 1;
    }
  }

  {
    smtk::model::OperatorPtr writeOp = session.op("write");
    if (!writeOp)
    {
      std::cerr << "No write operator\n";
      return 1;
    }

    std::string writeFilePath(writeRoot);
    writeFilePath += "/" + smtk::common::UUID::random().toString() + ".exo";

    writeOp->specification()->findFile("filename")->setValue(writeFilePath);
    writeOp->specification()->associateEntity(model);

    smtk::model::OperatorResult writeOpResult = writeOp->operate();
    if (writeOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "Write operator failed\n";
      return 1;
    }

    cleanup(writeFilePath);
  }

  return 0;
}

// This macro ensures the mesh session library is loaded into the executable
smtkComponentInitMacro(smtk_mesh_session)
