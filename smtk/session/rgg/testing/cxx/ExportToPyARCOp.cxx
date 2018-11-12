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

#include "smtk/session/rgg/Registrar.h"
#include "smtk/session/rgg/Session.h"

#include "smtk/session/rgg/operators/CreateModel.h"
#include "smtk/session/rgg/operators/ReadRXFFile.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"

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
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register rgg resources to the resource manager
  {
    smtk::session::rgg::Registrar::registerTo(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register rgg operators to the operation manager
  {
    smtk::session::rgg::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Model model;
  {
    auto createModelOp = smtk::session::rgg::CreateModel::create();
    if (!createModelOp)
    {
      std::cerr << "No create model operator\n";
      return 1;
    }

    auto createModelOpResult = createModelOp->operate();
    if (createModelOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "create model operator failed\n";
      return 1;
    }

    smtk::model::Model model =
      createModelOpResult->findComponent("created")->valueAs<smtk::model::Entity>();
    if (!model.isValid())
    {
      std::cerr << "create model operator constructed an invalid model\n";
      return 1;
    }
  }

  {
    auto readRXFOp = smtk::session::rgg::ReadRXFFile::create();
    if (!readRXFOp)
    {
      std::cerr << "No \"Read RXF File\" operator\n";
      return 1;
    }

    readRXFOp->parameters()->associate(model.component());

    readRXFOp->parameters()
      ->findFile("filename")
      ->setValue(dataRoot + "/model/3d/rgg/sampleCore.rxf");

    auto readRXFOpResult = readRXFOp->operate();
    if (readRXFOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "\"Read RXF File\" operator failed\n";
      return 1;
    }
  }

  {
    auto exportToPyARCOp =
      operationManager->create("smtk.session.rgg.export_to_pyarc.export_to_pyarc");

    if (!exportToPyARCOp)
    {
      std::cerr << "No export to pyarc operator\n";
      return 1;
    }

    std::string writeFilePath(writeRoot);
    writeFilePath += "/" + smtk::common::UUID::random().toString() + ".son";

    exportToPyARCOp->parameters()->findFile("filename")->setValue(writeFilePath);

    exportToPyARCOp->parameters()->associate(model.component());

    auto exportToPyARCOpResult = exportToPyARCOp->operate();
    if (exportToPyARCOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "export to pyarc operator failed\n";
      return 1;
    }

    cleanup(writeFilePath);
  }

  return 0;
}
