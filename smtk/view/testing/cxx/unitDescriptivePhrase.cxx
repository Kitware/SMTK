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

#include "smtk/resource/Manager.h"
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/operation/Manager.h"

#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

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
  (void)phraseModel;
  auto root = phraseModel->root();
  auto phrResources = root->subphrases();
  std::cout << "rsrc " << phrResources[0]->title() << "\n";
  auto phrModels = phrResources[0]->subphrases();
  std::cout << "modl " << phrModels[0]->title() << "\n";
  auto phrModelSummary = phrModels[0]->subphrases();
  std::cout << "summ " << phrModelSummary[1]->title() << "\n";
  DescriptivePhrases phrFaces = phrModelSummary[1]->subphrases();
  phrFaces.erase(phrFaces.begin() + 2, phrFaces.begin() + 6);
  std::vector<int> idx;
  idx.push_back(0);
  idx.push_back(0);
  idx.push_back(1);
  phraseModel->updateChildren(phrModelSummary[1], phrFaces, idx);
  std::cout << "There are " << phrModelSummary[1]->subphrases().size() << " faces\n";
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
  auto phraseModel = smtk::view::ResourcePhraseModel::create();
  phraseModel->addSource(rsrcMgr, operMgr);
  auto rsrcs = smtk::resource::testing::loadTestResources(rsrcMgr, argc, argv);

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
