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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/attribute/operators/Signal.h"

#include "smtk/resource/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/groups/InternalGroup.h"
#include "smtk/plugin/Registry.h"

#include "smtk/view/json/jsonView.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <tuple>

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
  bool action(const DescriptivePhrase* phrase, const BadgeAction& act) override
  {
    if (!dynamic_cast<const BadgeActionToggle*>(&act))
    {
      return false; // we only support single clicks.
    }
    if (phrase && phrase->relatedComponent())
    {
      // Do something as evidence the call was made:
      ++BadgeA::testCounter;
      return true;
    }
    return false;
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
  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override
  {
    return phrase ? "yes" : "no";
  }
  bool action(const DescriptivePhrase* phrase, const BadgeAction& act) override
  {
    if (!dynamic_cast<const BadgeActionToggle*>(&act))
    {
      return false; // we only support single clicks.
    }
    if (phrase && !phrase->relatedComponent() && phrase->relatedResource())
    {
      // Do something as evidence the call was made:
      ++BadgeB::testCounter;
      return true;
    }
    return false;
  }
};
int BadgeB::testCounter = 0;

int unitBadge(int argc, char* argv[])
{
  smtk::io::Logger::instance().setFlushToStdout(true);
  auto viewManager = smtk::view::Manager::create();
  viewManager->badgeFactory().registerTypes<std::tuple<BadgeA, BadgeB>>();
  json j = {
    { "Name", "Test" },
    { "Type", "smtk::view::ResourcePhraseModel" },
    { "Component",
      { { "Name", "Details" },
        { "Type", "smtk::view::ResourcePhraseModel" },
        { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
        { "Children",
          { { { "Name", "PhraseModel" },
              { "Attributes", { { "Type", "smtk::view::ResourcePhraseModel" } } },
              { "Children",
                { { { "Name", "SubphraseGenerator" }, { "Attributes", { { "Type", "default" } } } },
                  { { "Name", "Badges" },
                    { "Children",
                      {
                        { { "Name", "Badge" }, { "Attributes", { { "Type", "BadgeA" } } } },
                        { { "Name", "Comment" }, { "Text", "Test that comments are allowed." } },
                        { { "Name", "Badge" }, { "Attributes", { { "Type", "BadgeB" } } } },
                        { { "Name", "Badge" },
                          { "Attributes", { { "Type", "smtk::view::AssociationBadge" } } },
                          { "Children",
                            { {
                                { "Name", "AppliesTo" },
                                { "Attributes",
                                  { { "Resource", "smtk::model::Resource" },
                                    { "Component", "edge" } } },
                              },
                              {
                                { "Name", "Requires" },
                                { "Attributes", { { "Definition", "BoundaryCondition" } } },
                              } } } },
                      } } } } } } } } } }
  };
  smtk::view::ConfigurationPtr viewConfig = j;
  smtk::resource::ManagerPtr resourceManager;
  smtk::operation::ManagerPtr operationManager;

  auto phraseModel =
    loadTestData(argc, argv, viewManager, *viewConfig, dataArgs, resourceManager, operationManager);
  // BadgeSet& badgeSet(const_cast<BadgeSet&>(phraseModel->badges()));
  // badgeSet.configure(viewConfig, viewManager);
  auto resource = phraseModel->root()->subphrases()[0]->relatedResource();
  auto registry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);
  auto attRsrc = resourceManager->create<smtk::attribute::Resource>();
  attRsrc->setName("Attribute Test Resource");
  auto defBC = attRsrc->createDefinition("BoundaryCondition");
  auto assoc = defBC->createLocalAssociationRule();
  assoc->setAcceptsEntries("smtk::model::Resource", "edge", true);
  assoc->setIsExtensible(true);
  assoc->setMaxNumberOfValues(0);
  auto defDBC = attRsrc->createDefinition("Dirichlet", "BoundaryCondition");
  auto defNBC = attRsrc->createDefinition("Neumann", "BoundaryCondition");
  auto attDBC = attRsrc->createAttribute(defDBC);
  auto attNBC = attRsrc->createAttribute(defNBC);
  auto edges = resource->filter("edge");
  // Associate all but 2 edges with a boundary condition
  for (const auto& edge : edges)
  {
    if (edge->name() != "edge 0" && edge->name() != "edge 9")
    {
      if (edge->name() < "edge 20")
      {
        attDBC->associate(edge);
      }
      else
      {
        attNBC->associate(edge);
      }
    }
  }

  // create a Signal operation that will let Observers know that an
  // attribute was created, modified, or removed.
  smtk::attribute::Registrar::registerTo(operationManager);
  auto signalOp = operationManager->create<smtk::attribute::Signal>();
  if (signalOp)
  {
    auto compsItem = signalOp->parameters()->findComponent("created");
    compsItem->appendValue(attDBC);
    compsItem->appendValue(attNBC);
    signalOp->parameters()->findResource("resourcesCreated")->appendValue(attRsrc);
    signalOp->operate();
  }

  int bcBadgeCounter = 0;

  smtk::view::BadgeSet& badgeSet = phraseModel->badges();
  phraseModel->root()->visitChildren(
    [&badgeSet, &bcBadgeCounter](DescriptivePhrasePtr p, const std::vector<int>& idx) -> int {
      int indent = static_cast<int>(idx.size()) * 2;
      if (p)
      {
        auto badges = badgeSet.badgesFor(p.get());
        std::cout << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << "): "
                  << " badges:";
        for (smtk::view::Badge* badge : badges)
        {
          std::string tip = badge->tooltip(p.get());
          std::cout << " " << tip;
          // Verify that the AssociationBadge works by counting the number
          // of times edge 0 and edge 9 are listed:
          if (tip.find("BoundaryCondition") != std::string::npos)
          {
            ++bcBadgeCounter;
          }
          // Exercise each badge's action:
          badge->action(p.get(), BadgeActionToggle());
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
  std::cout << "AssociationBadge applied count " << bcBadgeCounter << "\n";
  if (!dataArgs.empty())
  {
    smtkTest(BadgeA::testCounter == 98, "Expected 98 clicks on BadgeA.");
    smtkTest(BadgeB::testCounter == 2, "Expected 2 clicks on BadgeB.");
    smtkTest(bcBadgeCounter == 4, "Expected 4 association warnings.");

    // Don't leak
    free(dataArgs[1]);
  }
  return 0;
}
