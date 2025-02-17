//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/FillOutAttributesAgent.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/json/jsonFillOutAttributesAgent.h"

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{
namespace
{

bool hasMatchingDefinitions(
  const smtk::attribute::Resource* resource,
  const FillOutAttributesAgent::AttributeSet& attSet)
{
  return std::any_of(
    attSet.m_definitions.begin(), attSet.m_definitions.end(), [&resource](const std::string& def) {
      return resource->hasDefinition(def);
    });
}

} // anonymous namespace

FillOutAttributesAgent::FillOutAttributesAgent(Task* owningTask)
  : Agent(owningTask)
{
}

State FillOutAttributesAgent::state() const
{
  return m_internalState;
}

void FillOutAttributesAgent::configure(const Configuration& config)
{
  this->Superclass::configure(config);
  auto mgrs = m_parent->managers();
  State prev = m_internalState;
  auto isi = config.find("internal-state");
  if (isi != config.end() && isi->is_string())
  {
    bool isValid;
    State next = smtk::task::stateEnum(isi->get<std::string>(), &isValid);
    if (isValid)
    {
      m_internalState = next;
    }
    else
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Invalid internal-state value \"" << isi->get<std::string>() << "\". Ignoring.");
    }
  }

  m_inputPort = nullptr;
  m_outputPort = nullptr;
  auto ipi = config.find("input-port");
  if (ipi != config.end())
  {
    auto ipname = ipi->get<smtk::string::Token>();
    auto ppi = m_parent->ports().find(ipname);
    m_inputPort = (ppi != m_parent->ports().end()) ? ppi->second : nullptr;
  }
  auto opi = config.find("output-port");
  if (opi != config.end())
  {
    auto opname = opi->get<smtk::string::Token>();
    auto ppi = m_parent->ports().find(opname);
    m_outputPort = (ppi != m_parent->ports().end()) ? ppi->second : nullptr;
  }

  auto result = config.find("attribute-sets");
  if (result != config.end())
  {
    result->get_to(m_attributeSets);
  }
  if (mgrs)
  {
    if (auto operationManager = mgrs->get<smtk::operation::Manager::Ptr>())
    {
      m_observer = operationManager->observers().insert(
        [this](
          const smtk::operation::Operation& op,
          smtk::operation::EventType event,
          smtk::operation::Operation::Result result) { return this->update(op, event, result); },
        /* priority */ 0,
        /* initialize */ true,
        "FillOutAttributesAgent monitors operations for updates.");
    }
  }
  if (!m_attributeSets.empty())
  {
    if (!this->initializeResources())
    {
      // There were no resources found so the task is unavailable
      m_internalState = State::Unavailable;
      m_parent->updateAgentState(this, prev, State::Unavailable);
    }
    else
    {
      m_parent->updateAgentState(this, prev, this->computeInternalState());
    }
  }
  else
  {
    m_parent->updateAgentState(this, prev, this->computeInternalState());
  }
}

Agent::Configuration FillOutAttributesAgent::configuration() const
{
  Configuration jj = this->Superclass::configuration();

  // Always record the current internal state.
  jj["internal-state"] = smtk::task::stateName(m_internalState);

  // Record any configuration specifying attributes to validate.
  if (!m_attributeSets.empty())
  {
    jj["attribute-sets"] = m_attributeSets;
  }

  /// Record port mapping from parent task.
  if (m_inputPort)
  {
    jj["input-port"] = m_inputPort->name();
  }
  if (m_outputPort)
  {
    jj["output-port"] = m_outputPort->name();
  }
  return jj;
}

std::shared_ptr<PortData> FillOutAttributesAgent::portData(const Port* port) const
{
  if (!m_outputPort || port != m_outputPort)
  {
    return nullptr;
  }

  auto mgrs = m_parent->managers();
  auto resourceManager = mgrs ? mgrs->get<smtk::resource::Manager::Ptr>() : nullptr;
  if (!resourceManager)
  {
    return nullptr;
  }

  auto data = std::make_shared<smtk::task::ObjectsInRoles>();
  for (const auto& attSet : m_attributeSets)
  {
    for (const auto& rsrcEntry : attSet.m_resources)
    {
      auto rsrc = resourceManager->get(rsrcEntry.first);
      if (rsrc)
      {
        if (
          attSet.m_outputData == PortDataObjects::Resources ||
          attSet.m_outputData == PortDataObjects::Both)
        {
          data->addObject(rsrc.get(), attSet.m_role);
        }
        if (
          attSet.m_outputData == PortDataObjects::Attributes ||
          attSet.m_outputData == PortDataObjects::Both)
        {
          for (const auto& attId : rsrcEntry.second.m_valid)
          {
            if (auto* att = rsrc->component(attId))
            {
              data->addObject(att, attSet.m_role);
            }
          }
        }
      }
    }
  }
  return data;
}

void FillOutAttributesAgent::portDataUpdated(const Port* port)
{
  // Only respond if the port is our input.
  if (!port || m_inputPort != port)
  {
    return;
  }

  // Reconfigure based on port data.
  State prev = m_internalState;
  bool stateMayHaveChanged = false;
  for (auto* conn : port->connections())
  {
    auto portData = std::dynamic_pointer_cast<ObjectsInRoles>(port->portData(conn));
    const auto& roleMap = portData->data();
    if (portData)
    {
      for (auto& attSet : m_attributeSets)
      {
        // Does the port data provide any resources for this attribute-set's role?
        auto it = roleMap.find(attSet.m_role);
        if (it == roleMap.end())
        { // Accept "unassigned" resources in any role (so that raw resource connections work).
          it = roleMap.find("unassigned"_token);
          if (it == roleMap.end())
          {
            continue;
          }
        }
        // There is a matching role. Are the objects in that role attribute resources?
        for (auto* obj : it->second)
        {
          if (auto* resource = dynamic_cast<smtk::attribute::Resource*>(obj))
          {
            auto result = attSet.m_resources.insert({ resource->id(), { {}, {} } });
            auto rit = result.first;
            stateMayHaveChanged |= this->updateResourceEntry(*resource, attSet, rit->second);
            if (result.second)
            {
              // We actually inserted a new resource.
              // Even if there are no attributes to validate, the state may have changed
              // because there is a matching definition.
              stateMayHaveChanged |= hasMatchingDefinitions(resource, attSet);
            }
          }
        }
      }
    }
  }
  if (stateMayHaveChanged)
  {
    m_parent->updateAgentState(this, prev, this->computeInternalState());
    if (m_outputPort)
    {
      // Propagate changes downstream.
      m_parent->portDataUpdated(m_outputPort);
    }
  }
}

smtk::string::Token FillOutAttributesAgent::PortDataObjectsToken(PortDataObjects value)
{
  switch (value)
  {
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown PortDataObjects value " << static_cast<int>(value) << ".");
      // fall through
    case PortDataObjects::Resources:
      return "smtk::task::FillOutAttributesAgent::PortDataObjects::Resources"_token;
    case PortDataObjects::Attributes:
      return "smtk::task::FillOutAttributesAgent::PortDataObjects::Attributes"_token;
    case PortDataObjects::Both:
      return "smtk::task::FillOutAttributesAgent::PortDataObjects::Both"_token;
  }
}

FillOutAttributesAgent::PortDataObjects FillOutAttributesAgent::PortDataObjectsValue(
  smtk::string::Token token)
{
  switch (token.id())
  {
    default:
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown PortDataObjects token " << token.id() << " \"" << token.data() << "\".");
      // fall through
    case "resources"_hash:
    case "smtk::task::FillOutAttributesAgent::PortDataObjects::Resources"_hash:
      return PortDataObjects::Resources;
    case "attributes"_hash:
    case "smtk::task::FillOutAttributesAgent::PortDataObjects::Attributes"_hash:
      return PortDataObjects::Attributes;
    case "both"_hash:
    case "smtk::task::FillOutAttributesAgent::PortDataObjects::Both"_hash:
      return PortDataObjects::Both;
  }
}

bool FillOutAttributesAgent::getViewData(smtk::common::TypeContainer& configuration) const
{
  using ResourceSet = std::
    set<smtk::attribute::Resource::WeakPtr, std::owner_less<smtk::attribute::Resource::WeakPtr>>;

  bool didChange = false;
  auto resourceManager = m_parent->managers()->get<smtk::resource::Manager::Ptr>();
  if (!resourceManager)
  {
    return didChange;
  }

  std::set<smtk::common::UUID> previous;
  if (configuration.contains<ResourceSet>())
  {
    for (const auto& weakResource : configuration.get<ResourceSet>())
    {
      if (auto resource = weakResource.lock())
      {
        previous.insert(resource->id());
      }
    }
  }

  ResourceSet viewData;
  for (const auto& attributeSet : m_attributeSets)
  {
    for (const auto& entry : attributeSet.m_resources)
    {
      if (
        auto resource =
          std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceManager->get(entry.first)))
      {
        viewData.insert(resource);
        if (previous.find(entry.first) == previous.end())
        {
          didChange = true;
        }
      }
    }
  }
  if (!didChange && previous.size() != viewData.size())
  {
    didChange = true; // previous entries were removed.
  }
  if (didChange)
  {
    configuration.insertOrAssign(viewData);
  }
  return didChange;
}

void FillOutAttributesAgent::taskStateChanged(State, State&) {}

void FillOutAttributesAgent::taskStateChanged(Task*, State, State) {}

bool FillOutAttributesAgent::initializeResources()
{
  // By default, assume we are unconfigured:
  bool foundResource = false;
  if (m_attributeSets.empty())
  {
    return foundResource;
  }

  auto mgrs = m_parent->managers();
  if (auto resourceManager = mgrs ? mgrs->get<smtk::resource::Manager::Ptr>() : nullptr)
  {
    // Iterate attribute sets to see if any are configured with valid
    // attribute resource and/or attribute UUIDs. If so and if we can
    // find the matching resource in the manager, then we can return true.
    bool anyAutoconfigure = false;
    for (const auto& attributeSet : m_attributeSets)
    {
      if (attributeSet.m_autoconfigure)
      {
        anyAutoconfigure = true;
      }
      for (const auto& resourceEntry : attributeSet.m_resources)
      {
        if (auto rsrc = resourceManager->get<smtk::attribute::Resource>(resourceEntry.first))
        {
          foundResource = true;
        }
      }
    }

    // If any attribute sets were marked "autoconfigure" (meaning we should
    // identify any attribute resources with the proper role), then iterate
    // resources in the resource manager and configure as required.
    if (anyAutoconfigure)
    {
      auto resources = resourceManager->find<smtk::attribute::Resource>();
      for (const auto& resource : resources)
      {
        std::string role = smtk::project::detail::role(resource);
        for (auto& attributeSet : m_attributeSets)
        {
          if (
            attributeSet.m_role.empty() || attributeSet.m_role == "*" ||
            attributeSet.m_role == role)
          {
            if (attributeSet.m_autoconfigure)
            {
              foundResource = true;
              auto it = attributeSet.m_resources.insert({ resource->id(), { {}, {} } }).first;
              this->updateResourceEntry(*resource, attributeSet, it->second);
            }
          }
        }
      }
    }
  }
  return foundResource;
}

bool FillOutAttributesAgent::updateResourceEntry(
  smtk::attribute::Resource& resource,
  const AttributeSet& predicate,
  ResourceAttributes& entry)
{
  bool changesMade = false;
  // I. Remove invalid entries for attributes that are valid or deleted.
  std::set<smtk::common::UUID> expunged;
  std::set<smtk::common::UUID> validated;
  std::set<smtk::common::UUID> invalidated;
  for (const auto& invalidId : entry.m_invalid)
  {
    auto att = resource.findAttribute(invalidId);
    if (att)
    {
      if (att->isValid()) // TODO: accept predicate override for categories?
      {
        validated.insert(invalidId);
      }
    }
    else
    {
      expunged.insert(invalidId);
    }
  }
  // II. Check valid attributes to see if they have been invalidated or expunged.
  for (const auto& validId : entry.m_valid)
  {
    auto att = resource.findAttribute(validId);
    if (att)
    {
      if (!att->isValid()) // TODO: accept predicate override for categories?
      {
        invalidated.insert(validId);
      }
    }
    else
    {
      expunged.insert(validId);
    }
  }
  // If the set of invalid attributes was changed, we need to re-run computeInternalState().
  changesMade |= !expunged.empty() || !validated.empty() || !invalidated.empty();
  for (const auto& id : validated)
  {
    entry.m_invalid.erase(id);
    entry.m_valid.insert(id);
  }
  for (const auto& id : expunged)
  {
    entry.m_invalid.erase(id);
  }
  for (const auto& id : invalidated)
  {
    entry.m_invalid.insert(id);
    entry.m_valid.erase(id);
  }
  // II. Check for newly-created attributes
  std::vector<smtk::attribute::AttributePtr> attributes;
  for (const auto& definition : predicate.m_definitions)
  {
    resource.findAttributes(definition, attributes);
    for (const auto& attribute : attributes)
    {
      changesMade |= testValidity(attribute, entry);
    }
  }
  for (const auto& attName : predicate.m_instances)
  {
    auto attribute = resource.findAttribute(attName);
    if (!attribute)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Can not find required attribute: " << attName << " in resource:" << resource.name());
    }
    else
    {
      changesMade |= testValidity(attribute, entry);
    }
  }
  return changesMade;
}

int FillOutAttributesAgent::update(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  (void)op;
  bool predicatesUpdated = false;
  State prev = m_internalState;
  switch (event)
  {
    case smtk::operation::EventType::DID_OPERATE:
    {
      auto categoriesModified = result->findResource("categoriesModified");
      if (categoriesModified && categoriesModified->numberOfValues())
      {
        predicatesUpdated = true; //categories have been changed
      }

      auto mentionedResources = smtk::operation::extractResources(result);
      for (const auto& weakResource : mentionedResources)
      {
        auto resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(weakResource.lock());
        if (resource)
        {
          std::string role = smtk::project::detail::role(resource);
          // Do we care about this resource?
          for (auto& predicate : m_attributeSets)
          {
            auto it = predicate.m_resources.find(resource->id());
            bool doUpdate = false;
            if (it != predicate.m_resources.end())
            {
              doUpdate = true;
            }
            else if (
              predicate.m_role == role || predicate.m_role == "*" || predicate.m_role.empty())
            {
              if (predicate.m_autoconfigure)
              {
                it = predicate.m_resources.insert({ resource->id(), { {}, {} } }).first;
                doUpdate = true;
              }
            }
            if (doUpdate)
            {
              predicatesUpdated |= this->updateResourceEntry(*resource, predicate, it->second);
            }
          }
        }
      }
    }
    break;
    case smtk::operation::EventType::WILL_OPERATE:
      break;
  }
  if (predicatesUpdated)
  {
    m_parent->updateAgentState(this, prev, this->computeInternalState());
  }
  return 0;
}

bool FillOutAttributesAgent::hasRelevantInformation(
  const smtk::resource::ManagerPtr& resourceManager,
  bool& foundResources) const
{
  // If all of the Task's definitions and instances are not relevant then neither is the task
  auto resources = resourceManager->find<smtk::attribute::Resource>();
  foundResources = false;
  for (const auto& attributeSet : m_attributeSets)
  {
    for (const auto& resInfo : attributeSet.m_resources)
    {
      auto attResource =
        std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceManager->get(resInfo.first));
      if (!attResource)
      {
        continue; // Could not find the resource
      }
      foundResources = true;
      for (const auto& defName : attributeSet.m_definitions)
      {
        auto def = attResource->findDefinition(defName);
        // We want to check the definition for relevancy
        if (def && def->isRelevant())
        {
          return true;
        }
      }
      // Need to check the instances
      for (const auto& attName : attributeSet.m_instances)
      {
        auto att = attResource->findAttribute(attName);
        // We want to check the definition for relevancy
        if (att && att->isRelevant())
        {
          return true;
        }
      }
    }
  }
  return false;
}

State FillOutAttributesAgent::computeInternalState()
{
  auto mgrs = m_parent->managers();
  auto resourceManager = mgrs ? mgrs->get<smtk::resource::Manager::Ptr>() : nullptr;
  if (!resourceManager)
  {
    m_internalState = State::Unavailable;
    return m_internalState; // Cannot find a resource manager so we can't recompute state
  }

  bool foundResources;
  if (!this->hasRelevantInformation(resourceManager, foundResources))
  {
    // If we don't have any relevant attributes but we found relevant resources,
    // the task is irrelevant. Otherwise, the task is unavailable.
    m_internalState = foundResources ? State::Irrelevant : State::Unavailable;
    return m_internalState;
  }
  for (const auto& predicate : m_attributeSets)
  {
    for (const auto& resourceEntry : predicate.m_resources)
    {
      if (!resourceEntry.second.m_invalid.empty())
      {
        m_internalState = State::Incomplete;
        return m_internalState;
      }
    }
  }
  m_internalState = State::Completable;
  return m_internalState;
}

bool FillOutAttributesAgent::testValidity(
  const smtk::attribute::AttributePtr& attribute,
  ResourceAttributes& entry)
{
  auto uid = attribute->id();
  if (
    (entry.m_invalid.find(uid) == entry.m_invalid.end()) &&
    (entry.m_valid.find(uid) == entry.m_valid.end()))
  {
    // We've found a new attribute. Classify it.
    if (attribute->isValid()) // TODO: accept predicate override for categories?
    {
      entry.m_valid.insert(uid);
    }
    else
    {
      entry.m_invalid.insert(uid);
    }
    return true;
  }
  return false;
}

} // namespace task
} // namespace smtk
