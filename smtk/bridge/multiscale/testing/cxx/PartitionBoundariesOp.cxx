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

#include "smtk/mesh/Manager.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <cassert>

using namespace smtk::model;

namespace
{

std::string afrlRoot = std::string(AFRL_DIR);
std::string dataRoot = SMTK_DATA_DIR;

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

int PartitionBoundariesOp(int argc, char* argv[])
{
  if (afrlRoot.empty())
  {
    std::cerr << "AFRL directory not defined\n";
    return 1;
  }

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::multiscale::Session::Ptr session = smtk::bridge::multiscale::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
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

  if (importOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
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
  if (revolveOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Revolve operator failed\n";
    return 1;
  }

  model = revolveOpResult->findModelEntity("model")->value();

  smtk::model::OperatorPtr partitionBoundariesOp = session->op("partition boundaries");
  if (!partitionBoundariesOp)
  {
    std::cerr << "No partition boundaries operator\n";
    return 1;
  }

  partitionBoundariesOp->specification()->associateEntity(model);
  partitionBoundariesOp->specification()->findDouble("origin")->setValue(0, -0.02);
  partitionBoundariesOp->specification()->findDouble("origin")->setValue(1, 0.);
  partitionBoundariesOp->specification()->findDouble("origin")->setValue(2, 0.);
  partitionBoundariesOp->specification()->findDouble("radius")->setValue(1.2);

  smtk::model::OperatorResult partitionBoundariesOpResult = partitionBoundariesOp->operate();

  if (partitionBoundariesOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "partition boundaries operator failed\n";
    return 1;
  }

  smtk::attribute::ModelEntityItemPtr created =
    partitionBoundariesOpResult->findModelEntity("created");

  assert(created->numberOfValues() == 7);

  for (smtk::attribute::ModelEntityItem::const_iterator it = created->begin(); it != created->end();
       ++it)
  {
    assert(it->isValid());
    assert(it->isVertex());
  }

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_MeshIOVTK)
  smtkPythonInitMacro(import_from_deform, smtk.bridge.multiscale.import_from_deform, true);
