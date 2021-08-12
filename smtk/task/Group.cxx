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

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonFillOutAttributes.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Manager.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

FillOutAttributes::FillOutAttributes() = default;

FillOutAttributes::FillOutAttributes(
  const Configuration& config,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, managers)
  , m_managers(managers)
{
  this->configure(config);
}

FillOutAttributes::FillOutAttributes(
  const Configuration& config,
  const PassedDependencies& dependencies,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, dependencies, managers)
  , m_managers(managers)
{
  this->configure(config);
}

void FillOutAttributes::configure(const Configuration& config)
{
  // The predicate from_json method needs the resource manager:
  auto& helper = json::Helper::instance();
  helper.setManagers(m_managers);

  if (config.contains("attribute-sets"))
  {
    config.at("attribute-sets").get_to(m_attributeSets);
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
    this->initializeResources();
    this->internalStateChanged(this->computeInternalState());
  }
}

smtk::common::Visit FillOutAttributes::visitAttributeSets(AttributeSetVisitor visitor)
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

bool FillOutAttributes::initializeResources()
{
  bool foundResource = false;
  if (m_attributeSets.empty())
  {
    return foundResource;
  }
  if (auto resourceManager = m_managers->get<smtk::resource::Manager::Ptr>())
  {
    auto resources = resourceManager->find<smtk::attribute::Resource>();
    for (auto resource : resources)
    {
      const std::string& role = smtk::project::detail::role(resource);
      for (auto& attributeSet : m_attributeSets)
      {
        if (
          attributeSet.m_role.empty() || attributeSet.m_role == "*" || attributeSet.m_role == role)
        {
          foundResource = true;
          auto it = attributeSet.m_resources.insert({ resource->id(), { {}, {} } }).first;
          this->updateResourceEntry(*resource, attributeSet, it->second);
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
      auto uid = attribute->id();
      if (
        (entry.m_invalid.find(uid) == entry.m_invalid.end()) &&
        (entry.m_valid.find(uid) == entry.m_valid.end()))
      {
        // We've found a new attribute. Classify it.
        changesMade = true;
        if (attribute->isValid()) // TODO: accept predicate override for categories?
        {
          entry.m_valid.insert(uid);
        }
        else
        {
          entry.m_invalid.insert(uid);
        }
      }
    }
  }
  return changesMade;
}

int FillOutAttributes::update(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  bool predicatesUpdated = false;
  switch (event)
  {
    case smtk::operation::EventType::DID_OPERATE:
    {
      auto mentionedResources = smtk::operation::extractResources(result);

      for (auto& weakResource : mentionedResources)
      {
        auto resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(weakResource.lock());
        if (resource)
        {
          const std::string& role = smtk::project::detail::role(resource);
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
              it = predicate.m_resources.insert({ resource->id(), { {}, {} } }).first;
              doUpdate = true;
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

State FillOutAttributes::computeInternalState() const
{
  State s = State::Completable;
  for (const auto& predicate : m_attributeSets)
  {
    for (const auto& resourceEntry : predicate.m_resources)
    {
      if (!resourceEntry.second.m_invalid.empty())
      {
        s = State::Incomplete;
        return s;
      }
    }
  }
  return s;
}

} // namespace task
} // namespace smtk
