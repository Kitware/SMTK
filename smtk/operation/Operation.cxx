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

#include "smtk/operation/queries/SynchronizedCache.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"

#include "smtk/operation/Operation_xml.h"

#include "nlohmann/json.hpp"

#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace
{
// We construct unique parameter and result names in a thread-safe way, rather
// than letting the attribute system spin one for us. This atomic counter is
// used to create that name. Its value is irrelevant so we don't need to reset
// it; its uniqueness is what we are after.
std::atomic<std::size_t> g_uniqueCounter{ 0 };
} // namespace

namespace smtk
{
namespace operation
{

Operation::Operation()
  : m_specification(nullptr)
  , m_parameters(nullptr)
  , m_resultDefinition(nullptr)
{
}

Operation::~Operation()
{
  // If the specification exists...
  if (m_specification != nullptr)
  {
    smtk::resource::ScopedLockGuard lock(
      m_specification->lock({}), smtk::resource::LockType::Write);

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

bool Operation::configure(
  const smtk::attribute::AttributePtr& /*unused*/,
  const smtk::attribute::ItemPtr& /*unused*/)
{
  // Do nothing. Subclasses might want to do something, though.
  return false;
}

bool Operation::ableToOperate()
{
  return this->parameters()->isValid();
}

ResourceAccessMap Operation::identifyLocksRequired()
{
  return extractResourcesAndLockTypes(this->parameters());
}

Operation::Result Operation::operate()
{
  // Call operate that will invoke observers.
  return this->operate(BaseKey(
    nullptr, ObserverOption::InvokeObservers, LockOption::LockAll, ParametersOption::Validate));
}

Operation::Result Operation::operate(const BaseKey& key)
{
  // Gather all requested resources and their lock types.
  const auto resourcesAndLockTypes = this->identifyLocksRequired();

  // Mutex to prevent multiple Operations from locking resources at the same
  // time (which could result in deadlock).
  static std::mutex mutex;

  // Track resources that were actually locked by this operation instance.
  ResourceAccessMap lockedByThis;

  size_t numRetries = 0;

  if (key.m_lockOption != LockOption::SkipLocks)
  {
    // Lock the resources.
    while (true)
    {
      // TODO: A maximum retry limit may be a good thing add here to prevent some operation from
      // occupying a thread in the operation thread pool indefinitely in the event that it cannot
      // acquire all of its resources' locks within a reasonable amount of time.
      if (numRetries++ > 0)
      {
        // Allow other threads to have CPU time.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      // Only allow one operation at a time to attempt to acquire all necessary resource locks.
      std::lock_guard<std::mutex> guard(mutex);

      if (key.m_parent)
      {
        // Inherit resource locks from the parent operation.
        this->m_lockedResources = key.m_parent->lockedResources();
      }

      // Populate queue for pending resource locks.
      std::queue<ResourceAccessMap::value_type> pending;
      for (const auto& resourceAndLockType : resourcesAndLockTypes)
      {
        pending.push(resourceAndLockType);
      }

      while (!pending.empty())
      {
        const auto resourceAndLockType = pending.front();
        auto resource = resourceAndLockType.first.lock();
        const auto& lockType = resourceAndLockType.second;

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

        // Is this resource already locked (by a parent perhaps).
        const auto it = this->m_lockedResources.find(resource);
        if (it != this->m_lockedResources.end())
        {
          // Verify that no writer is allowed while the (parent) operation
          // already has a read lock on the resource.
          if (it->second == resource::LockType::Read && lockType == resource::LockType::Write)
          {
            // This operation should not be able to operate. Undo the current
            // resource locking and fail early.
            this->unlockResources(lockedByThis);
            smtkErrorMacro(
              this->log(),
              "Attempted to acquire a write lock on a resource that a parent operation currently "
              "holds a read lock on.");
            return this->createResult(Outcome::UNABLE_TO_OPERATE);
          }
          else
          {
            // This lock is already acquired by the parent, so pop this
            // resource from the queue and continue at the next
            // resource.
            pending.pop();
            continue;
          }
        }
        else if (key.m_lockOption == LockOption::ParentLocksOnly)
        {
          // The parent does not already hold the lock for this resource, so
          // fail early.
          return this->createResult(Outcome::UNABLE_TO_OPERATE);
        }

        // Attempt to acquire this resource's lock.
        if (
          lockType != smtk::resource::LockType::DoNotLock && !resource->lock({}).tryLock(lockType))
        {
          // This resource's lock could not be acquired. Release the locks on any
          // resource whose lock was successfully acquired so that other threads
          // have a chance to attempt the locks if they are waiting.
          this->unlockResources(lockedByThis);
          lockedByThis.clear();
          this->m_lockedResources.clear();

          // Stop processing the queue. The locks will be attempted again later.
          break;
        }

        this->m_lockedResources.insert(resourceAndLockType);
        lockedByThis.insert(resourceAndLockType);

        // Pop this resource from the queue.
        pending.pop();
      } // while (!pending.empty())

      if (pending.empty())
      {
        // All of the locks were acquired successfully. Continue with the
        // operation logic.
        break;
      }
      else
      {
        // All of the resource locks could not be acquired. Release the locks on any
        // resource whose lock was successfully acquired so that other threads
        // have a chance to attempt the locks if they are waiting.
        this->unlockResources(lockedByThis);
        lockedByThis.clear();
        this->m_lockedResources.clear();
      }
    } // while (true)
  }

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
  Outcome outcome = Outcome::UNKNOWN;

  // First, we check that the operation is able to operate.
  if (key.m_paramsOption == ParametersOption::Validate && !this->ableToOperate())
  {
    outcome = Outcome::UNABLE_TO_OPERATE;
    result = this->createResult(outcome);
    // If the operation cannot operate, there is no need to call any observers.
    observePostOperation = false;
  }
  // Then, we check if any observers wish to cancel this operation.
  else
  {
    if (
      key.m_observerOption == ObserverOption::InvokeObservers && manager &&
      manager->observers()(*this, EventType::WILL_OPERATE, nullptr))
    {
      outcome = Outcome::CANCELED;
      result = this->createResult(outcome);
    }

    if (outcome != Outcome::CANCELED)
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
  if (key.m_observerOption == ObserverOption::InvokeObservers && observePostOperation && manager)
  {
    manager->observers()(*this, EventType::DID_OPERATE, result);
  }

  // Un-manage any resources marked for removal before releasing locks.
  bool removed = this->unmanageResources(result);
  if (!removed)
  {
    smtkErrorMacro(this->log(), "Failed to remove resources marked for removal.");
    result->findInt("outcome")->setValue(
      static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
  }

  // Unlock the resources locked by this operation instance.
  this->unlockResources(lockedByThis);
  this->m_lockedResources.clear();

  return result;
}

void Operation::unlockResources(const ResourceAccessMap& resources)
{
  for (const auto& resourceAndLockType : resources)
  {
    auto resource = resourceAndLockType.first.lock();
    const auto& lockType = resourceAndLockType.second;
    resource->lock({}).unlock(lockType);
  }
}

Operation::Outcome Operation::safeOperate()
{
  Handler dummy = [](const Operation&, Operation::Result) {};
  Outcome outcome = this->safeOperate(dummy);
  return outcome;
}

Operation::Outcome Operation::safeOperate(Handler handler)
{
  Outcome outcome = Outcome::UNKNOWN;
  auto result = this->operate();
  if (result)
  {
    outcome = static_cast<Outcome>(result->findInt("outcome")->value());
    if (handler)
    {
      handler(*this, result);
    }
    if (m_specification)
    {
      m_specification->removeAttribute(result);
    }
  }
  return outcome;
}

bool Operation::releaseResult(Result& result)
{
  if (!m_specification)
  {
    return false;
  }
  return m_specification->removeAttribute(result);
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
    smtk::resource::ScopedLockGuard lock(specification->lock({}), smtk::resource::LockType::Write);
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
    smtkErrorMacro(
      this->log(),
      "Could not identify parameters attribute definition for operation "
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
    smtk::resource::ScopedLockGuard lock(specification->lock({}), smtk::resource::LockType::Read);
    m_resultDefinition = extractResultDefinition(specification, this->typeName());
  }

  // Now that we have our result definition, we create our result attribute.
  Result result;

  if (m_resultDefinition)
  {
    // Create a new instance of the result.
    {
      auto specification = this->specification();
      smtk::resource::ScopedLockGuard lock(
        specification->lock({}), smtk::resource::LockType::Write);
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
  for (const auto& rsrc : resourcesFromResult)
  {
    auto resource = rsrc.lock();
    if (resource != nullptr)
    {
      resource->setClean(false);

      // Allow synchronized query caches to update according to this result.
      for (auto& cache : resource->queries().caches())
      {
        if (auto* synchronizedCache = dynamic_cast<SynchronizedCache*>(cache.second.get()))
        {
          synchronizedCache->synchronize(*this, result);
        }
      }
    }
  }
}

namespace
{
bool removeResource(
  const smtk::resource::ResourcePtr& resource,
  const smtk::resource::ManagerPtr& resourceManager,
  bool shouldRemoveLinks)
{
  // we need to remove links before removing the resource, otherwise
  // the resource will just load again.
  if (shouldRemoveLinks)
  {
    resourceManager->visit([&resource](smtk::resource::Resource& rsrc) {
      rsrc.links().removeAllLinksTo(resource);
      return smtk::common::Processing::CONTINUE;
    });
  }
  // resource manager observers should see the resource as ready to close.
  resource->setClean(true);
  bool removed = resourceManager->remove(resource);
  return removed;
}
} // namespace

bool Operation::unmanageResources(Operation::Result& result)
{
  auto item = result->findResource("resourcesToExpunge");
  auto removeLinksItem =
    result->findAs<smtk::attribute::VoidItem>("removeAssociations", smtk::attribute::RECURSIVE);
  // Access the resource manager (provided by the operation manager that created
  // this operation, if any)
  auto resourceManager = this->resourceManager();
  if (!item || !resourceManager)
  {
    return true;
  }

  bool ret = true;
  for (std::size_t i = 0; i < item->numberOfValues(); i++)
  {
    // no need to look at items that cannot be resolved
    if (item->value(i) == nullptr)
    {
      continue;
    }

    // ...access the associated resource.
    smtk::resource::ResourcePtr resource =
      std::dynamic_pointer_cast<smtk::resource::Resource>(item->value(i));
    if (resource)
    {
      bool shouldRemoveLinks = removeLinksItem ? removeLinksItem->isEnabled() : false;
      bool shouldRemove = true;
      auto project = std::dynamic_pointer_cast<smtk::project::Project>(resource);
      if (project)
      {
        // We were given a project. Close it and all its resources.
        for (const auto& rsrc : project->resources())
        {
          ret &= removeResource(rsrc, resourceManager, true);
        }
      }
      else if (this->managers())
      {
        // see if this resource is parented to another resource - if so give a warning
        if (resource->parentResource() != nullptr)
        {
          smtkWarningMacro(
            this->log(),
            "Resource (" << resource->name() << ") being closed belongs to another resource: "
                         << resource->parentResource()->name());
        }
      }
      if (shouldRemove)
      {
        bool rmvRet = removeResource(resource, resourceManager, shouldRemoveLinks);
        ret &= rmvRet;
      }
    }
  }
  return ret;
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

Operation::BaseKey Operation::childKey(
  ObserverOption observerOption,
  LockOption lockOption,
  ParametersOption paramsOption) const
{
  return BaseKey(this, observerOption, lockOption, paramsOption);
}

smtk::resource::ManagerPtr Operation::resourceManager()
{

  if (auto mgr = manager())
  {
    if (auto mgrs = mgr->managers())
    {
      if (mgrs->contains<smtk::resource::Manager::Ptr>())
      {
        return mgrs->get<smtk::resource::Manager::Ptr>();
      }
    }
  }

  return smtk::resource::ManagerPtr();
}

bool Operation::restoreTrace(const std::string& trace)
{
  auto specification = this->specification();
  smtk::resource::ScopedLockGuard lock(specification->lock({}), smtk::resource::LockType::Write);
  // clear existing instances
  Operation::Definition parameterDefinition =
    extractParameterDefinition(specification, this->typeName());
  std::vector<smtk::attribute::AttributePtr> defnAttrs;
  specification->findAttributes(parameterDefinition, defnAttrs);
  if (!defnAttrs.empty())
  {
    for (const auto& attr : defnAttrs)
    {
      specification->removeAttribute(attr);
    }
  }
  // repopulate with instances from the trace
  smtk::io::AttributeReader reader;
  if (!reader.readContents(specification, trace, this->log()))
  {
    specification->findAttributes(parameterDefinition, defnAttrs);
    if (!defnAttrs.empty())
    {
      // set m_parameters so the default isn't re-created.
      m_parameters = defnAttrs[0];
      return true;
    }
  }
  return false;
}

Operation::Outcome outcome(const Operation::Result& result)
{
  return static_cast<Operation::Outcome>(result->findInt("outcome")->value());
}

bool setOutcome(const Operation::Result& result, Operation::Outcome outcome)
{
  auto value = static_cast<int>(outcome);
  auto item = result->findInt("outcome");
  if (!item || item->value() == value)
  {
    return false;
  }
  return item->setValue(value);
}

} // namespace operation
} // namespace smtk
