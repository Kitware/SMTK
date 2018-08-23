//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/bridge/polygon/Registrar.h"

#include "smtk/common/Registry.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <stdlib.h>
#include <string.h>

using smtk::shared_ptr;
using namespace smtk::common;
using namespace smtk::view;
using namespace smtk::io;

static int maxIndent = 6;
static std::vector<char*> dataArgs;

namespace
{

int printer(DescriptivePhrasePtr p, const std::vector<int>& idx)
{
  int indent = static_cast<int>(idx.size()) * 2;
  if (p)
  {
    std::cout << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")";
    smtk::resource::FloatList rgba = p->relatedColor();
    if (rgba[3] >= 0.)
    {
      std::cout << " rgba(" << rgba[0] << "," << rgba[1] << "," << rgba[2] << "," << rgba[3] << ")";
    }
    auto sub = p->subphrases(); // force subphrases to get built, though we may not visit them
    (void)sub;
    std::cout << "\n";
    std::ostringstream idxStr;
    for (int ii : idx)
    {
      idxStr << " " << ii;
    }
    smtkTest(
      p->at(idx) == p, "Index " << idxStr.str() << " passed to visitor did not produce phrase!");
  }
  return indent > maxIndent ? 1 : 0;
}

} // namespace

int unitComponentPhraseModel(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::string testFile;
    testFile = SMTK_DATA_DIR;
    testFile += "/model/2d/smtk/epic-trex-drummer.smtk";
    dataArgs.push_back(argv[0]);
    dataArgs.push_back(strdup(testFile.c_str()));
    dataArgs.push_back(nullptr);
    argc = 2;
    argv = &dataArgs[0];
  }
  auto rsrcMgr = smtk::resource::Manager::create();
  auto operMgr = smtk::operation::Manager::create();
  operMgr->registerResourceManager(rsrcMgr);

  auto registry = smtk::common::Registry<smtk::bridge::polygon::Registrar, smtk::resource::Manager,
    smtk::operation::Manager>(rsrcMgr, operMgr);

  auto phraseModel = smtk::view::ComponentPhraseModel::create();
  std::multimap<std::string, std::string> filters;
  filters.insert(
    std::make_pair(std::string("smtk::bridge::polygon::Resource"), std::string("edge")));
  filters.insert(
    std::make_pair(std::string("smtk::bridge::polygon::Resource"), std::string("face")));
  phraseModel->setComponentFilters(filters);
  phraseModel->addSource(rsrcMgr, operMgr);
  smtk::resource::ResourceArray rsrcs;
  for (int i = 1; i < argc; i++)
  {
    auto rdr = operMgr->create<smtk::operation::ReadResource>();
    rdr->parameters()->findFile("filename")->setValue(argv[i]);
    rdr->operate();
    // rsrcs.push_back(rsrcMgr->read<smtk::bridge::polygon::Resource>(argv[1]));
  }
  smtk::resource::ResourcePtr rsrc = nullptr;
  std::for_each(rsrcMgr->resources().begin(), rsrcMgr->resources().end(),
    [&rsrc](const smtk::resource::ResourcePtr& rr) {
      if (rr && !rsrc)
      {
        rsrc = rr;
      }
    });
  smtkTest(!!rsrc, "Unable to discern that a resource was loaded.");

  test(phraseModel->root()->root() == phraseModel->root(),
    "Model's root phrase was not root of tree.");
  phraseModel->root()->visitChildren(printer);

  filters.erase(filters.begin());
  std::cout << "---\n";

  phraseModel->setComponentFilters(filters);
  phraseModel->root()->visitChildren(printer);

  if (!dataArgs.empty())
  {
    phraseModel->setOnlyShowActiveResourceComponents(true);
    std::cout << "---\n";
    smtkTest(phraseModel->root()->subphrases().empty(),
      "Expected an empty list with a null active resource");
    phraseModel->setActiveResource(rsrc);
    std::cout << "---\n";
    phraseModel->root()->visitChildren(printer);
    smtkTest(!phraseModel->root()->subphrases().empty(),
      "Expected a non-empty list with a non-null active resource");

    // Don't leak
    free(dataArgs[1]);
  }

  return 0;
}

smtkComponentInitMacro(smtk_polygon_session);
