//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/multiscale/RegisterSession.h"
#include "smtk/bridge/multiscale/Session.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

namespace
{
std::string afrlRoot = std::string(AFRL_DIR);
}

int ImportFromDEFORMOp(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  if (afrlRoot.empty())
  {
    std::cerr << "AFRL directory not defined\n";
    return 1;
  }

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register multiscale resources to the resource manager
  {
    smtk::bridge::multiscale::registerResources(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register multiscale operators to the operation manager
  {
    smtk::bridge::multiscale::registerOperations(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an import operator
  smtk::operation::NewOp::Ptr importFromDeformOp =
    operationManager->create("smtk.bridge.multiscale.import_from_deform.import_from_deform");

  if (!importFromDeformOp)
  {
    std::cerr << "Couldn't create \"import from deform\" operator" << std::endl;
    return 1;
  }

  importFromDeformOp->parameters()
    ->findFile("point-file")
    ->setValue(afrlRoot + "/Data/afrl_test_forging/ti6242_ptrak_fg.csv");
  importFromDeformOp->parameters()->findInt("timestep")->setValue(66);
  importFromDeformOp->parameters()
    ->findFile("element-file")
    ->setValue(afrlRoot + "/Data/afrl_test_forging/ti6242_node_elem_fg.dat");
  importFromDeformOp->parameters()
    ->findFile("pipeline-executable")
    ->setValue(afrlRoot + "/../dream3d/build/Bin/PipelineRunner");
  importFromDeformOp->parameters()->findString("attribute")->setToDefault();
  importFromDeformOp->parameters()
    ->findFile("output-file")
    ->setValue(afrlRoot + "/tmp/out.dream3d");

  std::vector<double> mu = { { 2.29, 2.29, 2.29, 2.29, 3. } };
  std::vector<double> sigma = { { .1, .1, .1, .1, .2 } };
  std::vector<double> min_cutoff = { { 5, 5, 5, 5, 4 } };
  std::vector<double> max_cutoff = { { 5, 5, 5, 5, 6 } };

  smtk::attribute::GroupItem::Ptr stats = importFromDeformOp->parameters()->findGroup("stats");
  stats->setNumberOfGroups(mu.size());

  for (std::size_t i = 0; i < mu.size(); i++)
  {
    stats->findAs<smtk::attribute::DoubleItem>(i, "mu")->setValue(mu.at(i));
    stats->findAs<smtk::attribute::DoubleItem>(i, "sigma")->setValue(sigma.at(i));
    stats->findAs<smtk::attribute::DoubleItem>(i, "min_cutoff")->setValue(min_cutoff.at(i));
    stats->findAs<smtk::attribute::DoubleItem>(i, "max_cutoff")->setValue(max_cutoff.at(i));
  }

  smtk::operation::NewOp::Result importFromDeformOpResult = importFromDeformOp->operate();
  if (importFromDeformOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "import from deform operator failed\n";
    return 1;
  }

  smtk::model::Model model = importFromDeformOpResult->findModelEntity("created")->value();
  if (!model.isValid())
  {
    std::cerr << "import from deform operator constructed an invalid model\n";
    return 1;
  }

  return 0;
}
