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

#include "smtk/operation/NewOp.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/XMLOperator.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <chrono>
#include <future>
#include <thread>

namespace
{
class MyOperator : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(MyOperator);
  smtkCreateMacro(MyOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

  MyOperator() {}
  ~MyOperator() override {}

  Result operateInternal() override;

  const char* xmlDescription() const override;
};

MyOperator::Result MyOperator::operateInternal()
{
  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();
  std::this_thread::sleep_for(std::chrono::seconds(sleep));

  return this->createResult(Outcome::SUCCEEDED);
}

const char myOperatorXML[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"2\">"
  "  <Definitions>"
  "    <AttDef Type=\"operator\" Label=\"operator\" Abstract=\"True\">"
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
  "    <AttDef Type=\"MyOperator\" Label=\"My Operator\" BaseType=\"operator\">"
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

const char* MyOperator::xmlDescription() const
{
  return myOperatorXML;
}
}

int TestAsyncOperator(int, char** const)
{
  int sleepValue = 3;

  smtk::operation::NewOp::Ptr myOperator = MyOperator::create();
  myOperator->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);

  std::future<smtk::operation::NewOp::Result> result(
    std::async(std::launch::async, [&]() { return myOperator->operate(); }));

  std::future_status status;
  int nSeconds = 0;
  do
  {
    std::cout << "polling worker thread" << std::endl;
    status = result.wait_for(std::chrono::seconds(1));
    ++nSeconds;
  } while (status != std::future_status::ready);

  smtk::operation::NewOp::Outcome outcome =
    smtk::operation::NewOp::Outcome(result.get()->findInt("outcome")->value());

  smtkTest((outcome == smtk::operation::NewOp::Outcome::SUCCEEDED), "Operation should succeed.");

  smtkTest((nSeconds >= sleepValue), "Operation should have run for 3 seconds.");

  return 0;
}
