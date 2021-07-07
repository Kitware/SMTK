//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/Manager.h"
#include "smtk/view/OperationIcons.h"
#include "smtk/view/Registrar.h"

#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/operators/Associate.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"

#include "smtk/plugin/Registry.h"

#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

int unitOperationIcon(int, char*[])
{
  // Setup: create some managers, register operations, register icons:
  smtk::io::Logger::instance().setFlushToStdout(true);
  auto viewManager = smtk::view::Manager::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  auto attributeRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);
  auto viewRegistry = smtk::plugin::addToManagers<smtk::view::Registrar>(viewManager);
  std::string color = "dark";

  // Create some "icon constructor" functors that really just increment counters:
  int genericCount = 0;
  int signalCount = 0;
  viewManager->operationIcons().registerDefaultIconConstructor([&genericCount](const std::string&) {
    ++genericCount;
    std::string result = "generic operation";
    return result;
  });
  viewManager->operationIcons().registerOperation<smtk::attribute::Signal>(
    [&signalCount](const std::string&) {
      ++signalCount;
      std::string result = "signal";
      return result;
    });

  // See which icon constructors get called when we ask for icons.
  for (const auto& op : operationManager->metadata().get<smtk::operation::NameTag>())
  {
    auto iconFromName = viewManager->operationIcons().createIcon(op.typeName(), color);
    auto iconFromIndex = viewManager->operationIcons().createIcon(op.index(), color);
    std::cout << "Op " << op.typeName() << ": " << iconFromIndex << " == " << iconFromName << "\n";
    smtkTest(iconFromName == iconFromIndex, "Icons for same operation did not match.");
  }
  smtkTest(signalCount == 2, "Did not use custom icon for Signal.");
  smtkTest(genericCount > 1, "Did not use default icon.");

  viewManager->operationIcons().createIcon<smtk::attribute::Signal>(color);
  smtkTest(signalCount == 3, "Failed to create icon with templated method.");

  genericCount = 0;
  viewManager->operationIcons().createIcon<smtk::attribute::Associate>(color);
  smtkTest(genericCount == 1, "Did not use default icon for Associate.");

  // Test that unregistration compiles and reports itself as working.
  smtkTest(
    viewManager->operationIcons().unregisterOperation<smtk::attribute::Signal>(),
    "Could not unregister Signal icon.");
  smtkTest(
    !viewManager->operationIcons().unregisterOperation<smtk::attribute::Associate>(),
    "Could unregister Associate icon.");

  // Verify that unregistration did in fact unregister.
  signalCount = 0;
  genericCount = 0;
  viewManager->operationIcons().createIcon<smtk::attribute::Signal>(color);
  smtkTest(genericCount == 1 && signalCount == 0, "Unregistering icon had no effect.");

  return 0;
}
