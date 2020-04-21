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

#include "smtk/view/BadgeFactory.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"
#include "smtk/view/ObjectIconBadge.h"
#include "smtk/view/Registrar.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/view/json/jsonView.h"

#include "smtk/common/testing/cxx/helpers.h"

static int maxIndent = 6;
static std::vector<char*> dataArgs;

using json = nlohmann::json;
using namespace smtk::view;

class BadgeA : public ObjectIconBadge
{
public:
  smtkTypeMacro(BadgeA);
  BadgeA() = default;
  BadgeA(BadgeSet& parent, const Configuration::Component& config)
    : ObjectIconBadge(parent, config)
  {
  }
  ~BadgeA() override = default;

  static int testCounter;

  std::string tooltip(const DescriptivePhrase*) const override
  {
    std::string tip = "A"; // "A(" + this->ObjectIconBadge::tooltip(phrase) + ")";
    return tip;
  }
  void action(const DescriptivePhrase* phrase) const override
  {
    if (phrase && phrase->relatedComponent())
    {
      // Do something as evidence the call was made:
      ++BadgeA::testCounter;
    }
  }
};
int BadgeA::testCounter = 0;

class BadgeB : public Badge
{
public:
  smtkTypeMacro(BadgeB);
  BadgeB() = default;
  BadgeB(BadgeSet&, const Configuration::Component&) {}
  ~BadgeB() override = default;

  static int testCounter;

  bool appliesToPhrase(const DescriptivePhrase* phrase) const override
  {
    return phrase ? !static_cast<bool>(phrase->relatedComponent()) : false;
  }
  std::string tooltip(const DescriptivePhrase*) const override { return "B"; }
  std::string svg(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override
  {
    return phrase ? "yes" : "no";
  }
  void action(const DescriptivePhrase* phrase) const override
  {
    if (phrase && !phrase->relatedComponent() && phrase->relatedResource())
    {
      // Do something as evidence the call was made:
      ++BadgeB::testCounter;
    }
  }
};
int BadgeB::testCounter = 0;

int unitBadge(int argc, char* argv[])
{
  auto viewManager = smtk::view::Manager::create();
  viewManager->badgeFactory().registerBadge<BadgeA>();
  viewManager->badgeFactory().registerBadge<BadgeB>();
  json j = { { "Name", "Test" }, { "Type", "smtk::view::ResourcePhraseModel" },
    { "Component",
      { { "Name", "Details" }, { "Type", "smtk::view::ResourcePhraseModel" },
        { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
        { "Children",
          { { { "Name", "PhraseModel" },
            { "Attributes", { { "Type", "smtk::view::ResourcePhraseModel" } } },
            { "Children",
              { { { "Name", "SubphraseGenerator" }, { "Attributes", { { "Type", "default" } } } },
                { { "Name", "Badges" },
                  { "Children",
                    {
                      { { "Name", "ignored" }, { "Attributes", { { "Type", "BadgeA" } } } },
                      { { "Name", "ignored" }, { "Attributes", { { "Type", "BadgeB" } } } },
                    } } } } } } } } } } };
  smtk::view::ConfigurationPtr viewConfig = j;
  auto phraseModel = loadTestData(argc, argv, viewManager, *viewConfig, dataArgs);
  // BadgeSet& badgeSet(const_cast<BadgeSet&>(phraseModel->badges()));
  // badgeSet.configure(viewConfig, viewManager);

  const auto& badgeSet = phraseModel->badges();
  phraseModel->root()->visitChildren(
    [&phraseModel, &badgeSet](DescriptivePhrasePtr p, const std::vector<int>& idx) -> int {
      int indent = static_cast<int>(idx.size()) * 2;
      if (p)
      {
        auto badges = badgeSet.badgesFor(p.get());
        std::cout << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << "): "
                  << " badges:";
        for (const auto& badge : badges)
        {
          std::cout << " " << badge->tooltip(p.get());
          // Exercise each badge's action:
          badge->action(p.get());
        }
        auto sub = p->subphrases(); // force subphrases to get built, though we may not visit them
        (void)sub;
        std::cout << "\n";
        std::ostringstream idxStr;
        for (int ii : idx)
        {
          idxStr << " " << ii;
        }
      }
      return indent > maxIndent ? 1 : 0;
    });

  std::cout << "BadgeA action count " << BadgeA::testCounter << "\n";
  std::cout << "BadgeB action count " << BadgeB::testCounter << "\n";
  if (!dataArgs.empty())
  {
    smtkTest(BadgeA::testCounter == 96, "Expected 96 clicks on BadgeA");
    smtkTest(BadgeB::testCounter == 1, "Expected 1 click on BadgeB");

    // Don't leak
    free(dataArgs[1]);
  }
  return 0;
}
