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
#include "smtk/view/PhraseContent.h"
#include "smtk/view/PhraseModelObserver.h"
#include "smtk/view/Registrar.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/testing/cxx/utility.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/model/SessionRef.h"

#include "smtk/plugin/Registry.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/view/json/jsonView.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <cstdlib>
#include <cstring>

using json = nlohmann::json;
using namespace smtk::common;
using namespace smtk::view;
using namespace smtk::io;

namespace
{
int numRemoved = 0;
int numInserted = 0;
int numMoved = 0;

void checkCounts(const std::array<int, 3>& counts, const std::string& event)
{
  std::cout << "\nUpon " << event << ":\n"
            << "  inserted: " << numInserted << "  removed: " << numRemoved
            << " moved: " << numMoved << "\n"
            << "  expected: " << counts[0] << "         : " << counts[1] << "      : " << counts[2]
            << "\n---\n";
  test(numInserted == counts[0], "Unexpected insertion count.");
  test(numRemoved == counts[1], "Unexpected insertion count.");
  test(numMoved == counts[2], "Unexpected insertion count.");
  numRemoved = 0;
  numInserted = 0;
  numMoved = 0;
}

class StringPhraseContent : public smtk::view::PhraseContent
{
public:
  smtkTypeMacro(StringPhraseContent);
  smtkSharedPtrCreateMacro(PhraseContent);
  Ptr setup(const std::string& title)
  {
    m_title = title;
    return shared_from_this();
  }
  ~StringPhraseContent() override = default;

  static DescriptivePhrasePtr createPhrase(
    const std::string& title,
    DescriptivePhrasePtr parent = DescriptivePhrasePtr())
  {
    auto result =
      DescriptivePhrase::create()->setup(DescriptivePhraseType::STRING_PROPERTY_VALUE, parent);
    auto content = StringPhraseContent::create()->setup(title);
    result->setContent(content);
    content->setLocation(result);
    return result;
  }

  std::string stringValue(ContentType contentType) const override
  {
    return contentType == TITLE ? m_title : std::string();
  }
  bool operator==(const PhraseContent& other) const override
  {
    return m_title == static_cast<const StringPhraseContent&>(other).m_title;
  }

protected:
  std::string m_title;
};

class DummyPhraseModel : public smtk::view::PhraseModel
{
public:
  smtkTypeMacro(DummyPhraseModel);
  smtkSuperclassMacro(smtk::view::PhraseModel);
  DummyPhraseModel()
    : m_root(DescriptivePhrase::create())
  {
  }
  DummyPhraseModel(const smtk::view::Configuration* config, smtk::view::Manager* manager)
    : Superclass(config, manager)
    , m_root(DescriptivePhrase::create())
  {
    auto generator = PhraseModel::configureSubphraseGenerator(config, manager);
    m_root->setDelegate(generator);
  }
  DescriptivePhrasePtr root() const override { return m_root; }

protected:
  DescriptivePhrasePtr m_root;
};
} // namespace

void print(const PhraseModel* phraseModel)
{
  phraseModel->root()->visitChildren(
    [](DescriptivePhrasePtr p, const std::vector<int>& idx) -> int {
      int indent = static_cast<int>(idx.size()) * 2;
      if (p)
      {
        std::cout << std::string(indent, ' ') << p->title() << "\n";
      }
      return 0;
    });
}

void loadPhrases(PhraseModel* phraseModel, const std::vector<std::string>& titles)
{
  std::vector<DescriptivePhrasePtr> phrases;
  phrases.reserve(titles.size());
  for (const auto& title : titles)
  {
    phrases.push_back(StringPhraseContent::createPhrase(title));
  }
  std::vector<int> path;
  int abort = 0;
  auto key = phraseModel->observers().insert([&abort](
                                               DescriptivePhrasePtr phr,
                                               PhraseModelEvent event,
                                               const std::vector<int>& path1,
                                               const std::vector<int>& path2,
                                               const std::vector<int>& range) {
    std::cout << "  ";
    switch (event)
    {
      case PhraseModelEvent::ABOUT_TO_INSERT:
        numInserted = 0;
        std::cout << "will insert";
        break;
      case PhraseModelEvent::INSERT_FINISHED:
        numInserted = range[1] - range[0] + 1;
        std::cout << "did  insert";
        break;
      case PhraseModelEvent::ABOUT_TO_REMOVE:
        numRemoved = 0;
        std::cout << "will remove";
        break;
      case PhraseModelEvent::REMOVE_FINISHED:
        numRemoved = range[1] - range[0] + 1;
        std::cout << "did  remove";
        break;
      case PhraseModelEvent::ABOUT_TO_MOVE:
        std::cout << "will move";
        break;
      case PhraseModelEvent::MOVE_FINISHED:
        numMoved += range[1] - range[0] + 1;
        std::cout << "did  move";
        break;
      case PhraseModelEvent::PHRASE_MODIFIED:
        std::cout << "refresh";
        break;
    }
    std::cout << "   (";
    for (const auto& pp : path1)
    {
      std::cout << " " << pp;
    }
    std::cout << " )  (";
    for (const auto& pp : path2)
    {
      std::cout << " " << pp;
    }
    std::cout << " )  {";
    for (const auto& rr : range)
    {
      std::cout << " " << rr;
    }
    std::cout << " }\n";
    if (event == PhraseModelEvent::ABOUT_TO_MOVE)
    {
      const auto& phrases(phr->subphrases());
      std::cout << "       move " << phrases[range[0]]->title() << " +" << (range[1] - range[0])
                << " to just before "
                << (range[2] >= static_cast<int>(phrases.size()) ? "end"
                                                                 : phrases[range[2]]->title())
                << "\n";
      // print(phr->phraseModel().get());
    }

    ++abort;
    if (abort > 200)
    {
      // Don't take forever if a regression occurs, just error out.
      throw std::runtime_error("Grrk!");
    }
  });
  phraseModel->updateChildren(phraseModel->root(), phrases, path);
  phraseModel->observers().erase(key);
}

int unitPhraseModel(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  json j = { { "Name", "Test" },
             { "Type", "DummyPhraseModel" },
             { "Component",
               { { "Name", "Details" },
                 { "Type", "DummyPhraseModel" },
                 { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
                 { "Children",
                   { { { "Name", "PhraseModel" },
                       { "Attributes", { { "Type", "DummyPhraseModel" } } },
                       { "Children",
                         { { { "Name", "SubphraseGenerator" },
                             { "Attributes", { { "Type", "default" } } } } } } } } } } } };
  auto viewManager = smtk::view::Manager::create();
  auto registry = smtk::plugin::addToManagers<smtk::view::Registrar>(viewManager);
  viewManager->phraseModelFactory().registerType<DummyPhraseModel>();
  smtk::view::ConfigurationPtr viewConfig = j;
  auto phraseModel = viewManager->phraseModelFactory().createFromConfiguration(viewConfig.get());

  // Test updateChildren bulk insertion.
  std::vector<std::string> titles0{ "a",
                                    "p",
                                    "b",
                                    "c",
                                    "d",
                                    "e",
                                    "f",
                                    "g",
                                    "h",
                                    "i",
                                    "j",
                                    "k",
                                    "m",
                                    "n",
                                    "o",
                                    "q",
                                    "r",
                                    "r",
                                    /* purposeful duplication to cause trouble */ "s",
                                    "t",
                                    "u",
                                    "v",
                                    "w",
                                    "x",
                                    "l",
                                    "y",
                                    "z" };
  loadPhrases(phraseModel.get(), titles0);
  print(phraseModel.get());
  checkCounts({ 27, 0, 0 }, "initial insertion");

  // Test updateChildren phrase-moves that resulted in infinite loop of old technique.
  std::vector<std::string> titles1{ "a",
                                    "b",
                                    "c",
                                    "d",
                                    "e",
                                    "f",
                                    "g",
                                    "h",
                                    "i",
                                    "y",
                                    "j",
                                    "k",
                                    "l",
                                    "m",
                                    "n",
                                    "o",
                                    "p",
                                    "q",
                                    "r",
                                    "r",
                                    /* purposeful duplication to cause trouble */ "w",
                                    "v",
                                    "u",
                                    "x",
                                    "s",
                                    "t",
                                    "z" };
  loadPhrases(phraseModel.get(), titles1);
  print(phraseModel.get());
  checkCounts({ 0, 1, 33 }, "uniqification+reorder");

  // Test updateChildren deletion and moves to beginning and end of list.
  std::vector<std::string> titles2{ "z", "b", "c", "a" };
  loadPhrases(phraseModel.get(), titles2);
  print(phraseModel.get());
  checkCounts({ 0, 22, 4 }, "deletion+reorder");

  // Test updateChildren removal of everything.
  std::vector<std::string> titles3;
  loadPhrases(phraseModel.get(), titles3);
  print(phraseModel.get());
  checkCounts({ 0, 4, 0 }, "cleanup");

  return 0;
}
