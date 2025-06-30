//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/HandleManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/Singletons.h"

#include <map>

namespace smtk
{
namespace view
{

HandleManager::HandleManager() = default;

HandleManager::Ptr HandleManager::instance()
{
  auto& singletons(smtk::common::singletons());
  if (!singletons.contains<smtk::view::HandleManager::Ptr>())
  {
    singletons.insertOrAssign<smtk::view::HandleManager::Ptr>(std::make_shared<HandleManager>());
  }
  return singletons.get<smtk::view::HandleManager::Ptr>();
}

void HandleManager::registerOperationManager(const smtk::operation::Manager::Ptr& opMgr)
{
  if (m_operationManager == opMgr)
  {
    return;
  }

  if (m_operationManager)
  {
    m_operationManager->observers().erase(m_watcher);
  }

  m_operationManager = opMgr;

  if (m_operationManager)
  {
    m_watcher = m_operationManager->observers().insert(
      [this](
        const smtk::operation::Operation& op,
        smtk::operation::EventType event,
        smtk::operation::Operation::Result result) {
        this->handleOperation(op, event, result);
        return 0;
      },
      std::numeric_limits<smtk::operation::Observers::Priority>::max(),
      false,
      "Maintain object handles.");
  }
}

bool HandleManager::typeResolves(
  smtk::string::Token requestedType,
  smtk::string::Token immediateType)
{
  if (requestedType == immediateType)
  {
    return true;
  }
  return this->typeResolvesRecursive(requestedType, immediateType);
}

bool HandleManager::typeResolvesRecursive(
  smtk::string::Token requestedType,
  smtk::string::Token immediateType)
{
  // std::cout << "  Resolve " << immediateType.data() << " to " << requestedType.data() << "\n";
  auto it = m_typeMap.find(immediateType);
  if (it == m_typeMap.end())
  {
    // std::cout << "    No entry for " << immediateType.data() << "\n";
    return false;
  }
  if (it->second == requestedType)
  {
    // std::cout << "    Matched " << immediateType.data() << "\n";
    return true;
  }
  if (it->first == it->second)
  {
    // std::cout << "    No match and " << it->first.data() << " is the base-most class\n";
    return false;
  }
  return this->typeResolvesRecursive(requestedType, it->second);
}

void HandleManager::handleOperation(
  const smtk::operation::Operation& op,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  (void)op;
  if (event == smtk::operation::EventType::WILL_OPERATE)
  {
    return;
  }
  HandlesByType handleGroup;
  for (auto obj : *result->findComponent("expunged"))
  {
    if (auto att = std::dynamic_pointer_cast<smtk::attribute::Attribute>(obj))
    {
      // Invalidate any items that exist in the manager as well.
      std::vector<std::shared_ptr<smtk::attribute::Item>> items;
      att->filterItems(
        items, [](std::shared_ptr<smtk::attribute::Item>) { return true; }, /*only active*/ false);
      for (const auto& item : items)
      {
        auto it = m_objects.find(this->handleString(item.get()));
        if (it == m_objects.end())
        {
          continue;
        }
        handleGroup[it->second.m_type].insert(it->second.m_handle);
      }
    }
    auto it = m_objects.find(this->handleString(obj.get()));
    if (it == m_objects.end())
    {
      continue;
    }
    handleGroup[it->second.m_type].insert(it->second.m_handle);
    m_objects.erase(it);
  }
  for (auto obj : *result->findResource("resourcesToExpunge"))
  {
    auto it = m_objects.find(this->handleString(obj.get()));
    if (it == m_objects.end())
    {
      continue;
    }
    handleGroup[it->second.m_type].insert(it->second.m_handle);
    m_objects.erase(it);
  }
  if (!handleGroup.empty())
  {
    m_observers(handleGroup, EventType::Expunged);
    handleGroup.clear();
  }

  // TODO: We could accept some filters and notify observers when objects
  //       of a particular type are created. It might make maintaining
  //       views easier since some views need to respond to new attributes
  //       being created (for example).
  for (auto obj : *result->findComponent("created"))
  {
    // TODO: Check that handle should be created by manager.
    handleGroup[obj->typeToken()].insert(this->handle(obj.get()));
  }
  for (auto obj : *result->findResource("resourcesCreated"))
  {
    // TODO: Check that handle should be created by manager.
    handleGroup[obj->typeToken()].insert(this->handle(obj.get()));
  }
  if (!handleGroup.empty())
  {
    m_observers(handleGroup, EventType::Created);
    handleGroup.clear();
  }

  for (auto obj : *result->findComponent("modified"))
  {
    if (auto att = std::dynamic_pointer_cast<smtk::attribute::Attribute>(obj))
    {
      // Invalidate any items that exist in the manager as well.
      std::vector<std::shared_ptr<smtk::attribute::Item>> items;
      att->filterItems(
        items, [](std::shared_ptr<smtk::attribute::Item>) { return true; }, /*only active*/ false);
      for (const auto& item : items)
      {
        auto it = m_objects.find(this->handleString(item.get()));
        if (it == m_objects.end())
        {
          continue;
        }
        handleGroup[it->second.m_type].insert(it->second.m_handle);
      }
    }
    auto it = m_objects.find(this->handleString(obj.get()));
    if (it == m_objects.end())
    {
      continue;
    }
    handleGroup[it->second.m_type].insert(it->second.m_handle);
  }
  for (auto obj : *result->findResource("resourcesModified"))
  {
    auto it = m_objects.find(this->handleString(obj.get()));
    if (it == m_objects.end())
    {
      continue;
    }
    handleGroup[it->second.m_type].insert(it->second.m_handle);
  }
  if (!handleGroup.empty())
  {
    m_observers(handleGroup, EventType::Modified);
    handleGroup.clear();
  }
}

} // namespace view
} // namespace smtk
