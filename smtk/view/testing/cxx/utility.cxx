//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/testing/cxx/utility.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Registrar.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/session/polygon/Registrar.h"

#include "smtk/plugin/Registry.h"

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/model/SessionRef.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <iostream>

namespace smtk
{
namespace view
{

PhraseModel::Ptr loadTestData(
  int argc,
  char* argv[],
  const ManagerPtr& viewManager,
  const Configuration& viewConfig,
  std::vector<char*>& dataArgs,
  smtk::resource::ManagerPtr& rsrcMgr,
  smtk::operation::ManagerPtr& operMgr)
{
  if (argc < 2)
  {
    std::string testFile;
#ifdef SMTK_DATA_DIR
    testFile = SMTK_DATA_DIR;
#else
    std::cerr << "ERROR: Test data not available. Cannot load test data.\n";
#endif
    testFile += "/model/2d/smtk/epic-trex-drummer.smtk";
    dataArgs.push_back(argv[0]);
    dataArgs.push_back(strdup(testFile.c_str()));
    dataArgs.push_back(nullptr);
    argc = 2;
    argv = dataArgs.data();
  }
  rsrcMgr = smtk::resource::Manager::create();
  operMgr = smtk::operation::Manager::create();
  operMgr->registerResourceManager(rsrcMgr);

  auto polygonRegistry =
    smtk::plugin::addToManagers<smtk::session::polygon::Registrar>(rsrcMgr, operMgr);
  auto viewRegistry = smtk::plugin::addToManagers<smtk::view::Registrar>(viewManager);
  auto phraseModel = viewManager->phraseModelFactory().createFromConfiguration(&viewConfig);
  // auto phraseModel = smtk::view::ResourcePhraseModel::create();
  phraseModel->addSource({ rsrcMgr, operMgr });
  smtk::resource::ResourceArray rsrcs;
  for (int i = 1; i < argc; i++)
  {
    auto rdr = operMgr->create<smtk::operation::ReadResource>();
    rdr->parameters()->findFile("filename")->setValue(argv[i]);
    rdr->operate();
    // rsrcs.push_back(rsrcMgr->read<smtk::session::polygon::Resource>(argv[1]));
  }

  return phraseModel;
}
} // namespace view
} // namespace smtk
