//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/SubmitOperation.h"

#include "smtk/project/ResourceContainer.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonSubmitOperation.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"

#include <stdexcept>

// Define this to get debug messages.
#undef SMTK_DBG_SUBMITOPERATION

namespace smtk
{
namespace task
{

using namespace smtk::string::literals;

constexpr const char* const SubmitOperation::type_name;

smtk::string::Token SubmitOperation::ConfiguredByToken(ConfiguredBy value)
{
  switch (value)
  {
    case ConfiguredBy::Task:
      return "smtk::task::SubmitOperation::ConfiguredBy::Task"_token;
    case ConfiguredBy::Adaptor:
      return "smtk::task::SubmitOperation::ConfiguredBy::Adaptor"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ConfiguredBy value " << static_cast<int>(value) << ".");
      // fall through
    case ConfiguredBy::User:
      return "smtk::task::SubmitOperation::ConfiguredBy::User"_token;
  }
}

SubmitOperation::ConfiguredBy SubmitOperation::ConfiguredByValue(smtk::string::Token token)
{
  switch (token.id())
  {
    case "task"_hash:
    case "smtk::task::SubmitOperation::ConfiguredBy::Task"_hash:
      return ConfiguredBy::Task;
    case "adaptor"_hash:
    case "smtk::task::SubmitOperation::ConfiguredBy::Adaptor"_hash:
      return ConfiguredBy::Adaptor;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ConfiguredBy value " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "user"_hash:
    case "smtk::task::SubmitOperation::ConfiguredBy::User"_hash:
      return ConfiguredBy::User;
  }
}

smtk::string::Token SubmitOperation::ItemVisibilityToken(ItemVisibility value)
{
  switch (value)
  {
    case ItemVisibility::Off:
      return "smtk::task::SubmitOperation::ItemVisibility::Off"_token;
    case ItemVisibility::RecursiveOff:
      return "smtk::task::SubmitOperation::ItemVisibility::RecursiveOff"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ItemVisibility value " << static_cast<int>(value) << ".");
      // fall through
    case ItemVisibility::On:
      return "smtk::task::SubmitOperation::ItemVisibility::On"_token;
  }
}

SubmitOperation::ItemVisibility SubmitOperation::ItemVisibilityValue(smtk::string::Token token)
{
  switch (token.id())
  {
    case "smtk::task::SubmitOperation::ItemVisibility::Off"_hash:
      return ItemVisibility::Off;
    case "smtk::task::SubmitOperation::ItemVisibility::RecursiveOff"_hash:
      return ItemVisibility::RecursiveOff;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ItemVisibility token " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "smtk::task::SubmitOperation::ItemVisibility::On"_hash:
      return ItemVisibility::On;
  }
}

smtk::string::Token SubmitOperation::RunStyleToken(RunStyle value)
{
  switch (value)
  {
    case RunStyle::Iteratively:
      return "smtk::task::SubmitOperation::RunStyle::Iteratively"_token;
    case RunStyle::Once:
      return "smtk::task::SubmitOperation::RunStyle::Once"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(), "Unknown RunStyle value " << static_cast<int>(value) << ".");
      // fall through
    case RunStyle::OnCompletion:
      return "smtk::task::SubmitOperation::RunStyle::OnCompletion"_token;
  }
}

SubmitOperation::RunStyle SubmitOperation::RunStyleValue(smtk::string::Token token)
{
  switch (token.id())
  {
    case "iteratively-by-user"_hash:
    case "smtk::task::SubmitOperation::RunStyle::Iteratively"_hash:
      return RunStyle::Iteratively;
    case "once-only"_hash:
    case "smtk::task::SubmitOperation::RunStyle::Once"_hash:
      return RunStyle::Once;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown RunStyle token " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "upon-completion"_hash:
    case "smtk::task::SubmitOperation::RunStyle::OnCompletion"_hash:
      return RunStyle::OnCompletion;
  }
}

SubmitOperation::SubmitOperation()
{
  // Mark the association specifier as invalid by default.
  m_associationSpec.m_itemPath = "-ignore-";
}

SubmitOperation::SubmitOperation(
  const Configuration& config,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, taskManager, managers)
  , m_managers(managers)
{
  // Mark the association specifier as invalid by default.
  m_associationSpec.m_itemPath = "-ignore-";
  this->configure(config);
}

SubmitOperation::SubmitOperation(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, dependencies, taskManager, managers)
  , m_managers(managers)
{
  // Mark the association specifier as invalid by default.
  m_associationSpec.m_itemPath = "-ignore-";
  this->configure(config);
}

void SubmitOperation::configure(const Configuration& config)
{
#ifdef SMTK_DBG_SUBMITOPERATION
  std::cout << "Configure SubmitOperation\n" << config.dump(2) << "\n";
#endif
  // The predicate from_json method needs the resource manager:
  auto& helper = json::Helper::instance();
  helper.setManagers(m_managers);

  auto result = config.find("parameters");
  if (result != config.end())
  {
    result->get_to(m_parameterSpecs);
  }
  result = config.find("associations");
  if (result != config.end())
  {
    result->get_to(m_associationSpec);
  }
  else
  {
    m_associationSpec.m_itemPath = "-ignore-";
  }

  result = config.find("run-style");
  m_runStyle =
    (result != config.end() ? SubmitOperation::RunStyleValue(result->get<smtk::string::Token>())
                            : RunStyle::Iteratively);

  if (m_managers)
  {
    if (auto operationManager = m_managers->get<smtk::operation::Manager::Ptr>())
    {
      m_observer = operationManager->observers().insert(
        [this](
          const smtk::operation::Operation& op,
          smtk::operation::EventType event,
          smtk::operation::Operation::Result result) { return this->update(op, event, result); },
        /* priority */ 0,
        /* initialize */ true,
        "SubmitOperation monitors operations for updates.");
      result = config.find("operation");
      if (result != config.end())
      {
        m_operation = operationManager->create(result->get<std::string>());
        if (!m_operation)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not create an operation of type \"" << result->get<std::string>() << "\".");
          return;
        }
      }
      else
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(), "No operation type specified; task will be irrelevant.");
      }
      result = config.find("run-since-edited");
      if (result != config.end())
      {
        m_runSinceEdited = result->get<bool>();
      }
      else
      {
        // If not specified, assume we have not been run.
        m_runSinceEdited = false;
      }
    }
  }
  this->internalStateChanged(this->computeInternalState());
}

bool SubmitOperation::markCompleted(bool completed)
{
  // TODO: This should never be true because users cannot mark incomplete tasks completed.
  if (
    completed && m_operation && m_runStyle == RunStyle::OnCompletion &&
    !m_operation->ableToOperate())
  {
    return false;
  }

  bool result = this->Superclass::markCompleted(completed);
  if (completed && result && m_runStyle == RunStyle::OnCompletion)
  {
    // Launch the operation.
    m_operation->manager()->launchers()(m_operation);
  }
  return result;
}

smtk::common::Visit SubmitOperation::visitParameterSpecs(ConstParameterSpecVisitor visitor) const
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }
  for (const auto& entry : m_parameterSpecs)
  {
    if (visitor(entry) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
  }
  return smtk::common::Visit::Continue;
}

smtk::common::Visit SubmitOperation::visitParameterSpecs(ParameterSpecVisitor visitor)
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }
  for (auto& entry : m_parameterSpecs)
  {
    if (visitor(entry) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
  }
  return smtk::common::Visit::Continue;
}

void SubmitOperation::configureHiddenItems(
  smtk::view::ConfigurationPtr view,
  const nlohmann::json& jItemArray) const
{
  if (!m_operation)
  {
    return;
  }

  int attsIndex = view->details().findChild("InstancedAttributes");
  if (attsIndex < 0)
  {
#ifdef SMTK_DBG_SUBMITOPERATION
    std::cout << "View \"" << view->name() << "\" has no InstancedAttributes defined."
              << " (Operation " << m_operation->typeName() << ")\n";
#endif
    return;
  }

  // Current convention and style logic uses 1 attribute - let's get it
  smtk::view::Configuration::Component& comp = view->details().child(attsIndex);
  std::size_t i, n = comp.numberOfChildren();
  for (i = 0; i < n; i++)
  {
    smtk::view::Configuration::Component& attComp = comp.child(i);
    if (attComp.name() != "Att")
    {
      continue;
    }

    // Check for ItemViews section
    int ivIndex = attComp.findChild("ItemViews");
    auto& itemViewsComp = (ivIndex >= 0) ? attComp.child(ivIndex) : attComp.addChild("ItemViews");
    for (const auto& it : jItemArray)
    {
      std::string itemPath = it.get<std::string>();
      auto& ivComp = itemViewsComp.addChild("View");
      ivComp.setAttribute("Path", itemPath);
      ivComp.setAttribute("Type", "null");
    }

    break; // because operations only have 1 attribute, we can quit here
  }        // for (i)
}

bool SubmitOperation::setNeedsToRun()
{
  if (!m_runSinceEdited)
  {
    return false;
  }
  m_runSinceEdited = false;
  this->internalStateChanged(this->computeInternalState());
  return true;
}

bool SubmitOperation::editableCompletion() const
{
  switch (m_runStyle)
  {
    default:
    case RunStyle::OnCompletion:
    case RunStyle::Iteratively:
      return this->Task::editableCompletion();
    case RunStyle::Once:
      return this->state() == State::Completed;
  }
}

int SubmitOperation::update(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  (void)op;
  bool recheck = false;
  switch (event)
  {
    case smtk::operation::EventType::DID_OPERATE:
      // No need to check if we are irrelevant.
      if (!m_operation)
      {
        break;
      }
#ifdef SMTK_DBG_SUBMITOPERATION
      std::cout << "Update SubmitOperation(" << m_operation->typeName() << ") w " << op.typeName()
                << "\n";
#endif
      // If relevant, was the operation a Signal that the operation's
      // parameters have changed?
      if (dynamic_cast<const smtk::attribute::Signal*>(&op))
      {
        auto modifiedAttributes = result->findReference("modified");
        // TODO: Instead of just checking m_operation->parameters(), we should check whether
        //       anything in modifiedAttributes is owned by m_operation->specification().
        recheck = modifiedAttributes->contains(m_operation->parameters());
        m_runSinceEdited &= !recheck;
      }
      else if (&op == m_operation.get())
      {
        auto outcome =
          static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
        m_runSinceEdited = (outcome == smtk::operation::Operation::Outcome::SUCCEEDED);
        recheck = true;
        // If the operation was launched by us (upon the user marking the task
        // complete) but the operation failed, then un-complete the task.
        if (m_runStyle == RunStyle::OnCompletion && !m_runSinceEdited)
        {
          this->markCompleted(false);
        }
      }
      break;
    case smtk::operation::EventType::WILL_OPERATE:
      // TODO: We could cancel the operation if m_runStyle == OnCompletion and
      // the task state is not Completed; this would prevent users from running
      // the operation themselves and force them to use the task system. But that
      // would not be very user-friendly. Maybe take the opportunity to mark the
      // task as Completed? We would need to be careful about invoking task observers
      // from within operation observers.
      break;
  }
  if (recheck)
  {
    // The operation's parameters have been edited or the operation has been run successfully.
    // The following may change the task's state based on either of the above.
    this->internalStateChanged(this->computeInternalState());
    // We may need to transition states yet again in some cases.
    if (
      m_runStyle == RunStyle::Once && m_runSinceEdited &&
      this->internalState() == State::Completable)
    {
      this->markCompleted(true);
    }
  }
  return 0;
}

bool SubmitOperation::applyParameterSpecifications()
{
  bool didModify = false;
  if (!m_operation)
  {
    return didModify;
  }
  auto params = m_operation->parameters();
  for (const auto& spec : m_parameterSpecs)
  {
    auto item = params->itemAtPath(spec.m_itemPath); // pass "sep" or "active" args?
    if (!item)
    {
      continue;
    }
    didModify |= this->applyParameterSpecification(spec, item);
  }

  auto assoc = params->associations();
  if (m_associationSpec.m_itemPath != "-ignore-" && assoc)
  {
    didModify |= this->applyParameterSpecification(m_associationSpec, assoc);
  }
  return didModify;
}

bool SubmitOperation::applyParameterSpecification(
  const ParameterSpec& spec,
  const smtk::attribute::Item::Ptr& item)
{
  // TODO: Check spec.m_configuredBy matches the "source" of this call (which
  //       should be passed to us but is not currently). That way, this method
  //       may be called by an operation observer (e.g., a Signal operation
  //       implies ConfiguredBy::User), by a task-adaptor (ConfiguredBy::Task),
  //       or internally during deserialization (ConfiguredBy::Task).
  bool didModify = false;
  if (item->isOptional() && item->isEnabled() != spec.m_enable)
  {
    item->setIsEnabled(spec.m_enable);
    didModify = true;
  }
  // TODO: Dispatch to set values based on Item subclass...
  return didModify;
}

State SubmitOperation::computeInternalState() const
{
  if (m_completed && m_runSinceEdited)
  {
    // If the operation was run in a previous session, we must honor that.
    return State::Completed;
  }
  if (!m_operation)
  {
    return State::Irrelevant;
  }
  if (m_operation->ableToOperate())
  {
    switch (m_runStyle)
    {
      case RunStyle::Iteratively:
      case RunStyle::Once:
        return m_runSinceEdited ? State::Completable : State::Incomplete;
      case RunStyle::OnCompletion:
        return State::Completable;
    }
  }
  return State::Incomplete;
}

} // namespace task
} // namespace smtk
