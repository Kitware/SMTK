//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/FillOutAttributes.h"

#include "smtk/project/ResourceContainer.h"

#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonFillOutAttributes.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"

#include <stdexcept>

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{

namespace
{

Port* fetchPort(
  FillOutAttributes* self,
  const std::string& portName,
  const nlohmann::json& portDict,
  json::Helper& helper)
{
  auto it = portDict.find(portName);
  Port* port = nullptr;
  if (it != portDict.end())
  {
    if (it->is_number_integer())
    {
      port = helper.ports().unswizzle(it->get<int>());
    }
    else
    {
      port = helper.taskManager().portInstances().findById(it->get<smtk::common::UUID>()).get();
    }
  }
  if (port)
  {
    port->setParent(self);
  }
  return port;
}

} // anonymous namespace

constexpr const char* const FillOutAttributes::type_name;

FillOutAttributes::FillOutAttributes() = default;

FillOutAttributes::FillOutAttributes(
  const Configuration& config,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : AgentlessTask(config, taskManager, managers)
  , m_managers(managers)
{
  this->configure(config);
}

FillOutAttributes::FillOutAttributes(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : AgentlessTask(config, dependencies, taskManager, managers)
  , m_managers(managers)
{
  this->configure(config);
}

void FillOutAttributes::configure(const Configuration& config)
{
  // The predicate from_json method needs the resource manager:
  auto& helper = json::Helper::instance();
  helper.setManagers(m_managers);

  auto result = config.find("ports");
  if (result != config.end())
  {
    m_ports["in"] = fetchPort(this, "in", *result, helper);
    m_ports["out"] = fetchPort(this, "out", *result, helper);
  }

  result = config.find("attribute-sets");
  if (result != config.end())
  {
    result->get_to(m_attributeSets);
  }
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
        "FillOutAttributes monitors operations for updates.");
    }
  }
  if (!m_attributeSets.empty())
  {
    if (!this->initializeResources())
    {
      // There were no resources found so the task is unavailable
      this->internalStateChanged(State::Unavailable);
    }
    else
    {
      this->internalStateChanged(this->computeInternalState());
    }
  }
}

const std::unordered_map<smtk::string::Token, Port*>& FillOutAttributes::ports() const
{
  if (m_ports.empty())
  {
    if (this->manager())
    {
      auto pi = this->manager()->portInstances().createFromName(
        "smtk::task::Port",
        { { "name", "in" },
          { "direction", "in" },
          { "data-types", { "smtk::task::ObjectsInRoles" } } },
        m_managers);
      auto po = this->manager()->portInstances().createFromName(
        "smtk::task::Port",
        { { "name", "out" },
          { "direction", "out" },
          { "data-types", { "smtk::task::ObjectsInRoles" } } },
        m_managers);
      auto* self = const_cast<FillOutAttributes*>(this);
      pi->setParent(self);
      po->setParent(self);
      self->m_ports["in"] = pi.get();
      self->m_ports["out"] = po.get();
    }
  }
  return m_ports;
}

std::shared_ptr<PortData> FillOutAttributes::portData(const Port* port) const
{
  auto resourceManager = m_managers->get<smtk::resource::Manager::Ptr>();
  auto it = m_ports.find("out");
  if (it == m_ports.end() || it->second != port || !resourceManager)
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
        data->addObject(rsrc.get(), attSet.m_role);
      }
    }
  }
  return data;
}

void FillOutAttributes::portDataUpdated(const Port* port)
{
  if (m_ports.empty() || m_ports["in"] != port)
  {
    return;
  }
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
        }
        if (it == roleMap.end())
        {
          continue;
        }
        // There is a matching role. Are the objects in that role attribute resources?
        for (auto* obj : it->second)
        {
          if (auto* resource = dynamic_cast<smtk::attribute::Resource*>(obj))
          {
            auto rit = attSet.m_resources.insert({ resource->id(), { {}, {} } }).first;
            stateMayHaveChanged |= this->updateResourceEntry(*resource, attSet, rit->second);
          }
        }
      }
    }
  }
  if (stateMayHaveChanged)
  {
    this->internalStateChanged(this->computeInternalState());
  }
}

bool FillOutAttributes::getViewData(smtk::common::TypeContainer& configuration) const
{
  using ResourceSet = std::
    set<smtk::attribute::Resource::WeakPtr, std::owner_less<smtk::attribute::Resource::WeakPtr>>;

  bool didChange = false;
  auto resourceManager = m_managers->get<smtk::resource::Manager::Ptr>();
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
  this->visitAttributeSets(
    [&didChange, &resourceManager, &previous, &viewData](const AttributeSet& attributeSet) {
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
      return smtk::common::Visit::Continue;
    });
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

smtk::common::Visit FillOutAttributes::visitAttributeSets(ConstAttributeSetVisitor visitor) const
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }
  for (const auto& entry : m_attributeSets)
  {
    if (visitor(entry) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
  }
  return smtk::common::Visit::Continue;
}

smtk::common::Visit FillOutAttributes::visitAttributeSets(AttributeSetVisitor visitor)
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }
  for (auto& entry : m_attributeSets)
  {
    if (visitor(entry) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
  }
  return smtk::common::Visit::Continue;
}

bool FillOutAttributes::initializeResources()
{
  // By default, assume we are unconfigured:
  bool foundResource = false;
  if (m_attributeSets.empty())
  {
    return foundResource;
  }

  if (auto resourceManager = m_managers->get<smtk::resource::Manager::Ptr>())
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

bool FillOutAttributes::updateResourceEntry(
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
        "Can not find required Attribute: " << attName << " in Resource:" << resource.name());
    }
    else
    {
      changesMade |= testValidity(attribute, entry);
    }
  }
  return changesMade;
}

bool FillOutAttributes::testValidity(
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

int FillOutAttributes::update(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  (void)op;
  bool predicatesUpdated = false;
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
    this->internalStateChanged(this->computeInternalState());
  }
  return 0;
}

bool FillOutAttributes::hasRelevantInformation(
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
State FillOutAttributes::computeInternalState() const
{
  auto resourceManager = m_managers->get<smtk::resource::Manager::Ptr>();
  if (!resourceManager)
  {
    return this->internalState(); // Can not find a resource manager so we can't recompute state
  }

  bool foundResources;
  if (!this->hasRelevantInformation(resourceManager, foundResources))
  {
    if (foundResources)
    {
      return State::Irrelevant;
    }
    return State::Unavailable;
  }
  for (const auto& predicate : m_attributeSets)
  {
    for (const auto& resourceEntry : predicate.m_resources)
    {
      if (!resourceEntry.second.m_invalid.empty())
      {
        return State::Incomplete;
      }
    }
  }
  return State::Completable;
}

} // namespace task
} // namespace smtk
