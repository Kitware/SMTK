//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_CreatorGroup_h
#define smtk_operation_CreatorGroup_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

class SMTKCORE_EXPORT CreatorGroup : protected Group
{
public:
  using Group::has;
  using Group::operations;
  using Group::operationNames;

  static constexpr const char* const type_name = "creator";

  CreatorGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
  {
  }

  /// Register an IO operation identified by it's unique name and the type of
  /// resource it handles.
  template <typename ResourceType>
  bool registerOperation(const std::string&);

  /// Register an IO operation identified by its type index and the type of
  /// resource it handles.
  template <typename ResourceType>
  bool registerOperation(const Operation::Index&);

  /// Register an IO operation identified by its class type and the name of the
  /// resource it reads.
  template <typename ResourceType, typename OperationType>
  bool registerOperation();

  /// Given a resource name, return the set of operators that were associated
  /// with the resource during registration.
  std::set<Operation::Index> operationsForResource(const std::string& resourceName) const;

  /// Given an operation index, return the resource associated with the operation.
  std::string resourceForOperation(const Operation::Index&) const;

  /// Given a resource type, return the set of operators that were associated
  /// with the resource during registration.
  template <typename ResourceType>
  std::set<Operation::Index> operationsForResource() const;

  std::set<std::string> supportedResources() const;
};

template <typename ResourceType>
bool CreatorGroup::registerOperation(const std::string& typeName)
{
  return Group::registerOperation(typeName, { smtk::common::typeName<ResourceType>() });
}

template <typename ResourceType>
bool CreatorGroup::registerOperation(const Operation::Index& index)
{
  return Group::registerOperation(index, { smtk::common::typeName<ResourceType>() });
}

template <typename ResourceType, typename OperationType>
bool CreatorGroup::registerOperation()
{
  return Group::registerOperation(
    std::type_index(typeid(OperationType)).hash_code(), { smtk::common::typeName<ResourceType>() });
}

template <typename ResourceType>
std::set<Operation::Index> CreatorGroup::operationsForResource() const
{
  return operationsForResource(smtk::common::typeName<ResourceType>());
}
}
}

#endif // smtk_operation_CreatorGroup_h
