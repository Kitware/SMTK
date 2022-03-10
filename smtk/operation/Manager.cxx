//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/Manager.h"
#include "smtk/operation/ResourceManagerOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/io/Logger.h"

#include <sstream>

namespace smtk
{
namespace operation
{

Manager::Manager()
  : m_metadataObservers([this](MetadataObserver& observer) {
    const auto& allMetadata = m_metadata.get<IndexTag>();
    for (const auto& metadata : allMetadata)
    {
      observer(metadata, true);
    }
  })
{
}

Manager::~Manager() = default;

bool Manager::registerOperation(Metadata&& metadata)
{
  auto alreadyRegisteredMetadata = m_metadata.get<IndexTag>().find(metadata.index());
  if (alreadyRegisteredMetadata == m_metadata.get<IndexTag>().end())
  {
    auto inserted = m_metadata.get<IndexTag>().insert(metadata);
    if (inserted.second)
    {
      m_metadataObservers(*inserted.first, true);
      return true;
    }
  }

  return false;
}

bool Manager::unregisterOperation(const std::string& typeName)
{
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    m_metadata.get<NameTag>().erase(metadata);
    m_metadataObservers(*metadata, false);
    return true;
  }

  return false;
}

bool Manager::unregisterOperation(const Operation::Index& index)
{
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    m_metadata.get<IndexTag>().erase(metadata);
    return true;
  }

  return false;
}

bool Manager::registered(const std::string& typeName) const
{
  const auto metadata = m_metadata.get<NameTag>().find(typeName);
  return metadata != m_metadata.get<NameTag>().end();
}

bool Manager::registered(const Operation::Index& index) const
{
  const auto metadata = m_metadata.get<IndexTag>().find(index);
  return metadata != m_metadata.get<IndexTag>().end();
}

std::shared_ptr<Operation> Manager::create(const std::string& typeName)
{
  std::shared_ptr<Operation> op;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    op = metadata->create();
    op->m_manager = shared_from_this();

    // Parameters are constructed lazily, allowing for RAII while having derived
    // classes construct parameters that are tailored to their use. This can
    // cause a race condition when observers that are called on a different
    // thread access parameters at the same time as the thread that created the
    // operation. Since only managed operations are observed, we can avoid this
    // issue by accessing the parameters as they are created by the manager.
    auto parameters = op->parameters();
  }
  else
  {
    std::ostringstream message;
    message << "Could not find \"" << typeName << "\" operation; considered:\n";
    const auto& byName = m_metadata.get<NameTag>();
    for (auto it = byName.begin(); it != byName.end(); ++it)
    {
      message << "  \"" << it->typeName() << "\"\n";
    }
    smtkInfoMacro(smtk::io::Logger::instance(), message.str());
  }
  if (auto managers = m_managers.lock())
  {
    // Pass application's managers if available.
    op->setManagers(managers);
  }

  return op;
}

std::shared_ptr<Operation> Manager::create(const Operation::Index& index)
{
  std::shared_ptr<Operation> op;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    // Create the resource with the appropriate UUID
    op = metadata->create();
    op->m_manager = shared_from_this();

    // Parameters are constructed lazily, allowing for RAII while having derived
    // classes construct parameters that are tailored to their use. This can
    // cause a race condition when observers that are called on a different
    // thread access parameters at the same time as the thread that created the
    // operation. Since only managed operations are observed, we can avoid this
    // issue by accessing the parameters as they are created by the manager.
    auto parameters = op->parameters();
  }

  return op;
}

bool Manager::registerResourceManager(smtk::resource::ManagerPtr& resourceManager)
{
  // Only allow one resource manager to manage created resources.
  if (m_resourceObserver.assigned())
  {
    this->observers().erase(m_resourceObserver);
  }

  std::weak_ptr<smtk::resource::Manager> weakRMPtr = resourceManager;

  // Define an observer that adds all created resources to the resource manager.
  m_resourceObserver = this->observers().insert(
    [&, weakRMPtr](
      const smtk::operation::Operation& /*unused*/,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result result) {
      auto rsrcManager = weakRMPtr.lock();
      if (!rsrcManager)
      {
        // The underlying resource manager has expired, so we can remove this
        // observer.
        m_observers.erase(m_resourceObserver);
        m_resourceObserver = Observers::Key();
        return 0;
      }

      // We are only interested in collecting resources post-operation
      if (event != smtk::operation::EventType::DID_OPERATE)
      {
        return 0;
      }

      // Gather all resource items
      std::vector<smtk::attribute::ResourceItemPtr> resourceItems;
      std::function<bool(smtk::attribute::ResourceItemPtr)> filter =
        [](smtk::attribute::ResourceItemPtr /*unused*/) { return true; };
      result->filterItems(resourceItems, filter);

      // For each resource item found...
      for (auto& resourceItem : resourceItems)
      {
        // ...for each resource in a resource item...
        for (std::size_t i = 0; i < resourceItem->numberOfValues(); i++)
        {
          // (no need to look at resources that cannot be resolved)
          if (!resourceItem->isValid() || resourceItem->value(i) == nullptr)
          {
            continue;
          }

          // ...add the resource to the manager.
          rsrcManager->add(resourceItem->value(i));
        }
      }
      return 0;
    },
    "Add created resources to the resource manager");

  return m_resourceObserver.assigned();
}

std::set<std::string> Manager::availableOperations() const
{
  std::set<std::string> availableOperations;
  for (const auto& md : m_metadata)
  {
    availableOperations.insert(md.typeName());
  }
  return availableOperations;
}

std::set<Operation::Index> Manager::availableOperations(
  const smtk::resource::ComponentPtr& component) const
{
  std::set<Operation::Index> availableOperations;
  for (const auto& md : m_metadata)
  {
    if (md.acceptsComponent(component))
    {
      availableOperations.insert(md.index());
    }
  }
  return availableOperations;
}

std::set<std::string> Manager::availableGroups() const
{
  std::set<std::string> available;
  for (const auto& md : m_metadata)
  {
    std::set<std::string> operatorGroups = md.groups();
    available.insert(operatorGroups.begin(), operatorGroups.end());
  }
  return available;
}
} // namespace operation
} // namespace smtk
