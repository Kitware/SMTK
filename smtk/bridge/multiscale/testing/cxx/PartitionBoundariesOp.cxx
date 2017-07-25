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

  smtk::model::OperatorPtr importFromDeformOp = session->op("import from deform");
  if (!importFromDeformOp)
  {
    std::cerr << "No import from deform operator\n";
    return 1;
  }

  importFromDeformOp->specification()
    ->findFile("point-file")
    ->setValue(afrlRoot + "/Data/afrl_test_forging/ti6242_ptrak_fg.csv");
  importFromDeformOp->specification()->findInt("timestep")->setValue(66);
  importFromDeformOp->specification()
    ->findFile("element-file")
    ->setValue(afrlRoot + "/Data/afrl_test_forging/ti6242_node_elem_fg.dat");
  importFromDeformOp->specification()
    ->findFile("pipeline-executable")
    ->setValue(afrlRoot + "/../dream3d/install/DREAM3D.app/Contents/bin/PipelineRunner");
  importFromDeformOp->specification()->findString("attribute")->setToDefault();
  importFromDeformOp->specification()
    ->findFile("output-file")
    ->setValue(afrlRoot + "/tmp/out.dream3d");

  std::vector<double> mu = { { 2.29, 2.29, 2.29, 2.29, 3. } };
  std::vector<double> sigma = { { .1, .1, .1, .1, .2 } };
  std::vector<double> min_cutoff = { { 5, 5, 5, 5, 4 } };
  std::vector<double> max_cutoff = { { 5, 5, 5, 5, 6 } };

  smtk::attribute::GroupItem::Ptr stats = importFromDeformOp->specification()->findGroup("stats");
  stats->setNumberOfGroups(mu.size());

  for (std::size_t i = 0; i < mu.size(); i++)
  {
    stats->findAs<smtk::attribute::DoubleItem>(i, "mu")->setValue(mu.at(i));
    stats->findAs<smtk::attribute::DoubleItem>(i, "sigma")->setValue(sigma.at(i));
    stats->findAs<smtk::attribute::DoubleItem>(i, "min_cutoff")->setValue(min_cutoff.at(i));
    stats->findAs<smtk::attribute::DoubleItem>(i, "max_cutoff")->setValue(max_cutoff.at(i));
  }

  smtk::model::OperatorResult importFromDeformOpResult = importFromDeformOp->operate();
  // cleanup("out.dream3d");
  // cleanup("out.xdmf");
  if (importFromDeformOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "import from deform operator failed\n";
    return 1;
  }

  smtk::model::Model model = importFromDeformOpResult->findModelEntity("model")->value();

  if (false)
  {
    smtk::model::OperatorPtr writeOp = session->op("export mesh");
    if (!writeOp)
    {
      std::cerr << "No write operator\n";
      return 1;
    }

    writeOp->specification()
      ->findFile("filename")
      ->setValue("/Users/tjcorona/Desktop/Dream3DOutputModel.vtk");
    // writeOp->specification()->associateEntity(model);
    bool valueSet = writeOp->specification()->findMesh("mesh")->setValue(
      manager->meshes()->findCollection(model.entity())->second->meshes());

    smtk::model::OperatorResult writeOpResult = writeOp->operate();
    if (writeOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "Write operator failed\n";
      return 1;
    }
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

  assert(created->numberOfValues() == 3);

  for (smtk::attribute::ModelEntityItem::const_iterator it = created->begin(); it != created->end();
       ++it)
  {
    assert(it->isVertex());
  }

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_MeshIOVTK)
  smtkPythonInitMacro(import_from_deform, smtk.bridge.multiscale.import_from_deform);
