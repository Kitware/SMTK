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
#include "smtk/operation/ResourceManagerOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace operation
{

Manager::Manager()
  : m_observers()
  , m_resourceObserver(-1)
  , m_resourceMetadataObserver(-1)
{
}

Manager::~Manager()
{
}

bool Manager::registerOperator(Metadata&& metadata)
{
  auto alreadyRegisteredMetadata = m_metadata.get<IndexTag>().find(metadata.index());
  if (alreadyRegisteredMetadata == m_metadata.get<IndexTag>().end())
  {
    auto size = m_metadata.get<IndexTag>().size();
    m_metadata.get<IndexTag>().insert(metadata);
    if (m_metadata.get<IndexTag>().size() > size)
    {
      m_metadataObservers(metadata);
      return true;
    }
  }

  return false;
}

std::shared_ptr<NewOp> Manager::create(const std::string& uniqueName)
{
  std::shared_ptr<NewOp> op;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(uniqueName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    op = metadata->create();
    op->m_manager = this;
    m_observers(op, smtk::operation::EventType::CREATED, nullptr);
  }

  return op;
}

std::shared_ptr<NewOp> Manager::create(const NewOp::Index& index)
{
  std::shared_ptr<NewOp> op;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    // Create the resource with the appropriate UUID
    op = metadata->create();
    op->m_manager = this;
    m_observers(op, smtk::operation::EventType::CREATED, nullptr);
  }

  return op;
}

bool Manager::registerResourceManager(smtk::resource::ManagerPtr& resourceManager)
{
  // Only allow one resource manager to manage created resources.
  if (m_resourceObserver != -1)
  {
    this->observers().erase(m_resourceObserver);
  }

  // Use this resource manager to conduct resource manager-related operations
  // (e.g. SaveResource, LoadResource, CreateResource).
  if (m_resourceMetadataObserver != -1)
  {
    this->metadataObservers().erase(m_resourceMetadataObserver);
  }

  std::weak_ptr<smtk::resource::Manager> weakRMPtr = resourceManager;

  // Define a metadata observer that appends the assignment of the resource
  // manager to the create functor for operators that inherit from
  // ResourceManagerOperator.
  auto resourceMetadataObserver = [&, weakRMPtr](const smtk::operation::Metadata& md) {
    auto rsrcManager = weakRMPtr.lock();
    if (!rsrcManager)
    {
      // The underlying resource manager has expired, so we can remove this
      // metadata observer.
      m_metadataObservers.erase(m_resourceMetadataObserver);
      m_resourceMetadataObserver = -1;
      return;
    }

    // We are only interested in operators that inherit from
    // ResourceManagerOperator.
    if (std::dynamic_pointer_cast<ResourceManagerOperator>(md.create()) == nullptr)
    {
      return;
    }

    // This metadata observer actually manipulates the metadata, so we need a
    // const cast. This is an exception to the rule of metadata observers.
    smtk::operation::Metadata& metadata = const_cast<smtk::operation::Metadata&>(md);

    auto create = metadata.create;
    metadata.create = [=]() {
      auto op = create();
      std::dynamic_pointer_cast<ResourceManagerOperator>(op)->setResourceManager(weakRMPtr);
      return op;
    };
  };

  // Apply the metadata observer to extant operator metadata.
  for (auto& md : m_metadata)
  {
    resourceMetadataObserver(md);
  }

  // Add this metadata observer to the set of metadata observers.
  m_resourceMetadataObserver = this->metadataObservers().insert(resourceMetadataObserver);

  // Define an observer that adds all created resources to the resource manager.
  m_resourceObserver =
    this->observers().insert([&, weakRMPtr](std::shared_ptr<smtk::operation::NewOp>,
      smtk::operation::EventType event, smtk::operation::NewOp::Result result) {
      auto rsrcManager = weakRMPtr.lock();
      if (!rsrcManager)
      {
        // The underlying resource manager has expired, so we can remove this
        // observer.
        m_observers.erase(m_resourceObserver);
        m_resourceObserver = -1;
        return 0;
      }

      // We are only interested in collecting resources post-operation
      if (event != smtk::operation::EventType::DID_OPERATE)
      {
        return 0;
      }

      // Gather all resource items
      std::vector<smtk::attribute::ResourceItemPtr> resourceItems;
      std::function<bool(smtk::attribute::ResourceItemPtr)> filter = [](
        smtk::attribute::ResourceItemPtr) { return true; };
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
    });

  return m_resourceObserver != -1;
}

std::set<NewOp::Index> Manager::availableOperators(
  const smtk::resource::ComponentPtr& component) const
{
  std::set<NewOp::Index> availableOperators;
  for (auto& md : m_metadata)
  {
    if (md.acceptsComponent(component))
    {
      availableOperators.insert(md.index());
    }
  }
  return availableOperators;
}
}
}
