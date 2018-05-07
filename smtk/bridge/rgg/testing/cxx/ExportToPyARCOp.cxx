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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

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

int ExportToPyARCOp(int argc, char* argv[])
{
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  smtk::model::StringList sessions = manager->sessionTypeNames();
  for (smtk::model::StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::rgg::Session::Ptr session = smtk::bridge::rgg::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  smtk::model::StringList opnames = session->operatorNames();
  for (smtk::model::StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::Model model;
  {
    smtk::model::OperatorPtr createModelOp = session->op("create model");
    if (!createModelOp)
    {
      std::cerr << "No create model operator\n";
      return 1;
    }

    smtk::model::OperatorResult createModelOpResult = createModelOp->operate();
    if (createModelOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "create model operator failed\n";
      return 1;
    }

    model = createModelOpResult->findModelEntity("created")->value();
    if (!model.isValid())
    {
      std::cerr << "create model operator constructed an invalid model\n";
      return 1;
    }
  }

  {
    smtk::model::OperatorPtr readRXFOp = session->op("read rxf file");
    if (!readRXFOp)
    {
      std::cerr << "No create model operator\n";
      return 1;
    }

    readRXFOp->specification()->associateEntity(model);

    readRXFOp->specification()
      ->findFile("filename")
      ->setValue(dataRoot + "/model/3d/rgg/sampleCore.rxf");

    smtk::model::OperatorResult readRXFOpResult = readRXFOp->operate();
    if (readRXFOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "create model operator failed\n";
      return 1;
    }
  }

  {
    smtk::model::OperatorPtr exportToPyARCOp = session->op("export to pyarc");
    if (!exportToPyARCOp)
    {
      std::cerr << "No export to pyarc operator\n";
      return 1;
    }

    std::string writeFilePath(writeRoot);
    writeFilePath += "/" + smtk::common::UUID::random().toString() + ".son";

    exportToPyARCOp->specification()->findFile("filename")->setValue(writeFilePath);

    exportToPyARCOp->specification()->associateEntity(model);

    smtk::model::OperatorResult exportToPyARCOpResult = exportToPyARCOp->operate();
    if (exportToPyARCOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "export to pyarc operator failed\n";
      return 1;
    }

    cleanup(writeFilePath);
  }

  return 0;
}

smtkPythonInitMacro(export_to_pyarc, smtk.bridge.rgg.export_to_pyarc, true);
