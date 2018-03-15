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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <chrono>
#include <future>
#include <thread>

namespace
{
class MyResource : public smtk::resource::Resource
{
public:
  smtkTypeMacro(MyResource);
  smtkCreateMacro(MyResource);
  smtkSharedFromThisMacro(smtk::resource::Resource);
  smtkResourceTypeNameMacro("MyResource");

  smtk::resource::ComponentPtr find(const smtk::common::UUID&) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::ComponentPtr&)> queryOperation(
    const std::string&) const override
  {
    return [](const smtk::resource::ComponentPtr&) { return true; };
  }

  void visit(smtk::resource::Component::Visitor&) const override {}

protected:
  MyResource()
    : Resource()
  {
  }
};

class MyComponent : public smtk::resource::Component
{
public:
  smtkTypeMacro(MyComponent);
  smtkCreateMacro(MyComponent);
  smtkSharedFromThisMacro(smtk::resource::Component);

  smtk::common::UUID id() const override { return myId; }
  void setId(const smtk::common::UUID& anId) override { myId = anId; }

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

  ReadOperation() {}
  ~ReadOperation() override {}

  smtk::io::Logger& log() const override { return m_logger; }

  Result operateInternal() override;

  virtual Specification createSpecification() override;

private:
  mutable smtk::io::Logger m_logger;
};

ReadOperation::Result ReadOperation::operateInternal()
{
  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep * 100));

  return this->createResult(Outcome::SUCCEEDED);
}

ReadOperation::Specification ReadOperation::createSpecification()
{
  Specification spec = smtk::attribute::Collection::create();

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

  WriteOperation() {}
  ~WriteOperation() override {}

  smtk::io::Logger& log() const override { return m_logger; }

  Result operateInternal() override;

  virtual Specification createSpecification() override;

private:
  mutable smtk::io::Logger m_logger;
};

WriteOperation::Result WriteOperation::operateInternal()
{
  int sleep = this->parameters()->findAs<smtk::attribute::IntItem>("sleep")->value();
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep * 100));

  return this->createResult(Outcome::SUCCEEDED);
}

WriteOperation::Specification WriteOperation::createSpecification()
{
  Specification spec = smtk::attribute::Collection::create();

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
}

int TestMutexedOperation(int, char** const)
{
  auto resource = MyResource::create();
  auto component = MyComponent::create();
  component->setResource(resource);

  int sleepValue = 4;

  std::cout << "Read test" << std::endl;

  smtk::operation::Operation::Ptr readOperation1 = ReadOperation::create();
  readOperation1->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);
  readOperation1->parameters()
    ->findAs<smtk::attribute::ComponentItem>("component")
    ->setValue(component);

  smtk::operation::Operation::Ptr readOperation2 = ReadOperation::create();
  readOperation2->parameters()->findAs<smtk::attribute::IntItem>("sleep")->setValue(sleepValue);
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
    std::cout << "polling threads" << std::endl;
    status1 = result1.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  while (status2 != std::future_status::ready)
  {
    std::cout << "polling threads" << std::endl;
    status2 = result2.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  smtk::operation::Operation::Outcome outcome1 =
    smtk::operation::Operation::Outcome(result1.get()->findInt("outcome")->value());

  smtk::operation::Operation::Outcome outcome2 =
    smtk::operation::Operation::Outcome(result2.get()->findInt("outcome")->value());

  smtkTest(
    (outcome1 == smtk::operation::Operation::Outcome::SUCCEEDED), "Operation 1 should succeed.");
  smtkTest(
    (outcome2 == smtk::operation::Operation::Outcome::SUCCEEDED), "Operation 2 should succeed.");

  smtkTest((nMilliseconds < sleepValue * 100 * 1.5), "Read operations should run in parallel.");

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
  nMilliseconds = 0;

  while (status3 != std::future_status::ready)
  {
    std::cout << "polling threads" << std::endl;
    status3 = result3.wait_for(std::chrono::milliseconds(100));
    nMilliseconds += 100;
  }

  while (status4 != std::future_status::ready)
  {
    std::cout << "polling threads" << std::endl;
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

  smtkTest((nMilliseconds > sleepValue * 100 * 1.5), "Write operations should run serially.");

  return 0;
}
