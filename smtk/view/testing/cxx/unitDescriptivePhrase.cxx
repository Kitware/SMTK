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
#include "smtk/view/Manager.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/testing/cxx/utility.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/model/SessionRef.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/view/json/jsonView.h"

#include <fstream>
#include <iostream>
#include <string>

#include <cstdlib>
#include <cstring>

using json = nlohmann::json;
using namespace smtk::common;
using namespace smtk::view;
using namespace smtk::io;

static int maxIndent = 6;
static std::vector<char*> dataArgs;

void testUpdateChildren(smtk::view::PhraseModel::Ptr phraseModel)
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
  std::cout << "Number of Subphrases in " << phrModelSummary[3]->title() << " is "
            << phrFaces.size() << std::endl;
  for (const auto& phrase : phrFaces)
  {
    std::cout << "\t" << phrase->title() << std::endl;
  }
  std::cout << "Removing 3 entries from " << phrModelSummary[3]->title() << "\n";
  phrFaces.erase(phrFaces.begin() + 2, phrFaces.begin() + 5);
  std::vector<int> idx;
  idx.push_back(0);
  idx.push_back(0);
  idx.push_back(1);
  int numObservations = 0;
  phraseModel->observers()
    .insert(
      [&numObservations](
        DescriptivePhrasePtr pp,
        PhraseModelEvent pe,
        const std::vector<int>& src,
        const std::vector<int>& dst,
        const std::vector<int>& delta) {
        (void)src;
        (void)dst;
        std::cout << "Phrase event " << static_cast<int>(pe) << " " << pp->title() << " "
                  << delta[0] << " " << delta[1] << "\n";
        smtkTest(delta.size() == 2, "Expecting phrase update to specify range of removed entries.");
        smtkTest(delta[0] == 2 && delta[1] == 4, "Expecting delta = [2, 4].");
        ++numObservations;
      },
      0,    // assign a neutral priority
      false // Do not immediately notify of existing items.
      )
    .release();
  phraseModel->updateChildren(phrModelSummary[3], phrFaces, idx);
  smtkTest(numObservations == 2, "Expected to observe removal of rows in 2 steps.");
  std::cout << "There are " << phrModelSummary[3]->subphrases().size() << " entries remaining.\n";
}

int unitDescriptivePhrase(int argc, char* argv[])
{
  json j = { { "Name", "Test" },
             { "Type", "smtk::view::ResourcePhraseModel" },
             { "Component",
               { { "Name", "Details" },
                 { "Type", "smtk::view::ResourcePhraseModel" },
                 { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
                 { "Children",
                   { { { "Name", "PhraseModel" },
                       { "Attributes", { { "Type", "smtk::view::ResourcePhraseModel" } } },
                       { "Children",
                         { { { "Name", "SubphraseGenerator" },
                             { "Attributes", { { "Type", "default" } } } } } } } } } } } };
  auto viewManager = smtk::view::Manager::create();
  smtk::view::ConfigurationPtr viewConfig = j;
  smtk::resource::ManagerPtr resourceManager;
  smtk::operation::ManagerPtr operationManager;
  auto phraseModel =
    loadTestData(argc, argv, viewManager, *viewConfig, dataArgs, resourceManager, operationManager);
  test(
    phraseModel->root()->root() == phraseModel->root(),
    "Model's root phrase was not root of tree.");
  phraseModel->root()->visitChildren(
    [](DescriptivePhrasePtr p, const std::vector<int>& idx) -> int {
      int indent = static_cast<int>(idx.size()) * 2;
      if (p)
      {
        std::cout << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")";
        auto sub = p->subphrases(); // force subphrases to get built, though we may not visit them
        (void)sub;
        std::cout << "\n";
        std::ostringstream idxStr;
        for (int ii : idx)
        {
          idxStr << " " << ii;
        }
        smtkTest(
          p->at(idx) == p,
          "Index " << idxStr.str() << " passed to visitor did not produce phrase!");
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
