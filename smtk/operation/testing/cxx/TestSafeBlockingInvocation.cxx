//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/operation/Launcher.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <chrono>
#include <thread>

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

  const char* xmlDescription() const override;
};

MyOperation::Result MyOperation::operateInternal()
{
  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();
  std::this_thread::sleep_for(std::chrono::seconds(sleep));

  return this->createResult(Outcome::SUCCEEDED);
}

const char operationXML[] = R"xml(
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <AttDef Type="operation" Label="operation" Abstract="True">
      <ItemDefinitions>
        <Int Name="debug level" Optional="True">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="result" Abstract="True">
      <ItemDefinitions>
        <Int Name="outcome" Label="outcome" Optional="False" NumberOfRequiredValues="1">
        </Int>
        <String Name="log" Optional="True" NumberOfRequiredValues="0" Extensible="True">
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="MyOperation" Label="My Operation" BaseType="operation">
      <ItemDefinitions>
        <Int Name="sleep" Optional="False">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="result(test op)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>)xml";

const char* MyOperation::xmlDescription() const
{
  return operationXML;
}
} // namespace

// Parameters are constructed lazily, allowing for RAII while having derived
// classes construct parameters that are tailored to their use. This can
// cause a race condition when observers that are called on a different
// thread access parameters at the same time as the thread that created the
// operation. Since only managed operations are observed, we can avoid this
// issue by accessing the parameters as they are created by the manager. This
// test confirms that the abovementioned fix to smtk::operation::Manager solves
// the race condition.
//
// An observer is added to an operation manager that launches a thread that
// waits an amount of time and then accesses the operation's parameters.
// Meanwhile, the thread that creates the operation access the operation's
// parameters. By varying the wait time of the observing thread (which is
// called before the operation's create() method returns), we scan through
// timing intervals to ensure that both the observing thread and the main thread
// access the parameters object at the same time +/- 1 microsecond. We then test
// to ensure that the objects are the same.
int TestSafeBlockingInvocation(int /*unused*/, char** const /*unused*/)
{
  int ntest = 50;
  static std::atomic<int> handlerCount(0);
  for (int i = 0; i < ntest; ++i)
  {
    // Create an operation manager
    smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

    // Add an observer that accesses the operation's parameters from a separate
    // thread.
    smtk::operation::Operation::Parameters parametersFromObserver = nullptr;
    operationManager->observers()
      .insert(
        [&](
          const smtk::operation::Operation& op,
          smtk::operation::EventType eventType,
          smtk::operation::Operation::Result /*unused*/) -> int {
          if (eventType == smtk::operation::EventType::WILL_OPERATE)
          {
            std::thread t([&]() {
              std::this_thread::sleep_for(std::chrono::microseconds(i));
              parametersFromObserver = op.parameters();
            });
            t.detach();
          }
          return 0;
        })
      .release();

    // Register MyOperation
    operationManager->registerOperation<MyOperation>("MyOperation");

    // Construct an instance of MyOperation
    smtk::operation::Operation::Ptr myOperation = operationManager->create<MyOperation>();
    auto outcome = myOperation->safeOperate(
      [&](smtk::operation::Operation& op, smtk::operation::Operation::Result /*result*/) {
        std::cout << "  Operation " << &op << " handler " << handlerCount << "\n";
        ++handlerCount;
      });
    std::cout << "    Operation " << myOperation.get() << " outcome " << static_cast<int>(outcome)
              << "\n";
    ::test(
      outcome == smtk::operation::Operation::Outcome::SUCCEEDED,
      "Expected operation to complete successfully.");

    // Immediately access its parameters
    auto parametersFromMainThread = myOperation->parameters();

    // Wait for the observer to finish assign its parameters
    int j = 0;
    while (parametersFromObserver == nullptr)
    {
      std::this_thread::sleep_for(std::chrono::microseconds(++j));
    }

    // Confirm that the parameters instance from both threads are the same
    smtkTest(
      (parametersFromObserver == parametersFromMainThread), "Parameters should be the same.");
  }
  ::test(handlerCount == 50, "Expected handler to run 50 times (0–49).");

  return 0;
}
