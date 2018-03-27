//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/Group.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

namespace smtk
{
namespace operation
{

namespace
{
template <typename OperationIdentifier>
struct IdentifierTag;

template <>
struct IdentifierTag<std::string>
{
  typedef NameTag value;
};

template <>
struct IdentifierTag<Operation::Index>
{
  typedef IndexTag value;
};

template <typename OperationIdentifier>
Operation::Specification getSpecification(
  const OperationIdentifier& uniqueId, const std::weak_ptr<smtk::operation::Manager>& managerWkPtr)
{
  auto manager = managerWkPtr.lock();

  // Groups are only defined in the context of operation metadata, which
  // is held by a manager.
  if (!manager)
  {
    return Operation::Specification();
  }

  // Locate the metadata associated with this operator
  auto md =
    manager->metadata().get<typename IdentifierTag<OperationIdentifier>::value>().find(uniqueId);

  return md != manager->metadata().get<typename IdentifierTag<OperationIdentifier>::value>().end()
    ? md->specification()
    : Operation::Specification();
}
}

bool Group::registerOperation(const std::string& typeName, std::set<std::string> values)
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<std::string>(typeName, m_manager);

  // Add the group name to the operator specification's list of groups.
  return spec ? addTag(spec, name(), values) : false;
}

bool Group::registerOperation(const Operation::Index& index, std::set<std::string> values)
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<Operation::Index>(index, m_manager);

  // Add the group name to the operator specification's list of groups.
  return spec ? addTag(spec, name(), values) : false;
}

bool Group::unregisterOperation(const std::string& typeName)
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<std::string>(typeName, m_manager);

  // Remove the group name from the operator specification's list of groups.
  return spec ? removeTag(spec, name()) : false;
}

bool Group::unregisterOperation(const Operation::Index& index)
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<Operation::Index>(index, m_manager);

  // Remove the group name from the operator specification's list of groups.
  return spec ? removeTag(spec, name()) : false;
}

Operation::Specification Group::specification(const std::string& typeName) const
{
  return getSpecification<std::string>(typeName, m_manager);
}

Operation::Specification Group::specification(const Operation::Index& index) const
{
  return getSpecification<Operation::Index>(index, m_manager);
}

bool Group::has(const std::string& typeName) const
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<std::string>(typeName, m_manager);

  // If there is no specification, there are no tags to query.
  if (spec == nullptr)
  {
    return false;
  }

  // Query the specification's set of groups for the tag name.
  std::set<std::string> tagNames = extractTagNames(spec);
  return tagNames.find(name()) != tagNames.end();
}

bool Group::has(const Operation::Index& index) const
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<Operation::Index>(index, m_manager);

  // If there is no specification, there are no tags to query.
  if (spec == nullptr)
  {
    return false;
  }

  // Query the specification's set of tags for the tag name.
  std::set<std::string> tagNames = extractTagNames(spec);
  return tagNames.find(name()) != tagNames.end();
}

std::set<std::string> Group::values(const std::string& typeName) const
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<std::string>(typeName, m_manager);

  // If there is no specification, there are no tags to query.
  if (spec == nullptr)
  {
    return std::set<std::string>();
  }

  return tagValues(spec, name());
}

std::set<std::string> Group::values(const Operation::Index& index) const
{
  // Access the operation's specification.
  Operation::Specification spec = getSpecification<Operation::Index>(index, m_manager);

  // If there is no specification, there are no tags to query.
  if (spec == nullptr)
  {
    return std::set<std::string>();
  }

  return tagValues(spec, name());
}

std::set<Operation::Index> Group::operations() const
{
  std::set<Operation::Index> operationIndices;

  auto manager = m_manager.lock();

  // Groups are only defined in the context of operation metadata, which
  // is held by a manager.
  if (!manager)
  {
    return operationIndices;
  }

  for (auto& md : manager->metadata())
  {
    std::set<std::string> operatorGroups = md.groups();
    if (operatorGroups.find(name()) != operatorGroups.end())
    {
      operationIndices.insert(md.index());
    }
  }
  return operationIndices;
}

std::set<std::string> Group::operationNames() const
{
  std::set<std::string> operationNames;

  auto manager = m_manager.lock();

  // Groups are only defined in the context of operation metadata, which
  // is held by a manager.
  if (!manager)
  {
    return operationNames;
  }

  for (auto& md : manager->metadata())
  {
    std::set<std::string> operatorGroups = md.groups();
    if (operatorGroups.find(name()) != operatorGroups.end())
    {
      operationNames.insert(md.typeName());
    }
  }
  return operationNames;
}
}
}
