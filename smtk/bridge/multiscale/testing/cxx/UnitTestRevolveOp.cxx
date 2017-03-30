//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <fstream>

static int maxIndent = 10;

using namespace smtk::model;

namespace
{

std::string afrlRoot = std::string(AFRL_DIR);

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

int UnitTestRevolveOp(int argc, char* argv[])
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

  smtk::model::OperatorPtr dream3dOp = session->op("dream3d");
  if (!dream3dOp)
  {
    std::cerr << "No dream3d operator\n";
    return 1;
  }

  dream3dOp->specification()
    ->findFile("point-file")
    ->setValue(afrlRoot + "/Dream3DPipelines/Inputs/DEF_PTR.RST");
  dream3dOp->specification()
    ->findFile("step-file")
    ->setValue(afrlRoot + "/Dream3DPipelines/Inputs/F2_DataExtract_Step623.DAT");
  dream3dOp->specification()
    ->findFile("pipeline-executable")
    ->setValue(afrlRoot + "/Placeholders/bin/PipelineRunner");
  dream3dOp->specification()->findFile("output-file")->setValue("out.dream3d");
  dream3dOp->specification()->findString("attribute")->setToDefault();

  smtk::attribute::FileItem::Ptr statsfiles = dream3dOp->specification()->findFile("stats-files");
  if (!statsfiles)
  {
    std::cerr << "No stats files!\n";
    return 1;
  }

  bool ok = statsfiles->setNumberOfValues(4);

  if (!ok)
  {
    std::cerr << "FileItem failed\n";
    return 1;
  }

  statsfiles->setValue(0, afrlRoot + "/Dream3DPipelines/Inputs/randomEquiaxed_mu1.dream3d");
  statsfiles->setValue(1, afrlRoot + "/Dream3DPipelines/Inputs/randomEquiaxed_mu15.dream3d");
  statsfiles->setValue(2, afrlRoot + "/Dream3DPipelines/Inputs/randomEquiaxed_mu2.dream3d");
  statsfiles->setValue(3, afrlRoot + "/Dream3DPipelines/Inputs/randomEquiaxed_mu25.dream3d");

  smtk::model::OperatorResult dream3dOpResult = dream3dOp->operate();
  cleanup("out.dream3d");
  if (dream3dOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Dream3d operator failed\n";
    return 1;
  }

  smtk::model::Model model = dream3dOpResult->findModelEntity("model")->value();

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

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_MeshIOVTK)
