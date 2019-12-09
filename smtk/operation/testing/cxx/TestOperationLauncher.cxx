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
#include <future>
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

const char myOperationXML[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"2\">"
  "  <Definitions>"
  "    <AttDef Type=\"operation\" Label=\"operation\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"debug level\" Optional=\"True\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"outcome\" Label=\"outcome\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "        </Int>"
  "        <String Name=\"log\" Optional=\"True\" NumberOfRequiredValues=\"0\" Extensible=\"True\">"
  "        </String>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"MyOperation\" Label=\"My Operation\" BaseType=\"operation\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"sleep\" Optional=\"False\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(test op)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

const char* MyOperation::xmlDescription() const
{
  return myOperationXML;
}
}

int TestOperationLauncher(int /*unused*/, char** const /*unused*/)
{
  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register MyOperation
  operationManager->registerOperation<MyOperation>("MyOperation");

  int sleepValue = 3;

  smtk::operation::Operation::Ptr myOperation = operationManager->create<MyOperation>();
  myOperation->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);

  // Use the default operation launcher to launch our operation asynchronously
  std::future<smtk::operation::Operation::Result> result =
    operationManager->launchers()(myOperation);

  std::future_status status;
  int nSeconds = 0;
  do
  {
    std::cout << "polling worker thread" << std::endl;
    status = result.wait_for(std::chrono::seconds(1));
    ++nSeconds;
  } while (status != std::future_status::ready);

  smtk::operation::Operation::Outcome outcome =
    smtk::operation::Operation::Outcome(result.get()->findInt("outcome")->value());

  smtkTest(
    (outcome == smtk::operation::Operation::Outcome::SUCCEEDED), "Operation should succeed.");

  smtkTest((nSeconds >= sleepValue), "Operation should have run for 3 seconds.");

  return 0;
}
