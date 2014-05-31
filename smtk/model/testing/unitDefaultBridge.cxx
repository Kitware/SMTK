#include "smtk/util/AutoInit.h"

#include "smtk/model/Bridge.h"
#include "smtk/model/DefaultBridge.h"
#include "smtk/model/ExportJSON.h"
#include "smtk/model/ImportJSON.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"
#include "smtk/model/OperatorResult.h"
#include "smtk/model/Parameter.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/Manager.h"

#include "smtk/util/Testing/helpers.h"
#include "smtk/model/testing/helpers.h"

#include "smtk/options.h"

#include "cJSON.h"

using namespace smtk::model;

template<typename T>
static void printVec(const std::vector<T>& v, const char* typeName, char sep = '\0')
{
  if (v.empty()) return;
  std::cout << " " << typeName << "(" << v.size() << "): [";
  std::cout << " " << v[0];
  if (sep)
    for (typename std::vector<T>::size_type i = 1; i < v.size(); ++i)
      std::cout << sep << " " << v[i];
  else
    for (typename std::vector<T>::size_type i = 1; i < v.size(); ++i)
      std::cout << " " << v[i];
  std::cout << " ]";
}

static void printParam(const Parameter& param)
{
  std::cout << (param.name().empty() ? "(null)" : param.name()) << ":";
  printVec(param.integerValues(), "i");
  printVec(param.floatValues(), "f");
  printVec(param.stringValues(), "s");
  printVec(param.uuidValues(), "u");
}

static void printParams(const Parameters& params, const std::string& msg)
{
  std::cout << msg << "\n";
  Parameters::const_iterator it;
  for (it = params.begin(); it != params.end(); ++it)
    {
    std::cout << "  ";
    printParam(*it);
    std::cout << "\n";
    }
}

class TestForwardingOperator : public Operator
{
public:
  smtkTypeMacro(TestForwardingOperator);
  smtkCreateMacro(TestForwardingOperator);
  smtkSharedFromThisMacro(Operator);

  TestForwardingOperator()
    {
    this->setParameter(Parameter("addToCount", Integer(0)));
    }
  virtual std::string name() const { return "forwarding operator"; }
  virtual bool ableToOperate()
    {
    return this->hasIntegerParameter("addToCount", 1, 1, true);
    }
  virtual OperatorPtr clone() const
    { return create()->cloneInternal(shared_from_this()); }

protected:
  virtual OperatorResult operateInternal()
    {
    printParams(this->parameters(), "actual input");
    this->s_state +=
      this->parameter("addToCount").integerValues()[0];
    OperatorResult result(OPERATION_SUCCEEDED);
    result.setParameter(Parameter("state",this->s_state));
    printParams(result.parameters(), "actual output");
    return result;
    }

  // Note that any state to be preserved between
  // invocations of the operator must be kept in
  // smtk::model::Manager or class static. This is
  // because the operator is cloned from a master
  // each time it is requested and the clone is
  // not given a chance to update the master held
  // by the Bridge after it has executed.
  static Integer s_state;
};

Integer TestForwardingOperator::s_state = 1;

class TestForwardingBridge : public smtk::model::DefaultBridge
{
public:
  smtkTypeMacro(TestForwardingBridge);
  smtkCreateMacro(TestForwardingBridge);
  smtkSharedFromThisMacro(Bridge);
  //smtkDeclareModelingKernel();

  smtk::model::Manager::Ptr remoteModel;
  smtk::model::Bridge::Ptr remoteBridge;

protected:
  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags)
    {
    return remoteBridge->transcribe(Cursor(remoteModel, entity.entity()), flags);
    }
  virtual OperatorResult ableToOperateDelegate(RemoteOperatorPtr oper)
    {
    OperatorPtr remOp = remoteBridge->op(oper->name(), remoteModel);
    remOp->setParameters(oper->parameters());
    OperatorResult result(remOp->ableToOperate() ? OPERATION_SUCCEEDED : OPERATION_FAILED);
    result.setParameters(remOp->parameters());
    return result;
    }
  virtual OperatorResult operateDelegate(RemoteOperatorPtr oper)
    {
    printParams(oper->parameters(), "local input");
    OperatorPtr remOp = remoteBridge->op(oper->name(), remoteModel);
    remOp->setParameters(oper->parameters());
    OperatorResult result = remOp->operate();
    printParams(result.parameters(), "local output");
    return result;
    }
};

// Test remote bridging: create 2 model::Manager instances,
// add a native operator to manager A's "native" bridge,
// serialize the bridge session into a DefaultBridge instance
// that backs it into manager B, and invoke the remote version
// of the operator attached to the DefaultBridge on manager B.
// Check that the operation was invoked on manager A and that
// the OperatorResult from both operations are identical (by
// having the native operator cache its result parameters).
int main()
{
  int status = 0;

  try {

    // Create the managers
    Manager::Ptr remoteMgr = Manager::create();
    Manager::Ptr localMgr = Manager::create();

    // Create the native operator
    TestForwardingOperator::Ptr nativeOp = TestForwardingOperator::create();
    // Add the operator to the default bridge of the "remote" manager.
    Bridge::Ptr remoteBridge = remoteMgr->bridgeForModel(smtk::util::UUID::null());
    remoteBridge->addOperator(nativeOp);

    // Now we want to mirror the remote manager locally.
    // Serialize the "remote" bridge session:
    cJSON* sessJSON = cJSON_CreateObject();
    ExportJSON::forManagerBridgeSession(remoteBridge->sessionId(), sessJSON, remoteMgr);
    // ... and import the session locally to a new bridge object.
    TestForwardingBridge::Ptr localBridge = TestForwardingBridge::create();
    localBridge->remoteBridge = remoteBridge;
    ImportJSON::ofRemoteBridgeSession(sessJSON->child, localBridge, localMgr);

    // Run the local operator.
    // Examine the remote version to verify the operation was forwarded.
    OperatorPtr localOp = localBridge->op("forwarding operator", localMgr);
    localOp->setParameter("addToCount", Integer(2));
    OperatorResult localResult = localOp->operate();

    test(localResult.outcome() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
    test(localResult.parameter("state").integerValues()[0] == 3, "Operation should have yielded state == 3.");

    std::cout << "\n\n\n";

    // Rerun the local operator.
    localResult = localOp->setParameter("addToCount", Integer(8))->operate();

    test(localResult.outcome() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
    test(localResult.parameter("state").integerValues()[0] == 11, "Operation should have yielded state == 11.");

    // Rerun the local operator but with an improper input (lacking a required parameter)
    localOp->removeParameter("addToCount");
    localResult = localOp->operate();

    test(localResult.outcome() == UNABLE_TO_OPERATE, "Operation should have been unable to execute.");

  } catch (const std::string& msg) {
    (void) msg; // Ignore the message; it's already been printed.
    std::cerr << "Exiting...\n";
    status = -1;
  }

  return status;
}
