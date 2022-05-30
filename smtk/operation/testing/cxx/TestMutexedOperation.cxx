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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

namespace
{
std::atomic<bool> semaphore(false);

class MyResource : public smtk::resource::DerivedFrom<MyResource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(MyResource);
  smtkCreateMacro(MyResource);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  smtk::resource::ComponentPtr find(const smtk::common::UUID& /*compId*/) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& /*v*/) const override {}

protected:
  MyResource() = default;
};

class MyComponent : public smtk::resource::Component
{
public:
  smtkTypeMacro(MyComponent);
  smtkCreateMacro(MyComponent);
  smtkSharedFromThisMacro(smtk::resource::Component);

  const smtk::common::UUID& id() const override { return myId; }
  bool setId(const smtk::common::UUID& anId) override
  {
    myId = anId;
    return true;
  }

  const smtk::resource::ResourcePtr resource() const override { return myResource; }
  void setResource(MyResource::Ptr& r) { myResource = r; }

private:
  MyResource::Ptr myResource;
  smtk::common::UUID myId;
};

class ReadOperation : public smtk::operation::Operation
{
public:
  smtkTypeMacro(ReadOperation);
  smtkCreateMacro(ReadOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  ReadOperation() = default;
  ~ReadOperation() override = default;

  smtk::io::Logger& log() const override { return m_logger; }

  Result operateInternal() override;

  Specification createSpecification() override;

private:
  mutable smtk::io::Logger m_logger;
};

ReadOperation::Result ReadOperation::operateInternal()
{
  bool semaphoreValue =
    (this->parameters()->findAs<smtk::attribute::IntItem>("semaphore")->value() != 0);

  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();

  int timeout = 0;
  while (semaphore != semaphoreValue)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (++timeout > sleep)
    {
      return this->createResult(Outcome::FAILED);
    }
  }

  while (semaphore.compare_exchange_weak(semaphoreValue, !semaphoreValue))
  {
  }
  return this->createResult(Outcome::SUCCEEDED);
}

ReadOperation::Specification ReadOperation::createSpecification()
{
  Specification spec = smtk::attribute::Resource::create();

  smtk::attribute::DefinitionPtr opDef = spec->createDefinition("operation");
  opDef->setLabel("operation");
  opDef->setIsAbstract(true);

  smtk::attribute::IntItemDefinitionPtr debugDef =
    smtk::attribute::IntItemDefinition::New("debug level");
  debugDef->setIsOptional(true);
  debugDef->setDefaultValue(0);
  opDef->addItemDefinition(debugDef);

  smtk::attribute::DefinitionPtr resDef = spec->createDefinition("result");
  resDef->setIsAbstract(true);

  smtk::attribute::IntItemDefinitionPtr outcomeDef =
    smtk::attribute::IntItemDefinition::New("outcome");
  outcomeDef->setIsOptional(false);
  outcomeDef->setLabel("outcome");
  outcomeDef->setNumberOfRequiredValues(1);
  resDef->addItemDefinition(outcomeDef);

  smtk::attribute::StringItemDefinitionPtr logDef =
    smtk::attribute::StringItemDefinition::New("log");
  logDef->setIsOptional(true);
  logDef->setLabel("log");
  logDef->setNumberOfRequiredValues(0);
  logDef->setIsExtensible(true);
  resDef->addItemDefinition(logDef);

  smtk::attribute::DefinitionPtr readOpDef = spec->createDefinition("ReadOperation", "operation");
  readOpDef->setLabel("read");

  smtk::attribute::IntItemDefinitionPtr sleepDef = smtk::attribute::IntItemDefinition::New("sleep");
  sleepDef->setDefaultValue(0);
  readOpDef->addItemDefinition(sleepDef);

  smtk::attribute::IntItemDefinitionPtr semaphoreDef =
    smtk::attribute::IntItemDefinition::New("semaphore");
  semaphoreDef->setDefaultValue(0);
  readOpDef->addItemDefinition(semaphoreDef);

  smtk::attribute::ComponentItemDefinitionPtr compDef =
    smtk::attribute::ComponentItemDefinition::New("component");
  compDef->setNumberOfRequiredValues(1);
  compDef->setIsOptional(false);
  compDef->setLockType(smtk::resource::LockType::Read);
  readOpDef->addItemDefinition(compDef);

  smtk::attribute::DefinitionPtr readResDef = spec->createDefinition("read result", "result");
  readOpDef->setLabel("read result");

  return spec;
}

class WriteOperation : public smtk::operation::Operation
{
public:
  smtkTypeMacro(WriteOperation);
  smtkCreateMacro(WriteOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  WriteOperation() = default;
  ~WriteOperation() override = default;

  smtk::io::Logger& log() const override { return m_logger; }

  Result operateInternal() override;

  Specification createSpecification() override;

private:
  mutable smtk::io::Logger m_logger;
};

WriteOperation::Result WriteOperation::operateInternal()
{
  if (semaphore)
  {
    return this->createResult(Outcome::FAILED);
  }
  semaphore = true;
  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep * 100));
  semaphore = false;
  return this->createResult(Outcome::SUCCEEDED);
}

WriteOperation::Specification WriteOperation::createSpecification()
{
  Specification spec = smtk::attribute::Resource::create();

  smtk::attribute::DefinitionPtr opDef = spec->createDefinition("operation");
  opDef->setLabel("operation");
  opDef->setIsAbstract(true);

  smtk::attribute::IntItemDefinitionPtr debugDef =
    smtk::attribute::IntItemDefinition::New("debug level");
  debugDef->setIsOptional(true);
  debugDef->setDefaultValue(0);
  opDef->addItemDefinition(debugDef);

  smtk::attribute::DefinitionPtr resDef = spec->createDefinition("result");
  resDef->setIsAbstract(true);

  smtk::attribute::IntItemDefinitionPtr outcomeDef =
    smtk::attribute::IntItemDefinition::New("outcome");
  outcomeDef->setIsOptional(false);
  outcomeDef->setLabel("outcome");
  outcomeDef->setNumberOfRequiredValues(1);
  resDef->addItemDefinition(outcomeDef);

  smtk::attribute::StringItemDefinitionPtr logDef =
    smtk::attribute::StringItemDefinition::New("log");
  logDef->setIsOptional(true);
  logDef->setLabel("log");
  logDef->setNumberOfRequiredValues(0);
  logDef->setIsExtensible(true);
  resDef->addItemDefinition(logDef);

  smtk::attribute::DefinitionPtr readOpDef = spec->createDefinition("WriteOperation", "operation");
  readOpDef->setLabel("read");

  smtk::attribute::IntItemDefinitionPtr sleepDef = smtk::attribute::IntItemDefinition::New("sleep");
  sleepDef->setDefaultValue(0);
  readOpDef->addItemDefinition(sleepDef);

  smtk::attribute::ComponentItemDefinitionPtr compDef =
    smtk::attribute::ComponentItemDefinition::New("component");
  compDef->setNumberOfRequiredValues(1);
  compDef->setIsOptional(false);
  compDef->setLockType(smtk::resource::LockType::Write);
  readOpDef->addItemDefinition(compDef);

  smtk::attribute::DefinitionPtr readResDef = spec->createDefinition("read result", "result");
  readOpDef->setLabel("read result");

  return spec;
}
} // namespace

int readTest(int sleepValue)
{
  auto resource = MyResource::create();
  auto component = MyComponent::create();
  component->setResource(resource);

  std::cout << "Read test" << std::endl;

  smtk::operation::Operation::Ptr readOperation1 = ReadOperation::create();
  readOperation1->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);
  readOperation1->parameters()->findAs<smtk::attribute::IntItem>("semaphore")->setValue(0);
  readOperation1->parameters()
    ->findAs<smtk::attribute::ComponentItem>("component")
    ->setValue(component);

  smtk::operation::Operation::Ptr readOperation2 = ReadOperation::create();
  readOperation2->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);
  readOperation2->parameters()->findAs<smtk::attribute::IntItem>("semaphore")->setValue(1);
  readOperation2->parameters()
    ->findAs<smtk::attribute::ComponentItem>("component")
    ->setValue(component);

  std::future<smtk::operation::Operation::Result> result1(
    std::async(std::launch::async, [&]() { return readOperation1->operate(); }));
  std::future<smtk::operation::Operation::Result> result2(
    std::async(std::launch::async, [&]() { return readOperation2->operate(); }));

  std::future_status status1 = std::future_status::timeout;
  std::future_status status2 = std::future_status::timeout;
  int nMilliseconds = 0;

  while (status1 != std::future_status::ready)
  {
    status1 = result1.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  while (status2 != std::future_status::ready)
  {
    status2 = result2.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  smtk::operation::Operation::Outcome outcome1 =
    smtk::operation::Operation::Outcome(result1.get()->findInt("outcome")->value());

  smtk::operation::Operation::Outcome outcome2 =
    smtk::operation::Operation::Outcome(result2.get()->findInt("outcome")->value());

  if (
    outcome1 != smtk::operation::Operation::Outcome::SUCCEEDED ||
    outcome2 != smtk::operation::Operation::Outcome::SUCCEEDED)
  {
    return 1;
  }

  return 0;
}

int writeTest(int sleepValue)
{
  auto resource = MyResource::create();
  auto component = MyComponent::create();
  component->setResource(resource);

  std::cout << "Write test" << std::endl;

  smtk::operation::Operation::Ptr writeOperation1 = WriteOperation::create();
  writeOperation1->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);
  writeOperation1->parameters()
    ->findAs<smtk::attribute::ComponentItem>("component")
    ->setValue(component);

  smtk::operation::Operation::Ptr writeOperation2 = WriteOperation::create();
  writeOperation2->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);
  writeOperation2->parameters()
    ->findAs<smtk::attribute::ComponentItem>("component")
    ->setValue(component);

  std::future<smtk::operation::Operation::Result> result3(
    std::async(std::launch::async, [&]() { return writeOperation1->operate(); }));
  std::future<smtk::operation::Operation::Result> result4(
    std::async(std::launch::async, [&]() { return writeOperation2->operate(); }));

  std::future_status status3 = std::future_status::timeout;
  std::future_status status4 = std::future_status::timeout;
  int nMilliseconds = 0;

  while (status3 != std::future_status::ready)
  {
    status3 = result3.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  while (status4 != std::future_status::ready)
  {
    status4 = result4.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  smtk::operation::Operation::Outcome outcome3 =
    smtk::operation::Operation::Outcome(result3.get()->findInt("outcome")->value());

  smtk::operation::Operation::Outcome outcome4 =
    smtk::operation::Operation::Outcome(result4.get()->findInt("outcome")->value());

  smtkTest(
    (outcome3 == smtk::operation::Operation::Outcome::SUCCEEDED), "Operation 3 should succeed.");
  smtkTest(
    (outcome4 == smtk::operation::Operation::Outcome::SUCCEEDED), "Operation 4 should succeed.");

  return 0;
}

// Test mutexed operations by executing two parallel read operations and two
// parallel write operations. Each read operation waits for a global semaphore
// to hold its value, failing after a timeout period, and then switches the
// semaphore's value. When two of these operations are executed within a time
// window less than the timeout period, both operations should pass. Each write
// operation fails if the semaphore is false, then toggles it to true, waits for
// a time, and toggles it back to false. If two of these operations are executed
// within a time window less than the wait period, both tests should pass.
int TestMutexedOperation(int /*unused*/, char** const /*unused*/)
{
  int returnValue = 1;

  int sleepValue = 4;
  for (int i = 1; i < 4; i++)
  {
    returnValue = readTest(i * 4);
    if (returnValue == 0)
    {
      break;
    }
  }

  smtkTest(returnValue == 0, "Mutexed read test failed.");

  return writeTest(sleepValue);
}
