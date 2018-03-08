//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ReaderGroup_h
#define smtk_operation_ReaderGroup_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Name.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Operation.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

class SMTKCORE_EXPORT ReaderGroup : private Group
{
public:
  using Group::has;
  using Group::operations;
  using Group::operationNames;

  static constexpr const char* const type_name = "reader";

  ReaderGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
  {
  }

  /// Register a read operation identified by it's unique name and the type of
  /// resource it reads.
  template <typename ResourceType>
  bool registerReader(const std::string&);

  /// Register a read operation identified by its type index and the type of
  /// resource it reads.
  template <typename ResourceType>
  bool registerReader(const Operation::Index&);

  /// Register an operation identified by its class type and the name of the
  /// resource it reads.
  template <typename ResourceType, typename OperationType>
  bool registerReader();

  /// Unregister a read operation identified by it's unique name.
  bool unregisterReader(const std::string& uniqueName)
  {
    return Group::unregisterOperation(uniqueName);
  }

  /// Unegister a read operation identified by its type index.
  bool unregisterReader(const Operation::Index& index) { return Group::unregisterOperation(index); }

  /// Unregister an operation identified by its class type.
  template <typename OperationType>
  bool unregisterReader()
  {
    return Group::unregisterOperation<OperationType>();
  }

  /// Obtain the resource name associated with an operation identified by its
  /// unique name.
  std::string readsResource(const std::string&) const;

  /// Obtain the resource name associated with the operation identified by its
  /// type index.
  std::string readsResource(const Operation::Index&) const;

  /// Obtain the resource name associated with the operation identified by its
  /// class type.
  template <typename OperationType>
  std::string readsResource() const;

  // Obtain the operation associated with the resource name.
  std::shared_ptr<smtk::operation::Operation> readerForResource(const std::string&) const;
};

template <typename ResourceType>
bool ReaderGroup::registerReader(const std::string& uniqueName)
{
  return registerOperation(uniqueName, { smtk::resource::name<ResourceType>() });
}

template <typename ResourceType>
bool ReaderGroup::registerReader(const Operation::Index& index)
{
  return registerOperation(index, { smtk::resource::name<ResourceType>() });
}

template <typename ResourceType, typename OperationType>
bool ReaderGroup::registerReader()
{
  return registerOperation(
    std::type_index(typeid(OperationType)).hash_code(), { smtk::resource::name<ResourceType>() });
}

template <typename OperationType>
std::string ReaderGroup::readsResource() const
{
  return readsResource(std::type_index(typeid(OperationType)).hash_code());
}
}
}

#endif // smtk_operation_Metadata_h
