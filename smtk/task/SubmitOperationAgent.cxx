//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/SubmitOperationAgent.h"

#include "smtk/project/ResourceContainer.h"

#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonSubmitOperationAgent.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/json/jsonAttribute.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/json/Helper.h"
#include "smtk/common/json/jsonLinks.h"
#include "smtk/common/json/jsonUUID.h"
#include "smtk/resource/json/jsonResourceLinkBase.h"
#include "smtk/string/json/jsonToken.h"

#include "smtk/resource/json/jsonResource.h"

#include "smtk/common/json/jsonTypeMap.h"
#include "smtk/io/Logger.h"

#include <stdexcept>

// Define this to get debug messages.
#undef SMTK_DBG_SUBMITOPERATION

// Ignore warning about non-inlined template specializations of smtk::common::Helper<>
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#pragma warning(disable : 4506) /* no definition for inline function */
#endif

namespace smtk
{
namespace task
{

using namespace smtk::string::literals;

namespace
{

void addAttributesWithMatchingItems(
  const std::string& selector,
  const std::string& sourcePath,
  SubmitOperationAgent::ParameterWatchMap& watching,
  SubmitOperationAgent::ParameterUpdateMap& parametersToUpdate,
  SubmitOperationAgent::ParameterSpecRef& ref,
  smtk::resource::Resource* rsrc)
{
  auto matches = rsrc->filterAs<std::set<smtk::attribute::AttributePtr>>(selector);
  for (const auto& match : matches)
  {
    auto sourceItem = match->itemAtPath(sourcePath);
    if (sourceItem)
    {
      watching[match->id()].insert(ref);
      parametersToUpdate[ref].insert(match.get());
    }
  }
}

} // anonymous namespace

SubmitOperationAgent::SubmitOperationAgent(Task* owningTask)
  : Agent(owningTask)
{
}

smtk::string::Token SubmitOperationAgent::ConfiguredByToken(ConfiguredBy value)
{
  switch (value)
  {
    case ConfiguredBy::Static:
      return "smtk::task::SubmitOperationAgent::ConfiguredBy::Static"_token;
    case ConfiguredBy::Port:
      return "smtk::task::SubmitOperationAgent::ConfiguredBy::Port"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ConfiguredBy value " << static_cast<int>(value) << ".");
      // fall through
    case ConfiguredBy::User:
      return "smtk::task::SubmitOperationAgent::ConfiguredBy::User"_token;
  }
}

SubmitOperationAgent::ConfiguredBy SubmitOperationAgent::ConfiguredByValue(
  smtk::string::Token token)
{
  switch (token.id())
  {
    case "port"_hash:
    case "smtk::task::SubmitOperationAgent::ConfiguredBy::Port"_hash:
      return ConfiguredBy::Port;
    case "static"_hash:
    case "smtk::task::SubmitOperationAgent::ConfiguredBy::Static"_hash:
      return ConfiguredBy::Static;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ConfiguredBy value " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "user"_hash:
    case "smtk::task::SubmitOperationAgent::ConfiguredBy::User"_hash:
      return ConfiguredBy::User;
  }
}

smtk::string::Token SubmitOperationAgent::ItemVisibilityToken(ItemVisibility value)
{
  switch (value)
  {
    case ItemVisibility::Off:
      return "smtk::task::SubmitOperationAgent::ItemVisibility::Off"_token;
    case ItemVisibility::RecursiveOff:
      return "smtk::task::SubmitOperationAgent::ItemVisibility::RecursiveOff"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ItemVisibility value " << static_cast<int>(value) << ".");
      // fall through
    case ItemVisibility::On:
      return "smtk::task::SubmitOperationAgent::ItemVisibility::On"_token;
  }
}

SubmitOperationAgent::ItemVisibility SubmitOperationAgent::ItemVisibilityValue(
  smtk::string::Token token)
{
  switch (token.id())
  {
    case "smtk::task::SubmitOperationAgent::ItemVisibility::Off"_hash:
      return ItemVisibility::Off;
    case "smtk::task::SubmitOperationAgent::ItemVisibility::RecursiveOff"_hash:
      return ItemVisibility::RecursiveOff;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown ItemVisibility token " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "smtk::task::SubmitOperationAgent::ItemVisibility::On"_hash:
      return ItemVisibility::On;
  }
}

smtk::string::Token SubmitOperationAgent::RunStyleToken(RunStyle value)
{
  switch (value)
  {
    case RunStyle::Iteratively:
      return "smtk::task::SubmitOperationAgent::RunStyle::Iteratively"_token;
    case RunStyle::Once:
      return "smtk::task::SubmitOperationAgent::RunStyle::Once"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(), "Unknown RunStyle value " << static_cast<int>(value) << ".");
      // fall through
    case RunStyle::OnCompletion:
      return "smtk::task::SubmitOperationAgent::RunStyle::OnCompletion"_token;
  }
}

SubmitOperationAgent::RunStyle SubmitOperationAgent::RunStyleValue(smtk::string::Token token)
{
  switch (token.id())
  {
    case "iteratively-by-user"_hash:
    case "smtk::task::SubmitOperationAgent::RunStyle::Iteratively"_hash:
      return RunStyle::Iteratively;
    case "once-only"_hash:
    case "smtk::task::SubmitOperationAgent::RunStyle::Once"_hash:
      return RunStyle::Once;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown RunStyle token " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "upon-completion"_hash:
    case "smtk::task::SubmitOperationAgent::RunStyle::OnCompletion"_hash:
      return RunStyle::OnCompletion;
  }
}

smtk::string::Token SubmitOperationAgent::PortDataHandlerToken(PortDataHandler value)
{
  switch (value)
  {
    case PortDataHandler::AddObjects:
      return "smtk::task::SubmitOperationAgent::PortDataHandler::AddObjects"_token;
    case PortDataHandler::SetObjects:
      return "smtk::task::SubmitOperationAgent::PortDataHandler::SetObjects"_token;
    case PortDataHandler::AssignFromAttribute:
      return "smtk::task::SubmitOperationAgent::PortDataHandler::AssignFromAttribute"_token;
    case PortDataHandler::AssignFromAttributeResource:
      return "smtk::task::SubmitOperationAgent::PortDataHandler::AssignFromAttributeResource"_token;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown PortDataHandler value " << static_cast<int>(value) << ".");
      // fall through
    case PortDataHandler::AssignMatchingAttributes:
      return "smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes"_token;
  }
}

SubmitOperationAgent::PortDataHandler SubmitOperationAgent::PortDataHandlerValue(
  smtk::string::Token token)
{
  switch (token.id())
  {
    case "add"_hash:
    case "smtk::task::SubmitOperationAgent::PortDataHandler::AddObjects"_hash:
      return PortDataHandler::AddObjects;
    case "set"_hash:
    case "smtk::task::SubmitOperationAgent::PortDataHandler::SetObjects"_hash:
      return PortDataHandler::SetObjects;
    case "assign-from-attribute"_hash:
    case "smtk::task::SubmitOperationAgent::PortDataHandler::AssignFromAttribute"_hash:
      return PortDataHandler::AssignFromAttribute;
    case "assign-from-attribute-resource"_hash:
    case "smtk::task::SubmitOperationAgent::PortDataHandler::AssignFromAttributeResource"_hash:
      return PortDataHandler::AssignFromAttributeResource;
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown PortDataHandler token " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "assign-from-matching-attributes"_hash:
    case "smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes"_hash:
      return PortDataHandler::AssignMatchingAttributes;
  }
}

void SubmitOperationAgent::configure(const Configuration& config)
{
#ifdef SMTK_DBG_SUBMITOPERATION
  std::cout << "Configure SubmitOperationAgent\n" << config.dump(2) << "\n";
#endif
  State prev = m_internalState;
  // The predicate from_json method needs the resource manager:
  auto mgrs = m_parent->managers();
  auto& helper = json::Helper::instance();
  helper.setManagers(mgrs);

  m_internalState = smtk::task::stateEnum(config.at("internal-state"));
  auto result = config.find("parameters");
  if (result != config.end())
  {
    try
    {
      m_parameterSpecs.clear();
      result->get_to(m_parameterSpecs);
    }
    catch (nlohmann::json::exception& e)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not deserialize parameters \"" << result->dump(2) << "\".");
    }
  }
  result = config.find("watching");
  if (result != config.end())
  {
    m_watching.clear();
    result->get_to(m_watching);
  }

  result = config.find("run-style");
  m_runStyle =
    (result != config.end()
       ? SubmitOperationAgent::RunStyleValue(result->get<smtk::string::Token>())
       : RunStyle::Iteratively);

  if (mgrs)
  {
    if (auto operationManager = mgrs->get<smtk::operation::Manager::Ptr>())
    {
      if (m_observer.assigned())
      {
        operationManager->observers().erase(m_observer);
      }
      m_observer = operationManager->observers().insert(
        [this](
          const smtk::operation::Operation& op,
          smtk::operation::EventType event,
          smtk::operation::Operation::Result result) { return this->update(op, event, result); },
        /* priority */ 0,
        /* initialize */ true,
        "SubmitOperationAgent monitors operations for updates.");

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
        // If the operation specification does not have a manager assigned to it, set one.
        // We need to do this since we may be dealing with link information which requires
        // a resource manager to decode them.
        if (m_operation->specification()->manager() == nullptr)
        {
          m_operation->specification()->setManager(mgrs->get<smtk::resource::Manager::Ptr>());
        }
        result = config.find("op-links");
        if (result != config.end())
        {
          try
          {
            auto* helper = smtk::common::Helper<>::activate();
            helper->setLeftPlaceholderId(m_operation->specification()->id());
            helper->setRightPlaceholderId(m_operation->specification()->id());
            m_operation->specification()->links().data() = *result;
            helper->deactivate();
          }
          catch (std::exception& e)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(), "Error deserializing links: \"" << e.what() << "\".");
          }
        }

        result = config.find("op-params");
        if (result != config.end())
        {
          auto opp = m_operation->parameters();
          std::vector<smtk::attribute::ItemExpressionInfo> itemExpressionInfo;
          std::vector<smtk::attribute::AttRefInfo> attRefInfo;
          std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
          smtk::attribute::from_json(
            *result, opp, itemExpressionInfo, attRefInfo, convertedAttDefs);
          m_operation->specification()->resetId(opp, result->at("ID").get<smtk::common::UUID>());
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

    // Attempt to deserialize output port data from previous session.
    // All resources mentioned by the operation must be present for
    // this to work, but the project owning this agent's task may not
    // be registered yet.
    result = config.find("output-resources");
    if (result != config.end())
    {
      auto parentRsrc = m_parent ? m_parent->resource() : nullptr;
      auto rsrcMgr = mgrs->get<smtk::resource::Manager::Ptr>();
      if (rsrcMgr)
      {
        for (const auto& jId : *result)
        {
          auto rsrcId = jId.get<smtk::common::UUID>();
          auto rsrc = rsrcMgr->get(rsrcId);
          if (!rsrc && parentRsrc->id() == rsrcId)
          {
            m_outputResources.insert(parentRsrc);
          }
          else if (rsrc)
          {
            m_outputResources.insert(rsrc);
          }
        }
      }
    }
    result = config.find("output-port");
    if (result != config.end())
    {
      m_outputPortName = result->get<smtk::string::Token>();
    }
    result = config.find("output-role");
    if (result != config.end())
    {
      m_outputRole = result->get<smtk::string::Token>();
    }
  }
  m_parent->updateAgentState(this, prev, this->computeInternalState());
}

Agent::Configuration SubmitOperationAgent::configuration() const
{
  auto config = this->Superclass::configuration();
  if (m_operation)
  {
    config["operation"] = m_operation->typeName();
    nlohmann::json jsonOpParam;
    smtk::attribute::to_json(jsonOpParam, m_operation->parameters());
    config["op-params"] = jsonOpParam;
    auto* helper = smtk::common::Helper<>::activate();
    helper->requiredIds().insert(m_operation->parameters()->id());
    helper->requiredIds().insert(m_operation->specification()->id());
    helper->setLeftPlaceholderId(m_operation->specification()->id());
    helper->setRightPlaceholderId(m_operation->specification()->id());
    config["op-links"] = m_operation->specification()->links().data();
    helper->deactivate();
  }
  config["parameters"] = m_parameterSpecs;
  if (!m_watching.empty())
  {
    config["watching"] = m_watching;
  }
  config["run-style"] = SubmitOperationAgent::RunStyleToken(m_runStyle);
  if (m_runSinceEdited)
  {
    config["run-since-edited"] = true;
  }
  config["internal-state"] = smtk::task::stateName(m_internalState);

  if (m_outputPortName.valid() && m_outputRole.valid())
  {
    config["output-port"] = m_outputPortName;
    config["output-role"] = m_outputRole;
    nlohmann::json::array_t portRsrcs;
    for (const auto& weakRsrc : m_outputResources)
    {
      if (auto rsrc = weakRsrc.lock())
      {
        portRsrcs.push_back(rsrc->id());
      }
    }
    if (!portRsrcs.empty())
    {
      config["output-resources"] = portRsrcs;
    }
  }
  return config;
}

std::string SubmitOperationAgent::troubleshoot() const
{
  std::string result;
  switch (this->state())
  {
    default:
    case State::Irrelevant:
    case State::Unavailable:
    case State::Completable:
    case State::Completed:
      break;
    case State::Incomplete:
      result = R"(<li>You must run the operation at least once.</li>)";
      break;
  }
  return result;
}

std::shared_ptr<PortData> SubmitOperationAgent::portData(const Port* port) const
{
  if (
    !m_outputPortName.valid() || !m_outputRole.valid() || !port || port->name() != m_outputPortName)
  {
    return nullptr;
  }
  auto data = std::make_shared<smtk::task::ObjectsInRoles>();
  bool empty = true;
  for (const auto& weakRsrc : m_outputResources)
  {
    if (auto rsrc = weakRsrc.lock())
    {
      data->addObject(rsrc.get(), m_outputRole);
      empty = false;
    }
  }
  if (empty)
  {
    return nullptr;
  }
  return data;
}

void SubmitOperationAgent::portDataUpdated(const Port* port)
{
  if (!port)
  {
    return;
  }
  smtk::string::Token portName = port->name();
  auto specIt = m_parameterSpecs.find(portName);
  if (specIt == m_parameterSpecs.end())
  {
    return;
  }

  State prev = m_internalState;
  ParameterUpdateMap parametersToUpdate;
  for (auto* conn : port->connections())
  {
    auto portData = std::dynamic_pointer_cast<ObjectsInRoles>(port->portData(conn));
    const auto& roleMap = portData->data();
    if (portData)
    {
      for (const auto& roleEntry : specIt->second)
      {
        auto roleIt = roleMap.find(roleEntry.first);
        if (roleIt == roleMap.end())
        {
          continue;
        }
        // We found a role in the port-data that matches our port and role.
        smtk::string::Token roleName(roleEntry.first);
        for (const auto& obj : roleIt->second)
        {
          auto* rsrc = dynamic_cast<smtk::resource::Resource*>(obj);
          auto* comp = dynamic_cast<smtk::resource::Component*>(obj);
          if (comp)
          {
            rsrc = comp->parentResource();
          }
          if (!rsrc)
          {
            continue;
          } // Skip components without a parent resource.

          for (std::size_t specIdx = 0; specIdx < roleEntry.second.size(); ++specIdx)
          {
            const auto& spec(roleEntry.second[specIdx]);
            // Check whether the object is accepted for the specIdx-th parameter.
            if (!spec.m_resourceTypeName.empty() && !rsrc->matchesType(spec.m_resourceTypeName))
            {
              continue; // Skip resources of improper types.
            }
            if (spec.m_resourceTemplate.valid() && rsrc->templateType() != spec.m_resourceTemplate)
            {
              continue; // Skip resources with invalid template-types (schema).
            }

            ParameterSpecRef ref(portName, roleName, specIdx, /* expunge */ false);
            // NB: We could cache queryOp by (rsrc, m_componentSelector) tuples if this is too slow.
            //     It is burdensome to re-parse the spec in a loop over objects.
            auto queryOp = rsrc->queryOperation(spec.m_componentSelector);
            switch (spec.m_portDataHandler)
            {
              case PortDataHandler::AddObjects:
              case PortDataHandler::SetObjects:
                // Skip objects we know to avoid
                if (comp && !queryOp(*comp))
                {
                  continue;
                }
                // Start monitoring the object:
                m_watching[obj->id()].insert(ref);
                parametersToUpdate[ref].insert(obj);
                break;
              case PortDataHandler::AssignFromAttribute:
                // Only accept components that are attributes.
                if (!comp || !queryOp(*comp))
                {
                  continue;
                }
                // Start monitoring the object:
                m_watching[comp->id()].insert(ref);
                parametersToUpdate[ref].insert(comp);
                break;
              case PortDataHandler::AssignFromAttributeResource:
              {
                // Require the object to be an attribute resource; fetch matching attributes.
                if (obj != rsrc)
                {
                  continue;
                }
                addAttributesWithMatchingItems(
                  spec.m_componentSelector,
                  spec.m_sourceItemPath,
                  m_watching,
                  parametersToUpdate,
                  ref,
                  rsrc);
              }
              break;
              case PortDataHandler::AssignMatchingAttributes:
                if (comp && queryOp(*comp))
                {
                  m_watching[comp->id()].insert(ref);
                  parametersToUpdate[ref].insert(comp);
                }
                else if (obj == rsrc)
                {
                  addAttributesWithMatchingItems(
                    spec.m_componentSelector,
                    spec.m_sourceItemPath,
                    m_watching,
                    parametersToUpdate,
                    ref,
                    rsrc);
                }
                break;
            }
          }
        }
      }
    }
  }
  // Now that we've updated the list of watched objects, update their
  // corresponding parameter values. Note that this may cause problems
  // once we support multiple objects contributing to the same parameter
  // as objects may be sourced from multiple ports but this method only
  // takes one port into account.
  if (this->copyToParameters(parametersToUpdate))
  {
    // Parameter values changed; check whether the state changes.
    m_parent->updateAgentState(this, prev, this->computeInternalState());
    // Launch a Signal operation to notify the UI the operation
    // parameters have changed.
    auto mgrs = m_parent->managers();
    bool didLaunch = false;
    if (auto operationManager = mgrs->get<smtk::operation::Manager::Ptr>())
    {
      if (auto signalOp = operationManager->create<smtk::attribute::Signal>())
      {
        std::set<std::string> updatedItems;
        for (const auto& ref : parametersToUpdate)
        {
          auto portIt = m_parameterSpecs.find(ref.first.m_portName);
          if (portIt != m_parameterSpecs.end())
          {
            auto roleIt = portIt->second.find(ref.first.m_roleName);
            if (roleIt != portIt->second.end())
            {
              if (ref.first.m_specIndex <= roleIt->second.size())
              {
                updatedItems.insert(roleIt->second[ref.first.m_specIndex].m_targetItemPath);
              }
            }
          }
        }
        signalOp->parameters()->findString("source")->setValue("SubmitOperationAgent");
        signalOp->parameters()->findComponent("modified")->appendValue(m_operation->parameters());
        signalOp->parameters()->findString("items")->setValues(
          updatedItems.begin(), updatedItems.end());
        operationManager->launchers()(signalOp);
        didLaunch = true;
      }
    }
    if (!didLaunch)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "Could not create Signal Operation for attribute event.");
    }
  }
}

void SubmitOperationAgent::configureHiddenItems(
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

bool SubmitOperationAgent::setNeedsToRun()
{
  if (!m_runSinceEdited)
  {
    return false;
  }
  State prev = m_internalState;
  m_runSinceEdited = false;
  m_parent->updateAgentState(this, prev, this->computeInternalState());
  return true;
}

void SubmitOperationAgent::taskStateChanged(State prev, State& next)
{
  // If we are downgraded from a completed state, require the operation to re-run.
  if (prev == State::Completed && next < prev)
  {
    State prevInternalState = m_internalState;
    m_internalState = m_internalState > State::Incomplete ? State::Incomplete : m_internalState;
    if (next < State::Incomplete)
    {
      // Do not call observers; just update the parent task's agent state;
      // because taskStateChanged is called from changeState and next < m_internalState,
      // we are already transitioning to a "lower" state than the agent would require.
      m_parent->updateAgentState(this, prevInternalState, m_internalState, false);
    }
    else
    {
      // Call setNeedsToRun() here; that will trigger observer notifications.
      this->setNeedsToRun();
      next = m_internalState;
    }
  }
  else if (next == State::Completed && prev == State::Completable)
  {
    // For OnCompletion style, when the user marks the task completed,
    // we launch the operation. If we observe the operation failing, the
    // task will then be marked incomplete.
    if (m_runStyle == RunStyle::OnCompletion && m_operation)
    {
      m_operation->manager()->launchers()(m_operation);
    }
  }
}

void SubmitOperationAgent::taskStateChanged(Task* task, State prev, State next)
{
  (void)task;
  (void)prev;
  (void)next;
  // We do not care about child-task state-changes.
}

int SubmitOperationAgent::update(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  (void)op;
  bool recheck = false;
  switch (event)
  {
    case smtk::operation::EventType::DID_OPERATE:
    {
      // No need to check if we are irrelevant.
      if (!m_operation)
      {
        break;
      }
#ifdef SMTK_DBG_SUBMITOPERATION
      std::cout << "Update SubmitOperationAgent(" << m_operation->typeName() << ") w "
                << op.typeName() << "\n";
#endif
      // Identify whether any watched components should modify operation parameters.
      // If so, populate parametersToUpdate.
      ParameterUpdateMap parametersToUpdate;
      this->prepareParameterUpdates(
        parametersToUpdate, result->findComponent("created"), false, recheck);
      this->prepareParameterUpdates(
        parametersToUpdate, result->findComponent("modified"), false, recheck);
      this->prepareParameterUpdates(
        parametersToUpdate, result->findComponent("expunged"), true, recheck);
      if (this->copyToParameters(parametersToUpdate))
      {
        // Parameters were modified; the operation needs to be re-run.
        // TODO: Instead of just checking m_operation->parameters(), we should check whether
        //       anything in modifiedComponents is owned by m_operation->specification().
        recheck = true;
        m_runSinceEdited = false;
      }
      // Are we being notified that the operation this agent manages was run?
      // If so, then handle completion of the operation (which indicates the task
      // is completable or completed).
      if (&op == m_operation.get())
      {
        auto outcome = smtk::operation::outcome(result);
        m_runSinceEdited = (outcome == smtk::operation::Operation::Outcome::SUCCEEDED);
        recheck = true;
        // If the operation succeeded and we are configured to produce
        // results on an output port, capture the resources from the result.
        // Otherwise, clear the resources recorded from the previous run.
        if (m_outputPortName.valid() && m_runSinceEdited)
        {
          m_outputResources.clear();
          for (const auto& weakRsrc : smtk::operation::extractResources(result))
          {
            if (auto rsrc = weakRsrc.lock())
            {
              // Include all mentioned resources except for the operation's specification.
              if (rsrc->id() != op.parameters()->parentResource()->id())
              {
                m_outputResources.insert(rsrc);
              }
            }
          }
        }
        else
        {
          m_outputResources.clear();
        }
        // If the operation failed, ensure the task is not marked "completed."
        if (!m_runSinceEdited)
        {
          m_parent->markCompleted(false);
        }
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
      return 0;
      break;
  }
  if (recheck)
  {
    // The operation's parameters have been edited or the operation has been run successfully.
    // The following may change the task's state based on either of the above.
    m_parent->updateAgentState(this, m_internalState, this->computeInternalState());
    // We may need to transition states yet again in some cases.
    if (m_runStyle == RunStyle::Once && m_runSinceEdited && m_internalState >= State::Completable)
    {
      m_parent->markCompleted(true);
    }
  }
  return 0;
}

bool SubmitOperationAgent::prepareParameterUpdates(
  ParameterUpdateMap& parametersToUpdate,
  const std::shared_ptr<smtk::attribute::ComponentItem>& components,
  bool expunging,
  bool& requireStateCheck)
{
  (void)requireStateCheck;
  for (const auto& component : *components)
  {
    if (auto* attribute = dynamic_cast<smtk::attribute::Attribute*>(component.get()))
    {
      auto it = m_watching.find(attribute->id());
      if (it == m_watching.end())
      {
        continue;
      }
      for (const auto& ref : it->second)
      {
        if (!expunging)
        {
          parametersToUpdate[ref].insert(attribute);
        }
        else
        {
          ParameterSpecRef expungeRef = ref;
          expungeRef.m_expunge = true;
          parametersToUpdate[expungeRef].insert(attribute);
        }
      }
    }
  }
  return true; // FIXME
}

bool SubmitOperationAgent::copyToParameters(ParameterUpdateMap& parametersToUpdate)
{
  bool didModify = false;
  for (const auto& entry : parametersToUpdate)
  {
    didModify |= this->copyToParameter(entry.first, entry.second);
  }
  return didModify;
}

bool SubmitOperationAgent::copyToParameter(
  const ParameterSpecRef& ref,
  const std::unordered_set<smtk::resource::PersistentObject*>& objects)
{
  // Look up the referenced parameter and exit early if it does not exist.
  auto portIt = m_parameterSpecs.find(ref.m_portName);
  if (portIt == m_parameterSpecs.end())
  {
    return false;
  }
  auto roleIt = portIt->second.find(ref.m_roleName);
  if (roleIt == portIt->second.end())
  {
    return false;
  }
  if (ref.m_specIndex >= roleIt->second.size())
  {
    return false;
  }

  // TODO: Check ref.m_expunge and remove \a objects if true.

  const ParameterSpec& spec(roleIt->second[ref.m_specIndex]);
  if (spec.m_configuredBy == ConfiguredBy::User && spec.m_userOverride)
  {
    // User has specified a value manually; do not overwrite it.
    return false;
  }
  auto targetItem = m_operation->parameters()->itemAtPath(spec.m_targetItemPath);
  if (!targetItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Bad item path \"" << spec.m_targetItemPath
                         << "\""
                            " in \""
                         << m_operation->parameters()->type() << "\".");
    return false;
  }

  if (auto* targetInt = dynamic_cast<smtk::attribute::IntItem*>(targetItem.get()))
  {
    return this->copyToIntParameter(spec, targetInt, objects);
  }
  else if (auto* targetDouble = dynamic_cast<smtk::attribute::DoubleItem*>(targetItem.get()))
  {
    return this->copyToDoubleParameter(spec, targetDouble, objects);
  }
  else if (auto* targetString = dynamic_cast<smtk::attribute::StringItem*>(targetItem.get()))
  {
    return this->copyToStringParameter(spec, targetString, objects);
  }
  else if (auto* targetReference = dynamic_cast<smtk::attribute::ReferenceItem*>(targetItem.get()))
  {
    std::size_t objIdx = targetReference->numberOfValues();
    switch (spec.m_portDataHandler)
    {
      case PortDataHandler::SetObjects:
        targetReference->reset();
        objIdx = 0; // Always start at 0 with SetObjects.
        // Fall through.
      case PortDataHandler::AddObjects:
      {
        targetReference->setNumberOfValues(objIdx + objects.size());
        for (const auto& object : objects)
        {
          if (targetReference->setValue(objIdx, object->shared_from_this()))
          {
            ++objIdx;
          }
        }
        targetReference->setNumberOfValues(objIdx);
        return true;
      }
      break;
      default:
      case PortDataHandler::AssignFromAttribute:
      case PortDataHandler::AssignFromAttributeResource:
      case PortDataHandler::AssignMatchingAttributes:
        return this->copyToReferenceParameter(spec, targetReference, objects);
    }
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(), "Unhandled parameter type \"" << targetItem->typeName() << "\".");
  return false;
}

bool SubmitOperationAgent::copyToIntParameter(
  const ParameterSpec& spec,
  smtk::attribute::IntItem* targetInt,
  const ObjectSet& objects)
{
  bool didModify = false;
  if (objects.empty())
  {
    // TODO: More thorough check that targetItem was modified?
    // TODO: Disable targetItem if it is optional and we provide no values?
    didModify = targetInt->numberOfValues() > 0;
    targetInt->setNumberOfValues(0);
    return didModify;
  }
  else if (objects.size() == 1)
  {
    auto* attribute = dynamic_cast<smtk::attribute::Attribute*>(*objects.begin());
    auto fromItem = attribute ? attribute->itemAtPath(spec.m_sourceItemPath) : nullptr;
    smtk::attribute::CopyAssignmentOptions options;
    auto status = targetInt->assign(fromItem, options, smtk::io::Logger::instance());
    if (!status.success())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "ConfigureOperation failed to assign parameter at path \" " << spec.m_targetItemPath
                                                                    << "\"");
    }
    if (status.modified())
    {
      didModify = true;
      // signalItems->appendValue(spec.m_targetItemPath);
    }
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "No support yet for combining item values from multiple items.");
    return false;
  }
  return didModify;
}

bool SubmitOperationAgent::copyToDoubleParameter(
  const ParameterSpec& spec,
  smtk::attribute::DoubleItem* targetDouble,
  const ObjectSet& objects)
{
  bool didModify = false;
  if (objects.empty())
  {
    // TODO: More thorough check that targetItem was modified?
    // TODO: Disable targetItem if it is optional and we provide no values?
    didModify = targetDouble->numberOfValues() > 0;
    targetDouble->setNumberOfValues(0);
    return didModify;
  }
  else if (objects.size() == 1)
  {
    auto* attribute = dynamic_cast<smtk::attribute::Attribute*>(*objects.begin());
    auto fromItem = attribute ? attribute->itemAtPath(spec.m_sourceItemPath) : nullptr;
    smtk::attribute::CopyAssignmentOptions options;
    auto status = targetDouble->assign(fromItem, options, smtk::io::Logger::instance());
    if (!status.success())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "ConfigureOperation failed to assign parameter at path \" " << spec.m_targetItemPath
                                                                    << "\"");
    }
    if (status.modified())
    {
      didModify = true;
      // signalItems->appendValue(spec.m_targetItemPath);
    }
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "No support yet for combining item values from multiple items.");
    return false;
  }
  return didModify;
}

bool SubmitOperationAgent::copyToStringParameter(
  const ParameterSpec& spec,
  smtk::attribute::StringItem* targetString,
  const ObjectSet& objects)
{
  bool didModify = false;
  if (objects.empty())
  {
    // TODO: More thorough check that targetItem was modified?
    // TODO: Disable targetItem if it is optional and we provide no values?
    didModify = targetString->numberOfValues() > 0;
    targetString->setNumberOfValues(0);
    return didModify;
  }
  else if (objects.size() == 1)
  {
    auto* attribute = dynamic_cast<smtk::attribute::Attribute*>(*objects.begin());
    auto fromItem = attribute ? attribute->itemAtPath(spec.m_sourceItemPath) : nullptr;
    smtk::attribute::CopyAssignmentOptions options;
    auto status = targetString->assign(fromItem, options, smtk::io::Logger::instance());
    if (!status.success())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "ConfigureOperation failed to assign parameter at path \" " << spec.m_targetItemPath
                                                                    << "\"");
    }
    if (status.modified())
    {
      didModify = true;
      // signalItems->appendValue(spec.m_targetItemPath);
    }
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "No support yet for combining item values from multiple items.");
    return false;
  }
  return didModify;
}

bool SubmitOperationAgent::copyToReferenceParameter(
  const ParameterSpec& spec,
  smtk::attribute::ReferenceItem* targetReference,
  const ObjectSet& objects)
{
  bool didModify = false;
  if (objects.empty())
  {
    // TODO: More thorough check that targetItem was modified?
    // TODO: Disable targetItem if it is optional and we provide no values?
    didModify = targetReference->numberOfValues() > 0;
    targetReference->setNumberOfValues(0);
    return didModify;
  }
  else if (objects.size() == 1)
  {
    auto* attribute = dynamic_cast<smtk::attribute::Attribute*>(*objects.begin());
    auto fromItem = attribute ? attribute->itemAtPath(spec.m_sourceItemPath) : nullptr;
    smtk::attribute::CopyAssignmentOptions options;
    auto status = targetReference->assign(fromItem, options, smtk::io::Logger::instance());
    if (!status.success())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "ConfigureOperation failed to assign parameter at path \" " << spec.m_targetItemPath
                                                                    << "\"");
    }
    if (status.modified())
    {
      didModify = true;
      // signalItems->appendValue(spec.m_targetItemPath);
    }
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "No support yet for combining item values from multiple items.");
    return false;
  }
  return didModify;
}

State SubmitOperationAgent::computeInternalState()
{
  m_internalState = State::Incomplete;
  if (m_parent->isCompleted() && m_runSinceEdited)
  {
    m_internalState = State::Completable;
    // If the operation was run in a previous session, we must honor that.
    return m_internalState;
  }
  if (!m_operation)
  {
    m_internalState = State::Irrelevant;
    return m_internalState;
  }
  if (m_operation->ableToOperate())
  {
    switch (m_runStyle)
    {
      case RunStyle::Iteratively:
      case RunStyle::Once:
        m_internalState = m_runSinceEdited ? State::Completable : State::Incomplete;
        break;
      case RunStyle::OnCompletion:
        m_internalState = State::Completable;
        break;
    }
  }
  return m_internalState;
}

} // namespace task
} // namespace smtk
