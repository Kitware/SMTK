//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/bridge/polygon/Registrar.h"

#include "smtk/common/Registry.h"

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <fstream>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <string.h>

using smtk::shared_ptr;
using namespace smtk::common;
using namespace smtk::view;
using namespace smtk::io;

static int maxIndent = 6;
static std::vector<char*> dataArgs;

void testUpdateChildren(smtk::view::ResourcePhraseModel::Ptr phraseModel)
{
  auto root = phraseModel->root();
  auto phrResources = root->subphrases();
  test(!phrResources.empty(), "Expected a phrase for the resource.");
  std::cout << "rsrc " << phrResources[0]->title() << "\n";
  auto phrModels = phrResources[0]->subphrases();
  test(!phrModels.empty(), "Expected a phrase for the resource's model.");
  std::cout << "modl " << phrModels[0]->title() << "\n";
  auto phrModelSummary = phrModels[0]->subphrases();
  test(phrModelSummary.size() > 3, "Expected phrases describing the resource's model.");
  std::cout << "summ " << phrModelSummary[3]->title() << "\n";
  DescriptivePhrases phrFaces = phrModelSummary[3]->subphrases();
  std::cout << "Removing 3 entries from " << phrModelSummary[3]->title() << "\n";
  phrFaces.erase(phrFaces.begin() + 2, phrFaces.begin() + 5);
  std::vector<int> idx;
  idx.push_back(0);
  idx.push_back(0);
  idx.push_back(1);
  int numObservations = 0;
  phraseModel->observe(
    [&numObservations](DescriptivePhrasePtr pp, PhraseModelEvent pe, const std::vector<int>& src,
      const std::vector<int>& dst, const std::vector<int>& delta) {
      (void)src;
      (void)dst;
      std::cout << "Phrase event " << static_cast<int>(pe) << " " << pp->title() << " " << delta[0]
                << " " << delta[1] << "\n";
      smtkTest(delta.size() == 2, "Expecting phrase update to specify range of removed entries.");
      smtkTest(delta[0] == 2 && delta[1] == 4, "Expecting delta = [2, 4].");
      ++numObservations;
    },
    false // Do not immediately notify of existing items.
    );
  phraseModel->updateChildren(phrModelSummary[3], phrFaces, idx);
  smtkTest(numObservations == 2, "Expected to observe removal of rows in 2 steps.");
  std::cout << "There are " << phrModelSummary[3]->subphrases().size() << " entries remaining.\n";
}

int unitDescriptivePhrase(int argc, char* argv[])
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
  auto phraseModel = smtk::view::ResourcePhraseModel::create();
  phraseModel->addSource(rsrcMgr, operMgr);
  smtk::resource::ResourceArray rsrcs;
  for (int i = 1; i < argc; i++)
  {
    auto rdr = operMgr->create<smtk::operation::ReadResource>();
    rdr->parameters()->findFile("filename")->setValue(argv[i]);
    rdr->operate();
    // rsrcs.push_back(rsrcMgr->read<smtk::bridge::polygon::Resource>(argv[1]));
  }

  test(phraseModel->root()->root() == phraseModel->root(),
    "Model's root phrase was not root of tree.");
  phraseModel->root()->visitChildren(
    [](DescriptivePhrasePtr p, const std::vector<int>& idx) -> int {
      int indent = static_cast<int>(idx.size()) * 2;
      if (p)
      {
        std::cout << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")";
        smtk::resource::FloatList rgba = p->relatedColor();
        if (rgba[3] >= 0.)
        {
          std::cout << " rgba(" << rgba[0] << "," << rgba[1] << "," << rgba[2] << "," << rgba[3]
                    << ")";
        }
        auto sub = p->subphrases(); // force subphrases to get built, though we may not visit them
        (void)sub;
        std::cout << "\n";
        std::ostringstream idxStr;
        for (int ii : idx)
        {
          idxStr << " " << ii;
        }
        smtkTest(p->at(idx) == p, "Index " << idxStr.str()
                                           << " passed to visitor did not produce phrase!");
      }
      return indent > maxIndent ? 1 : 0;
    });

  if (!dataArgs.empty())
  {
    // We know what model the test loads... move some phrases around to test PhraseModel's
    // updateChildren() and its observers.
    testUpdateChildren(phraseModel);
    // Don't leak
    free(dataArgs[1]);
  }

  return 0;
}

smtkComponentInitMacro(smtk_polygon_session);
