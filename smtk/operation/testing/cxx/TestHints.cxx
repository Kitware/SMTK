//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Launcher.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <chrono>
#include <thread>

#include "TestHints_xml.h"

namespace
{

class MyOperation : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(MyOperation);
  smtkCreateMacro(MyOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  MyOperation() = default;
  ~MyOperation() override = default;

  Result operateInternal() override;

  void generateSummary(Result&) override {}

  const char* xmlDescription() const override;
};

MyOperation::Result MyOperation::operateInternal()
{
  using namespace smtk::operation;
  // Do our "work"
  auto resource = smtk::model::Resource::create();
  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();
  std::this_thread::sleep_for(std::chrono::seconds(sleep));

  // Create a result
  auto result = this->createResult(Outcome::SUCCEEDED);
  result->findResource("resourcesCreated")->appendValue(resource);

  // Add some hints to the result.
  std::set<smtk::resource::Resource::Ptr> hintTargets{ resource };
  auto hint = addSelectionHint(result, hintTargets);
  ::test(!!hint, "Failed to add selection hint.");
  hint = addRenderFocusHint(result, hintTargets);
  ::test(!!hint, "Failed to add render-focus hint.");
  hint = addBrowserScrollHint(result, hintTargets);
  ::test(!!hint, "Failed to add browser-scroll hint.");
  hint = addBrowserExpandHint(result, hintTargets);
  ::test(!!hint, "Failed to add browser-expand hint.");
  // Add another selection hint to test that we can; also
  // test passing optional parameters.
  hint = addSelectionHint(
    result,
    hintTargets,
    smtk::view::SelectionAction::FILTERED_ADD,
    2,
    /* bitwise */ true,
    /* ephemeral */ true);
  ::test(!!hint, "Failed to add selection hint.");

  // Add "render visibility hints": one to show and one to hide objects.
  hint = addRenderVisibilityHint(result, hintTargets, /*show*/ true);
  hint = addRenderVisibilityHint(result, hintTargets, /*show*/ false);
  return result;
}

const char* MyOperation::xmlDescription() const
{
  return TestHints_xml;
}

int hintCount = 0;

std::function<void(const smtk::attribute::ReferenceItem::Ptr&)> printHints(
  const std::string& hintType,
  const std::vector<std::string>& itemsToPrint = {})
{
  return [hintType, &itemsToPrint](const smtk::attribute::ReferenceItem::Ptr& assoc) {
    ++hintCount;
    std::cout << "  " << hintType << " targets " << assoc << " ("
              << (assoc ? assoc->numberOfValues() : 0) << " object)\n";
    for (const auto& itemName : itemsToPrint)
    {
      auto item = assoc->attribute()->find(itemName);
      if (!item)
      {
        std::cerr << "  " << hintType << " no item \"" << itemName << "\"!\n";
        hintCount += 100;
        continue;
      }
      std::cout << "    " << itemName << " (" << item->typeName() << ")";
      if (item->isOptional())
      {
        std::cout << " enabled? " << item->isEnabled();
      }
      std::cout << "\n";
    }
  };
}

} // namespace

int TestHints(int /*unused*/, char** const /*unused*/)
{
  smtk::io::Logger::instance().setFlushToStdout(true);
  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register MyOperation
  operationManager->registerOperation<MyOperation>("MyOperation");

  // Construct an instance of MyOperation
  smtk::operation::Operation::Ptr myOperation = operationManager->create<MyOperation>();
  auto outcome = myOperation->safeOperate(
    [&](smtk::operation::Operation& op, smtk::operation::Operation::Result result) {
      using namespace smtk::operation;
      std::cout << "Operation " << &op << " hints:\n";
      visitSelectionHints(
        result,
        [&](
          const smtk::attribute::ReferenceItem::Ptr& assoc,
          smtk::view::SelectionAction action,
          int value,
          bool bitwise,
          bool ephemeral) {
          ++hintCount;
          std::cout << "  selection hint " << assoc << " target "
                    << (assoc ? assoc->numberOfValues() : 0) << " action "
                    << static_cast<int>(action) << " value " << value << " bitwise "
                    << (bitwise ? "T" : "F") << " ephemeral " << (ephemeral ? "T" : "F") << "\n";
        });
      visitFocusHintsOfType(result, "browser expand hint", printHints("browser expand hint"));
      visitFocusHintsOfType(result, "browser scroll hint", printHints("browser scroll hint"));
      visitFocusHintsOfType(result, "render focus hint", printHints("render focus hint"));
      visitAssociationHintsOfType(
        result, "render visibility hint", printHints("render visibility hint", { "show" }));
    });
  std::cout << "Operation " << myOperation.get() << " outcome " << static_cast<int>(outcome)
            << "\n";
  ::test(
    outcome == smtk::operation::Operation::Outcome::SUCCEEDED,
    "Expected operation to complete successfully.");

  std::cout << "Visited " << hintCount << " hints.\n";
  ::test(hintCount == 7, "Expected to visit 7 hints.");

  return 0;
}
