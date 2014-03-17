#include "smtk/model/BridgeBase.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"
#include "smtk/model/OperatorResult.h"
#include "smtk/model/Parameter.h"
#include "smtk/model/Storage.h"

#include "smtk/util/Testing/helpers.h"
#include "smtk/model/testing/helpers.h"

using namespace smtk::model;

template<typename T>
void printVec(const std::vector<T>& v, const char* typeName, char sep = '\0')
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

void printParam(const Parameter& param)
{
  std::cout << (param.name().empty() ? "(null)" : param.name()) << ":";
  printVec(param.integerValues(), "i");
  printVec(param.floatValues(), "f");
  printVec(param.stringValues(), "s");
  printVec(param.uuidValues(), "u");
}

int ParameterWatcher(OperatorEventType event, const Operator& op, const Parameter& oldParam, const Parameter& newParam, void* user)
{
  test(event == PARAMETER_CHANGE, "ParameterChange callback invoked with bad event type");
  std::cout << "Operator " << op.name() << " parameter change  old: ";
  printParam(oldParam);
  std::cout << "  new: ";
  printParam(newParam);
  std::cout << "\n";
  // increment the number of times the parameter was modified.
  (*reinterpret_cast<int*>(user))++;
  return 0;
}

int WillOperateWatcher(OperatorEventType event, const Operator& op, void* user)
{
  test(event == WILL_OPERATE, "WillOperate callback invoked with bad event type");
  int shouldCancel = *reinterpret_cast<int*>(user);
  std::cout << "Operator " << op.name() << " about to run: " << (shouldCancel ? "will" : "will not") << " request cancellation.\n";
  return shouldCancel;
}

int DidOperateWatcher(OperatorEventType event, const Operator& op, const OperatorResult& result, void* user)
{
  test(event == DID_OPERATE, "DidOperate callback invoked with bad event type");
  std::cout << "Operator " << op.name() << " did operate (outcome " << result.outcome() << ")\n";
  // increment the number of times the parameter was modified.
  (*reinterpret_cast<int*>(user))++;
  return 0;
}

class TestOutcomeOperator : public Operator
{
public:
  smtkTypeMacro(TestOutcomeOperator);
  smtkCreateMacro(TestOutcomeOperator);
  smtkSharedFromThisMacro(Operator);

  TestOutcomeOperator()
    : m_able(false) // fail operation until told otherwise
    {
    Parameter p0("shouldSucceed", Integer(0));
    this->setParameter(p0);
    }
  virtual std::string name() const { return "outcome test"; }
  virtual bool ableToOperate()
    {
    Parameters params = this->parameters();
    return
      // Force failure?
      this->m_able &&
      // Is "shouldSucceed" parameter present with at least 1 integer value?
      this->hasIntegerParameter("shouldSucceed")
      ;
    }
  bool m_able; // Used to force UNABLE_TO_OPERATE result.
  virtual OperatorPtr clone() const
    { return create()->cloneInternal(shared_from_this()); }

protected:
  virtual OperatorResult operateInternal()
    {
    return
      this->parameter("shouldSucceed").integerValues()[0] ?
      OperatorResult(OPERATION_SUCCEEDED) :
      OperatorResult(OPERATION_FAILED);
    }
};

void testOperatorOutcomes(Storage::Ptr storage)
{
  TestOutcomeOperator::Ptr op = TestOutcomeOperator::create();

  // Add the operator to the default bridge of our storage.
  storage->bridgeForModel(smtk::util::UUID::null())->addOperator(op);

  int shouldCancel = 1;
  int parameterWasModified = 0;
  int numberOfFailedOperations = 0;
  op->observe(PARAMETER_CHANGE, ParameterWatcher, &parameterWasModified);
  op->observe(WILL_OPERATE, WillOperateWatcher, &shouldCancel);
  op->observe(DID_OPERATE, DidOperateWatcher, &numberOfFailedOperations);

  // Verify that ableToOperate() is called and complains:
  test(op->operate().outcome() == UNABLE_TO_OPERATE, "Should have been unable to operate.");
  std::cout << "Operator " << op->name() << " unable to operate (outcome " << op->operate().outcome() << ").\n--\n";

  // Verify that the WILL_OPERATE observer is called and cancels the operation:
  op->m_able = true;
  test(op->operate().outcome() == OPERATION_CANCELED, "Operation should have been canceled.");
  std::cout << "--\n";

  // Verify that the operation fails when "shouldSucceed" parameter is zero.
  shouldCancel = 0;
  test(op->operate().outcome() == OPERATION_FAILED, "Operation should have failed.");
  std::cout << "--\n";

  // Verify that removing observer bypasses cancellation.
  op->unobserve(WILL_OPERATE, WillOperateWatcher, &shouldCancel);
  shouldCancel = 1; // Force cancellation if observer wasn't removed.

  // Verify that the parameter-change callback is invoked.
  op->setParameter(Parameter("shouldSucceed", Integer(1)));
  test(parameterWasModified, "Parameter-change callback never invoked.");

  // Verify that the operation succeeds when "shouldSucceed" parameter is non-zero.
  test(op->operate().outcome() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
  std::cout << "--\n";

  // Now test values passed to callbacks:
  test(numberOfFailedOperations > 0, "Expected more operator failures.");

  op->unobserve(DID_OPERATE, DidOperateWatcher, &numberOfFailedOperations);
  op->unobserve(PARAMETER_CHANGE, ParameterWatcher, &parameterWasModified);
}

class TestParameterOperator : public Operator
{
public:
  smtkTypeMacro(TestParameterOperator);
  smtkCreateMacro(TestParameterOperator);
  smtkSharedFromThisMacro(Operator);

  TestParameterOperator()
    {
    Parameter p0("shouldSucceed");
    p0.setIntegerValue(1);
    p0.setStringValue("zero");
    p0.setUUIDValue(smtk::util::UUID::random());
    FloatList fvec(3);
    fvec[0] = 0.; fvec[1] = 1.; fvec[2] = 2.;
    p0.setFloatValue(fvec);
    this->setParameter(p0);
    }
  virtual std::string name() const { return "parameter test"; }
  virtual bool ableToOperate()
    {
    return
      // Is "shouldSucceed" parameter present for each primitive type?
      this->hasFloatParameter("shouldSucceed", 3, 3) &&
      this->hasStringParameter("shouldSucceed", 1, 2) &&
      this->hasIntegerParameter("shouldSucceed", 1, 1) &&
      this->hasUUIDParameter("shouldSucceed", 1, -1)
      ;
    }
  virtual OperatorPtr clone() const
    { return create()->cloneInternal(shared_from_this()); }

protected:
  virtual OperatorResult operateInternal()
    {
    return
      this->parameter("shouldSucceed").integerValues()[0] ?
      OperatorResult(OPERATION_SUCCEEDED) :
      OperatorResult(OPERATION_FAILED);
    }
};

void testParameterChecks(Storage::Ptr storage)
{
  TestParameterOperator::Ptr op = TestParameterOperator::create();

  // Add the operator to the default bridge of our storage.
  storage->bridgeForModel(smtk::util::UUID::null())->addOperator(op);

  int parameterModifiedCount = 0;
  op->observe(PARAMETER_CHANGE, ParameterWatcher, &parameterModifiedCount);

  std::cout << "Operator " << op->name() << " Initial parameter ";
  printParam(op->parameter("shouldSucceed")); std::cout << "\n";
  test(op->operate().outcome() == OPERATION_SUCCEEDED, "Operation should have succeeded.");

  // Verify that extraneous parameters don't cause trouble
  Parameter extra("extraneousParameter");
  extra.setFloatValue(FloatList(2, 2.7));
  extra.setStringValue(StringList(1, "huh?"));
  extra.setIntegerValue(IntegerList(3, 0));
  extra.setUUIDValue(smtk::util::UUIDArray(2, smtk::util::UUID::random()));
  extra.setUUIDValue(smtk::util::UUID::null());
  op->setParameter(extra);
  test(op->operate().outcome() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
  test(op->parameter("shouldSucceed").validState() == PARAMETER_VALIDATED, "Required parameter was not valid.");

  // Verify that size checks are performed on parameters:
  op->setParameter(Parameter("shouldSucceed", FloatList(4, 1.4)));
  test(op->operate().outcome() == UNABLE_TO_OPERATE, "Should have been unable to operate.");

  test(parameterModifiedCount > 0, "Parameter-change callback never invoked.");
  test(op->parameter("extraneousParameter").validState() == PARAMETER_UNKNOWN, "Extraneous parameter was validated?");
  test(op->parameter("shouldSucceed").validState() == PARAMETER_INVALID, "Required parameter was invalid but marked valid.");
  test(op->parameter("nonExistent").validState() == PARAMETER_UNKNOWN, "Missing parameter was validated?");
  std::cout << "--\n";
}

void testBridgeAssociation(Storage::Ptr storage)
{
  // Test that operators added by previous tests still exist
  ModelEntity model = storage->addModel(3, 3, "Model Airplane");
  test(model.operators().size() == 2, "Expected 2 operators defined for the test model.");

  // Test op(name) method
  Operator::Ptr op = model.op("parameter test");
  test(op ? 1 : 0, "ModelEntity::op(\"ParameterTest\") returned a \"null\" shared pointer.");

  // Test Operator->Bridge association
  test(op->bridge() == storage->bridgeForModel(smtk::util::UUID::null()),
    "Bad bridge reported by operator.");

  // Test Operator->Storage association
  test(op->storage() == storage, "Bad storage reported by operator.");

  // Test operatorNames()
  StringList opNames = model.bridge()->operatorNames();
  printVec(opNames, "operator names", ',');
  std::cout << "\n";
}

int main()
{
  int status = 0;

  Storage::Ptr storage = Storage::create();

  try {

    testOperatorOutcomes(storage);
    testParameterChecks(storage);
    testBridgeAssociation(storage);

  } catch (const std::string& msg) {
    std::cerr << "Exiting...\n";
    status = -1;
  }

  return status;
}
