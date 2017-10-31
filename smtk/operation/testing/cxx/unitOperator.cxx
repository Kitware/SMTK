//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <iostream>

#define TEST_OP_NAME "test op"

class TestOp : public smtk::operation::Operator
{
public:
  smtkTypeMacro(TestOp);
  smtkSharedPtrCreateMacro(smtk::operation::Operator);
  smtkDeclareOperator();
  TestOp()
    : m_outcome(OPERATION_SUCCEEDED)
  {
  }
  ~TestOp() override {}

  bool ableToOperate() override { return m_outcome == UNABLE_TO_OPERATE ? false : true; }

  smtk::operation::Operator::Result operateInternal() override
  {
    return this->createResult(static_cast<smtk::operation::Operator::Outcome>(m_outcome));
  }

  smtk::io::Logger& log() override { return m_log; }

  int m_outcome;
  smtk::io::Logger m_log;
};

const char testOpXML[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"2\">"
  "  <Definitions>"
  "    <AttDef Type=\"" TEST_OP_NAME "\" Label=\"A Test Operator\" BaseType=\"operator\">"
  "    </AttDef>"
  "    <AttDef Type=\"result(test op)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

smtkImplementsOperator(, TestOp, test_op, TEST_OP_NAME, testOpXML);

struct
{
  smtk::operation::Operator::EventType event;
  bool haveResult;
  int outcome;
} expectedObservations[] = {
  { smtk::operation::Operator::CREATED_OPERATOR, false, -1 },
  { smtk::operation::Operator::WILL_OPERATE, false, -1 },
  { smtk::operation::Operator::DID_OPERATE, true, 2 },
  { smtk::operation::Operator::WILL_OPERATE, false, -1 },
  { smtk::operation::Operator::DID_OPERATE, true, 1 },
  { smtk::operation::Operator::WILL_OPERATE, false, -1 },
  { smtk::operation::Operator::DID_OPERATE, true, 3 },
};
int obs = 0;
bool haveCanceled = false;

int unitOperator(int, char* [])
{
  smtkComponentInitMacro(smtk_test_op_operator);
  auto manager = smtk::operation::Manager::create();
  std::shared_ptr<TestOp> testOp;

  int handle = manager->observe(
    [&testOp](smtk::operation::Operator::Ptr op, smtk::operation::Operator::EventType event,
      smtk::operation::Operator::Result result) -> int {
      int outcome = -1;
      std::cout << "[" << obs << "] " << op->name() << " event " << event << " result " << result;
      if (result && testOp)
      {
        outcome = result->findInt("outcome")->value();
        std::cout << " outcome " << outcome << " expected " << expectedObservations[obs].outcome;
      }
      std::cout << "\n";

      smtkTest(event == expectedObservations[obs].event, "Unexpected event " << event);
      smtkTest(!!result == expectedObservations[obs].haveResult, "Unexpected result " << result);
      smtkTest(outcome == expectedObservations[obs].outcome, "Unexpected outcome " << outcome);

      ++obs;
      // On the penultimate WILL_OPERATE, return a non-zero value to cancel the operation;
      // otherwise return 0. Note that obs has been incremented already, hence 4 not 3:
      return (obs == 4 ? 1 : 0);
    });

  int another =
    manager->observe([](smtk::operation::Operator::Ptr, smtk::operation::Operator::EventType,
                       smtk::operation::Operator::Result) -> int {
      smtkTest(false, "This observer should never be called");
      return 1;
    });

  smtkTest(handle != another, "Expected one handle " << handle << " and another " << another
                                                     << " to be distinct");
  smtkTest(manager->unobserve(another), "Could not unregister second observer");

  auto baseOp = manager->create<TestOp>();
  testOp = smtk::dynamic_pointer_cast<TestOp>(baseOp);

  testOp->m_outcome = smtk::operation::Operator::OPERATION_FAILED;
  auto result = testOp->operate();

  // This generates no events since ableToOperate() fails
  testOp->m_outcome = smtk::operation::Operator::UNABLE_TO_OPERATE;
  result = testOp->operate();

  testOp->m_outcome = smtk::operation::Operator::OPERATION_SUCCEEDED;
  result = testOp->operate();
  result = testOp->operate();

  smtkTest(manager->unobserve(handle), "Could not remove operation observer (" << handle << ")");

  smtkTest(obs == sizeof(expectedObservations) / sizeof(expectedObservations[0]), "Observed "
      << obs << " events,"
                " expected "
      << (sizeof(expectedObservations) / sizeof(expectedObservations[0])));

  return 0;
}
