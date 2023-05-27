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

#include "smtk/session/polygon/Registrar.h"

#include "smtk/plugin/Registry.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/SessionRef.h"
#include "smtk/model/operators/EntityGroupOperation.h"

#include "smtk/operation/operators/SetProperty.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <cstdlib>
#include <cstring>

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
      "Index " << idxStr.str() << " passed to visitor did not return the correct phrase!");
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
    argv = dataArgs.data();
  }
  auto rsrcMgr = smtk::resource::Manager::create();
  auto operMgr = smtk::operation::Manager::create();
  operMgr->registerResourceManager(rsrcMgr);

  auto registry = smtk::plugin::
    Registry<smtk::session::polygon::Registrar, smtk::resource::Manager, smtk::operation::Manager>(
      rsrcMgr, operMgr);

  // I. Construct a ComponentPhraseModel that displays edges and faces. Load some geometry.
  auto phraseModel = smtk::view::ComponentPhraseModel::create();
  phraseModel->setMutableAspects(
    static_cast<int>(PhraseContent::ContentType::TITLE) |
    static_cast<int>(PhraseContent::ContentType::COLOR));
  std::multimap<std::string, std::string> filters;
  filters.insert(
    std::make_pair(std::string("smtk::session::polygon::Resource"), std::string("edge")));
  filters.insert(
    std::make_pair(std::string("smtk::session::polygon::Resource"), std::string("face")));
  phraseModel->setComponentFilters(filters);
  phraseModel->addSource({ rsrcMgr, operMgr });
  smtk::resource::ResourceArray rsrcs;
  for (int i = 1; i < argc; i++)
  {
    auto rdr = operMgr->create<smtk::operation::ReadResource>();
    rdr->parameters()->findFile("filename")->setValue(argv[i]);
    rdr->operate();
    // rsrcs.push_back(rsrcMgr->read<smtk::session::polygon::Resource>(argv[1]));
  }
  smtk::resource::ResourcePtr rsrc = nullptr;
  rsrcMgr->visit([&rsrc](smtk::resource::Resource& rr) {
    rsrc = rr.shared_from_this();
    return smtk::common::Processing::STOP;
  });
  smtkTest(!!rsrc, "Unable to discern that a resource was loaded.");
  auto firstSize = phraseModel->root()->subphrases().size();

  test(
    phraseModel->root()->root() == phraseModel->root(),
    "Model's root phrase was not root of tree.");
  phraseModel->root()->visitChildren(printer);

  // I.a. Verify that top-level phrases have a mutability as specified.
  int wantMutability = phraseModel->mutableAspects();
  auto& topLevel = phraseModel->root()->subphrases();
  for (const auto& entry : topLevel)
  {
    PhraseContent::ContentType attribs[] = {
      PhraseContent::ContentType::TITLE,        PhraseContent::ContentType::SUBTITLE,
      PhraseContent::ContentType::COLOR,        PhraseContent::ContentType::VISIBILITY,
      PhraseContent::ContentType::ICON_LIGHTBG, PhraseContent::ContentType::ICON_DARKBG
    };
    int editable = 0;
    for (auto attrib : attribs)
    {
      editable |= entry->content()->editable(attrib) ? static_cast<int>(attrib) : 0;
    }
    smtkTest(editable == wantMutability, "Mutability and editability mismatched");
  }
  std::cout << "Top-level mutability and editability were properly matched.\n";

  // II. Now change the filters to a reduced set and verify there are fewer phrases.
  filters.erase(filters.begin());
  std::cout << "---\n";

  phraseModel->setComponentFilters(filters);
  auto reducedSize = phraseModel->root()->subphrases().size();
  std::cout << "initial size " << firstSize << " reduced to " << reducedSize << "\n";
  smtkTest(reducedSize < firstSize, "Expected fewer phrases without the edge filter.");
  phraseModel->root()->visitChildren(printer);

  if (!dataArgs.empty())
  {
    std::cout << "---\n";
    phraseModel->root()->visitChildren(printer);
    smtkTest(
      !phraseModel->root()->subphrases().empty(),
      "Expected a non-empty list with a non-null active resource.");
    smtkTest(
      phraseModel->root()->subphrases().size() == reducedSize,
      "Expected the same number of phrases as earlier.");

    // IV. Test updates due to operations.

    // Grab the faces as reported to us:
    smtk::resource::ComponentArray faces;
    smtk::model::EntityPtr fmod; // model owning the faces
    phraseModel->root()->visitChildren(
      [&faces, &fmod](DescriptivePhrasePtr p, const std::vector<int>& /*unused*/) {
        auto comp = p->relatedComponent();
        faces.push_back(comp);
        auto ment = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
        fmod = ment ? ment->owningModel() : nullptr;
        return 0;
      });
    smtkTest(!fmod || !faces.empty(), "Cannot test grouping without groupees.");

    // Configure the phrase model to show groups as well as faces at the top level:
    filters.insert(
      std::make_pair(std::string("smtk::session::polygon::Resource"), std::string("group")));
    phraseModel->setComponentFilters(filters);

    {
      // IV.a. Test that creation inserts phrases properly.
      // Add a filter so groups will be listed at the top level:
      // Create a new group
      auto sizeBeforeAdd = phraseModel->root()->subphrases().size();
      auto op = operMgr->create<smtk::model::EntityGroupOperation>();
      auto pm = op->parameters();
      pm->findString("Operation")->setValue("Create");
      pm->findString("group name")->setValue("everything");
      pm->findVoid("Face")->setIsEnabled(true);
      // pm->findModelEntity("cell to add")->setObjectValues(faces.begin(), faces.end());
      pm->associations()->appendValue(fmod);
      auto res = op->operate();
      bool ok =
        (res->findInt("outcome")->value(0) ==
         static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      std::cout << "add group success " << (ok ? "T" : "F") << "\n";
      auto sizeAfterAdd = phraseModel->root()->subphrases().size();
      std::cout << "---\n";
      phraseModel->root()->visitChildren(printer);
      smtkTest(
        sizeAfterAdd == sizeBeforeAdd + 1,
        "Adding a group should increase number of phrases by 1.");
    }
    {
      // IV.b. Test that modification reorders phrases properly.
      // Change entity names in non-alphabetical ways.
      // (i) Move from middle to end of list:
      auto op = operMgr->create<smtk::operation::SetProperty>();
      auto pm = op->parameters();
      auto value = pm->findString("string value");
      pm->associate(faces[0]);
      pm->findString("name")->setValue("name");
      value->appendValue("zzz");
      auto res = op->operate();
      bool ok =
        (res->findInt("outcome")->value(0) ==
         static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      std::cout << "set property success " << (ok ? "T" : "F") << "\n";
      std::cout << "---\n";
      phraseModel->root()->visitChildren(printer);
      smtkTest(
        phraseModel->root()->subphrases().back()->title() == "zzz",
        "Did not move renamed face to end.");

      // (ii) Make a change that requires no move, but which
      //      could cause a move if sorting alphabetically rather
      //      than by component type and *then* name.
      pm->disassociate(faces[0]);
      pm->associate(faces[1]);
      value->setValue("aaa");
      res = op->operate();
      ok =
        (res->findInt("outcome")->value(0) ==
         static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      std::cout << "set property success " << (ok ? "T" : "F") << "\n";
      std::cout << "---\n";
      phraseModel->root()->visitChildren(printer);
      smtkTest(
        phraseModel->root()->subphrases()[5]->title() == "aaa",
        "Improper phrase title \"" << phraseModel->root()->subphrases()[5]->title() << "\".");

      // (iii) Move something from the middle to the beginning
      //       of the list.
      pm->disassociate(faces[1]);
      auto gp = fmod->modelResource()->findEntitiesByProperty("name", "epic")[0].entityRecord();
      pm->associate(gp);
      res = op->operate();
      ok =
        (res->findInt("outcome")->value(0) ==
         static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      std::cout << "set property success " << (ok ? "T" : "F") << "\n";
      std::cout << "---\n";
      phraseModel->root()->visitChildren(printer);
      smtkTest(
        phraseModel->root()->subphrases().front()->title() == "aaa",
        "Did not move renamed group to beginning.");

      // (iv) Move something from the beginning to the middle
      value->setValue("epic");
      res = op->operate();
      ok =
        (res->findInt("outcome")->value(0) ==
         static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      std::cout << "set property success " << (ok ? "T" : "F") << "\n";
      std::cout << "---\n";
      phraseModel->root()->visitChildren(printer);
      smtkTest(
        phraseModel->root()->subphrases().front()->title() != "epic",
        "Did not move renamed group to middle.");
      smtkTest(
        phraseModel->root()->subphrases()[2]->title() == "epic",
        "Did not move renamed group to middle.");

      // (v) Move something from the end to the middle
      pm->disassociate(gp);
      pm->associate(faces[0]);
      res = op->operate();
      ok =
        (res->findInt("outcome")->value(0) ==
         static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      std::cout << "set property success " << (ok ? "T" : "F") << "\n";
      std::cout << "---\n";
      phraseModel->root()->visitChildren(printer);
      smtkTest(
        phraseModel->root()->subphrases()[6]->title() == "epic",
        "Did not move renamed group to middle.");
    }
    {
      // First sort all phrases alphabetically
      phraseModel->setSortFunction(DescriptivePhrase::compareByTitle);
      smtkTest(
        phraseModel->root()->subphrases()[0]->title() == "aaa",
        "Did not sort phrases alphabetically.");
      smtkTest(
        phraseModel->root()->subphrases()[4]->title() == "epic",
        "Did not sort phrases alphabetically.");

      // then sort by type and then title
      phraseModel->setSortFunction(DescriptivePhrase::compareByTypeThenTitle);
      smtkTest(
        phraseModel->root()->subphrases()[0]->title() == "background",
        "Did not sort phrases alphabetically.");
      smtkTest(
        phraseModel->root()->subphrases()[6]->title() == "epic",
        "Did not sort phrases alphabetically.");
    }

    // Don't leak
    free(dataArgs[1]);
  }

  return 0;
}
