//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/OperationFactory.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace project
{
bool OperationFactory::registerOperation(const std::string& typeName)
{
  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we check to see if the type is allowed by the manager.
    auto metadata = manager->metadata().get<smtk::operation::NameTag>().find(typeName);
    if (metadata == manager->metadata().get<smtk::operation::NameTag>().end())
    {
      return false;
    }

    // If the type is present in the manager, add it to our whitelist.
    m_types.insert(typeName);
    return true;
  }

  // Otherwise, return false.
  return false;
}

bool OperationFactory::registerOperation(const smtk::operation::Operation::Index& index)
{
  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we check to see if the type is allowed by the manager.
    auto metadata = manager->metadata().get<smtk::operation::IndexTag>().find(index);
    if (metadata == manager->metadata().get<smtk::operation::IndexTag>().end())
    {
      return false;
    }

    // If the type is present in the manager, add it to our whitelist.
    m_types.insert(metadata->typeName());
    return true;
  }

  // Otherwise, return false.
  return false;
}

bool OperationFactory::registerOperations(const std::set<std::string>& typeNames)
{
  bool registered = true;
  for (auto& typeName : typeNames)
  {
    registered &= this->registerOperation(typeName);
  }
  return registered;
}

bool OperationFactory::unregisterOperation(const std::string& typeName)
{
  return m_types.erase(typeName) > 0;
}

bool OperationFactory::unregisterOperation(const smtk::operation::Operation::Index& index)
{
  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we check to see if the type is allowed by the manager.
    auto metadata = manager->metadata().get<smtk::operation::IndexTag>().find(index);
    if (metadata == manager->metadata().get<smtk::operation::IndexTag>().end())
    {
      return false;
    }

    // If the type is present in the manager, remove it from our whitelist.
    return m_types.erase(metadata->typeName()) > 0;
  }

  // Otherwise, return false.
  return false;
}

std::shared_ptr<smtk::operation::Operation> OperationFactory::create(const std::string& typeName)
{
  if (auto manager = m_manager.lock())
  {
    return manager->create(typeName);
  }
  return std::shared_ptr<smtk::operation::Operation>();
}

std::shared_ptr<smtk::operation::Operation> OperationFactory::create(
  const smtk::operation::Operation::Index& index)
{
  if (auto manager = m_manager.lock())
  {
    return manager->create(index);
  }
  return std::shared_ptr<smtk::operation::Operation>();
}

std::set<smtk::operation::Operation::Index> OperationFactory::availableOperations(
  const smtk::resource::ComponentPtr& component) const
{
  std::set<smtk::operation::Operation::Index> indices;

  auto manager = m_manager.lock();
  if (!manager)
  {
    return indices;
  }

  std::set<smtk::operation::Operation::Index> allIndices = manager->availableOperations(component);
  if (m_types.empty())
  {
    return allIndices;
  }

  for (auto& index : allIndices)
  {
    if (
      m_types.find(manager->metadata().get<smtk::operation::IndexTag>().find(index)->typeName()) !=
      m_types.end())
    {
      indices.insert(index);
    }
  }

  return indices;
}
} // namespace project
} // namespace smtk
