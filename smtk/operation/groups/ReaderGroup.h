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

#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/ResourceIOGroup.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

class SMTKCORE_EXPORT ReaderGroup : public ResourceIOGroup
{
public:
  using ResourceIOGroup::registerOperation;

  static constexpr const char* const type_name = "reader";

  ReaderGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : ResourceIOGroup(type_name, manager)
  {
  }

  /// Register an IO operation identified by the unique names of the resource
  /// and operation and the file item name.
  bool registerOperation(const std::string&, const std::string&,
    const std::string& fileItemName = m_defaultFileItemName);

  /// Obtain the resource names associated with the operation identified by its
  /// class type.
  template <typename OperationType>
  std::set<std::string> readsResources() const;

  // Obtain the operation associated with the resource name.
  std::shared_ptr<smtk::operation::Operation> readerForResource(const std::string&) const;

private:
  /// Obtain the resource names associated with the operation identified by its
  /// type index.
  std::set<std::string> readsResources(const Operation::Index&) const;
};

template <typename OperationType>
std::set<std::string> ReaderGroup::readsResources() const
{
  return readsResources(std::type_index(typeid(OperationType)).hash_code());
}
}
}

#endif // smtk_operation_ReaderGroup_h
