//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ResourceIOGroup_h
#define smtk_operation_ResourceIOGroup_h

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

class SMTKCORE_EXPORT ResourceIOGroup : protected Group
{
public:
  using Group::contains;
  using Group::operations;
  using Group::operationNames;
  using Group::unregisterOperation;

  ResourceIOGroup(const std::string& name, std::shared_ptr<smtk::operation::Manager> manager)
    : Group(name, manager)
    , m_fileItemName(manager)
  {
  }

  // Read, import and export groups require an associated file. Write groups do
  // not, since resources have a location field. This method is therefore
  // overridden for the writer group.
  virtual bool requiresFileItem() const { return true; }

  /// Register an IO operation identified by it's unique name, the type of
  /// resource it handles and the file item name.
  template <typename ResourceType>
  bool registerOperation(
    const std::string&, const std::string& fileItemName = m_defaultFileItemName);

  /// Register an IO operation identified by its type index, the type of
  /// resource it handles and the file item name.
  template <typename ResourceType>
  bool registerOperation(
    const Operation::Index&, const std::string& fileItemName = m_defaultFileItemName);

  /// Register an IO operation identified by its class type, the name of the
  /// resource it reads and the file item name.
  template <typename ResourceType, typename OperationType>
  bool registerOperation(const std::string& fileItemName = m_defaultFileItemName);

  /// Obtain the file item name associated with an operation identified by its
  /// unique name.
  std::string fileItemNameForOperation(const std::string&) const;

  /// Obtain the file item name associated with the operation identified by its
  /// type index.
  std::string fileItemNameForOperation(const Operation::Index&) const;

  /// Obtain the file item definition associated with an operation identified
  /// by its unique name.
  smtk::attribute::FileItemDefinition::Ptr fileItemDefinitionForOperation(const std::string&) const;

  /// Obtain the file item definition associated with the operation identified
  /// by its type index.
  smtk::attribute::FileItemDefinition::Ptr fileItemDefinitionForOperation(
    const Operation::Index&) const;

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

  /// Obtain the file item associated with the operation identified by its
  /// class type.
  template <typename OperationType>
  std::string fileItemNameForOperation() const;

protected:
  class FileItemName : public smtk::operation::Group
  {
  public:
    static constexpr const char* const type_name = "fileItemName";

    FileItemName(std::shared_ptr<smtk::operation::Manager> manager)
      : Group(type_name, manager)
    {
    }
  };

  FileItemName m_fileItemName;
  static const std::string m_defaultFileItemName;
};

template <typename ResourceType>
bool ResourceIOGroup::registerOperation(
  const std::string& typeName, const std::string& fileItemName)
{
  if (this->requiresFileItem())
  {
    // Check that the file item name corresponds to a file item, and then register
    // the resource type and file item name to the operation.
    Operation::Specification spec = specification(typeName);
    if (spec == nullptr)
    {
      return false;
    }

    Operation::Parameters parameters = extractParameters(spec, typeName);

    if (parameters == nullptr || parameters->findFile(fileItemName) == nullptr)
    {
      return false;
    }
  }
  return (m_fileItemName.registerOperation(typeName, { fileItemName }) &&
    Group::registerOperation(typeName, { smtk::common::typeName<ResourceType>() }));
}

template <typename ResourceType>
bool ResourceIOGroup::registerOperation(
  const Operation::Index& index, const std::string& fileItemName)
{
  if (this->requiresFileItem())
  {
    // Check that the file item name corresponds to a file item, and then register
    // the resource type and file item name to the operation.
    auto manager = m_manager.lock();
    if (manager == nullptr)
    {
      return false;
    }

    auto metadata = manager->metadata().get<IndexTag>().find(index);
    if (metadata == manager->metadata().get<IndexTag>().end())
    {
      return false;
    }

    Operation::Specification spec = specification(metadata->typeName());
    if (spec == nullptr)
    {
      return false;
    }

    Operation::Parameters parameters = extractParameters(spec, metadata->typeName());

    if (parameters == nullptr || parameters->findFile(fileItemName) == nullptr)
    {
      return false;
    }
  }
  return (m_fileItemName.registerOperation(index, { fileItemName }) &&
    Group::registerOperation(index, { smtk::common::typeName<ResourceType>() }));
}

template <typename ResourceType, typename OperationType>
bool ResourceIOGroup::registerOperation(const std::string& fileItemName)
{
  if (this->requiresFileItem())
  {
    // Check that the file item name corresponds to a file item, and then register
    // the resource type and file item name to the operation.
    auto manager = m_manager.lock();
    if (manager == nullptr)
    {
      return false;
    }

    auto metadata =
      manager->metadata().get<IndexTag>().find(std::type_index(typeid(OperationType)).hash_code());
    if (metadata == manager->metadata().get<IndexTag>().end())
    {
      return false;
    }

    Operation::Specification spec = specification(metadata->typeName());
    if (spec == nullptr)
    {
      return false;
    }

    Operation::Parameters parameters = extractParameters(spec, metadata->typeName());

    if (parameters == nullptr || parameters->findFile(fileItemName) == nullptr)
    {
      return false;
    }
  }
  return (m_fileItemName.registerOperation(
            std::type_index(typeid(OperationType)).hash_code(), { fileItemName }) &&
    Group::registerOperation(std::type_index(typeid(OperationType)).hash_code(),
            { smtk::common::typeName<ResourceType>() }));
}

template <typename OperationType>
std::string ResourceIOGroup::fileItemNameForOperation() const
{
  return fileItemNameForOperation(std::type_index(typeid(OperationType)).hash_code());
}

template <typename ResourceType>
std::set<Operation::Index> ResourceIOGroup::operationsForResource() const
{
  return operationsForResource(smtk::common::typeName<ResourceType>());
}
}
}

#endif // smtk_operation_ResourceIOGroup_h
