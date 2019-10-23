//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Operation.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/operation/Operation_xml.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <mutex>
#include <sstream>

namespace
{
// We construct unique parameter and result names in a thread-safe way, rather
// than letting the attribute system spin one for us. This atomic counter is
// used to create that name. Its value is irrelevant so we don't need to reset
// it; its uniqueness is what we are after.
static std::atomic<std::size_t> g_uniqueCounter{ 0 };
}

namespace smtk
{
namespace operation
{

Operation::Operation()
  : m_debugLevel(0)
  , m_manager()
  , m_specification(nullptr)
  , m_parameters(nullptr)
  , m_resultDefinition(nullptr)
{
}

Operation::~Operation()
{
  // If the specification exists...
  if (m_specification != nullptr)
  {
    smtk::resource::ScopedLockGuard(m_specification->lock({}), smtk::resource::LockType::Write);

    // ...and if the parameters have been generated, remove the parameters from
    // the specification.
    if (m_parameters != nullptr)
    {
      m_specification->removeAttribute(m_parameters);
    }

    // Similarly, remove all results from the specification that were generated
    // by this operation.
    for (auto& result : m_results)
    {
      auto res = result.lock();
      if (!res)
      {
        continue;
      }

      m_specification->removeAttribute(res);
    }
  }
}

Operation::Specification Operation::specification()
{
  // Lazily create the specification.
  if (m_specification == nullptr)
  {
    if (auto manager = m_manager.lock())
    {
      auto metadata = manager->metadata().get<IndexTag>().find(this->index());

      // The only way for an operation's manager to be set is if a manager
      // created it. The only way for a manager to create an operation is if it
      // has a metadata instance for its type. Let's check anyway.
      assert(metadata != manager->metadata().get<IndexTag>().end());

      m_specification = metadata->specification();
    }
    else
    {
      m_specification = createSpecification();
    }
  }
  return m_specification;
}

bool Operation::configure(const smtk::attribute::AttributePtr&, const smtk::attribute::ItemPtr&)
{
  // Do nothing. Subclasses might want to do something, though.
  return false;
}

bool Operation::ableToOperate()
{
  return this->parameters()->isValid();
}

Operation::Result Operation::operate()
{
  // Gather all requested resources and their lock types.
  auto resourcesAndLockTypes = extractResourcesAndLockTypes(this->parameters());

  // Mutex to prevent multiple Operations from locking resources at the same
  // time (which could result in deadlock).
  static std::mutex mutex;

  // Lock the resources.
  mutex.lock();
  for (auto& resourceAndLockType : resourcesAndLockTypes)
  {
    auto resource = resourceAndLockType.first.lock();
    auto& lockType = resourceAndLockType.second;

// Leave this for debugging, but do not include it in every debug build
// as it can be quite noisy.
#if 0
    // Given the puzzling result of deadlock that can arise if one Operation
    // calls another Operation using its public API and passes it a Resource
    // with a Write LockType, we print to the terminal which resources we are
    // locking. If you are working on an Operation and are trying to debug a
    // deadlock, consider calling operations using the following syntax:
    // $
    // $ op->operate(Key());
    // $
    // This will avoid the inner Operation's resource locking and execute it
    // directly. Be sure to verify the operation's validity prior to execution
    // (via the ableToOperate() method).
    std::cout << "Operation \"" << this->typeName() << "\" is locking resource " << resource->name()
              << " (" << resource->typeName() << ") with lock type \""
              << (lockType == smtk::resource::LockType::Read
                     ? "Read"
                     : (lockType == smtk::resource::LockType::Write ? "Write" : "DoNotLock"))
              << "\"\n";
#endif

    resource->lock({}).lock(lockType);
  }
  mutex.unlock();

  // Remember where the log was so we only serialize messages for this
  // operation:
  std::size_t logStart = this->log().numberOfRecords();

  Result result;

  // If an operation manager is associated with the operation, call its pre- and
  // post-operation observers. Note that all observers will be called even if
  // one requests the operation be canceled. This is useful since all
  // DID_OPERATE observers are called whether the operation was canceled or not
  // -- and observers of both will expect them to be called in pairs.
  auto manager = m_manager.lock();
  bool observePostOperation = manager != nullptr;
  Outcome outcome;

  // First, we check that the operation is able to operate.
  if (!this->ableToOperate())
  {
    outcome = Outcome::UNABLE_TO_OPERATE;
    result = this->createResult(outcome);
    // If the operation cannot operate, there is no need to call any observers.
    observePostOperation = false;
  }
  // Then, we check if any observers wish to cancel this operation.
  else if (manager && manager->observers()(*this, EventType::WILL_OPERATE, nullptr))
  {
    outcome = Outcome::CANCELED;
    result = this->createResult(outcome);
  }
  else
  {
    // Finally, execute the operation.

    // Set the debug level if specified as a convenience for subclasses:
    smtk::attribute::IntItem::Ptr debugItem = this->parameters()->findInt("debug level");
    m_debugLevel = ((debugItem && debugItem->isEnabled()) ? debugItem->value() : 0);

    // Perform the derived operation.
    result = this->operateInternal();

    // Post-process the result if the operation was successful.
    outcome = static_cast<Outcome>(result->findInt("outcome")->value());
    if (outcome == Outcome::SUCCEEDED)
    {
      this->postProcessResult(result);
    }

    // By default, all executed operations are assumed to modify any input
    // resource accessed with a Write LockType and any resources referenced in
    // the result.
    if (outcome == Outcome::SUCCEEDED || outcome == Outcome::FAILED)
    {
      this->markModifiedResources(result);
    }
  }

  // Add a summary of the operation to the result.
  this->generateSummary(result);

  // Now grab all log messages and serialize them into the result attribute.
  {
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    {
      // Serialize relevant log records to a json-formatted string.
      auto records = this->log().records();
      nlohmann::json j = records;
      result->findString("log")->appendValue(j.dump());
    }
  }

  // Execute post-operation observation
  if (observePostOperation)
  {
    manager->observers()(*this, EventType::DID_OPERATE, result);
  }

  // Unlock the resources.
  for (auto& resourceAndLockType : resourcesAndLockTypes)
  {
    auto resource = resourceAndLockType.first.lock();
    auto& lockType = resourceAndLockType.second;

    resource->lock({}).unlock(lockType);
  }

  return result;
}

smtk::io::Logger& Operation::log() const
{
  return smtk::io::Logger::instance();
}

Operation::Parameters Operation::parameters()
{
  // If we haven't accessed our parameters yet, ask the specification to either
  // retrieve the exisiting one or create a new one.
  if (!m_parameters)
  {
    auto specification = this->specification();
    smtk::resource::ScopedLockGuard(specification->lock({}), smtk::resource::LockType::Write);
    if (m_parameters != nullptr)
    {
      return m_parameters;
    }
    m_parameters = createParameters(
      specification, this->typeName(), this->typeName() + std::to_string(g_uniqueCounter++));
  }

  // If we still don't have our parameters, then there's not much we can do.
  if (!m_parameters)
  {
    smtkErrorMacro(this->log(), "Could not identify parameters attribute definition for operation "
                                "\""
        << this->typeName() << "\".");
  }

  return m_parameters;
}

Operation::Parameters Operation::parameters() const
{
  if (!m_parameters)
  {
    // m_parameters is a cache variable. Rather than declare it mutable, we
    // const_cast here for its retrieval.
    return (const_cast<Operation*>(this))->parameters();
  }

  return m_parameters;
}

Operation::Result Operation::createResult(Outcome outcome)
{
  // Our result definition is located once per instance of the operation, and is
  // subsequently retrieved from cache to avoid superfluous lookups.
  if (!m_resultDefinition)
  {
    auto specification = this->specification();
    smtk::resource::ScopedLockGuard(specification->lock({}), smtk::resource::LockType::Read);
    m_resultDefinition = extractResultDefinition(specification, this->typeName());
  }

  // Now that we have our result definition, we create our result attribute.
  Result result;

  if (m_resultDefinition)
  {
    // Create a new instance of the result.
    {
      auto specification = this->specification();
      smtk::resource::ScopedLockGuard(specification->lock({}), smtk::resource::LockType::Write);
      result = specification->createAttribute(
        this->typeName() + "_result_" + std::to_string(g_uniqueCounter++), m_resultDefinition);
    }

    // Hold on to a copy of the generated result so we can remove it from our
    // specification when the operation is destroyed.
    m_results.push_back(result);
  }
  else
  {
    std::stringstream s;
    s << "Could not identify result attribute definition for operation \"" << this->typeName()
      << "\".";
    smtkErrorMacro(this->log(), s.str());
  }

  if (result)
  {
    result->findInt("outcome")->setValue(0, static_cast<int>(outcome));
  }
  return result;
}

void Operation::markModifiedResources(Operation::Result& result)
{
  // Gather all requested resources and their lock types.
  auto resourcesAndLockTypes = extractResourcesAndLockTypes(this->parameters());

  // Lock the resources.
  for (auto& resourceAndLockType : resourcesAndLockTypes)
  {
    auto resource = resourceAndLockType.first.lock();
    auto& lockType = resourceAndLockType.second;

    // If the operation was attempted (failed or succeeded), mark all resources
    // with write access as modified.
    if (resource != nullptr && lockType == smtk::resource::LockType::Write)
    {
      resource->setClean(false);
    }
  }

  // All resources referenced in the result are assumed to be modified.
  auto resourcesFromResult = extractResources(result);
  for (auto& rsrc : resourcesFromResult)
  {
    auto resource = rsrc.lock();
    if (resource != nullptr)
    {
      resource->setClean(false);
    }
  }
}

void Operation::generateSummary(Operation::Result& result)
{
  std::stringstream s;
  int outcome = result->findInt("outcome")->value();
  s << this->typeName() << ": ";
  switch (outcome)
  {
    case static_cast<int>(Outcome::UNABLE_TO_OPERATE):
      s << "unable to operate";
      break;
    case static_cast<int>(Outcome::CANCELED):
      s << "operation canceled";
      break;
    case static_cast<int>(Outcome::FAILED):
      s << "operation failed";
      break;
    case static_cast<int>(Outcome::SUCCEEDED):
      s << "operation succeeded";
      break;
    case static_cast<int>(Outcome::UNKNOWN):
      s << "outcome unknown";
      break;
  }

  if (outcome == static_cast<int>(Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), s.str());
  }
  else
  {
    smtkErrorMacro(this->log(), s.str());
  }
}

Operation::Specification Operation::createBaseSpecification() const
{
  Specification spec = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  reader.readContents(spec, Operation_xml, this->log());
  return spec;
}

} // operation namespace
} // smtk namespace
