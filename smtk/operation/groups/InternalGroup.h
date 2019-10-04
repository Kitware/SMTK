//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_InternalGroup_h
#define smtk_operation_InternalGroup_h

#include "smtk/CoreExports.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include <string>

namespace smtk
{
namespace operation
{
class Manager;

/// A group for operations that are used internally, and should therefore not be
/// displayed to the user as an available operation.
class SMTKCORE_EXPORT InternalGroup : protected Group
{
public:
  using Group::contains;
  using Group::operations;
  using Group::operationNames;
  using Group::operationName;
  using Group::operationLabel;
  using Group::unregisterOperation;

  static constexpr const char* const type_name = "internal";

  InternalGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
  {
  }

  // Rather than expose the base Group's registerOperation, we construct our own
  // to remove any potential confusion about potentially providing values to the
  // operation.
  bool registerOperation(const std::string&);
  bool registerOperation(const Operation::Index&);
  template <typename OperationType>
  bool registerOperation();
};

template <typename OperationType>
bool InternalGroup::registerOperation()
{
  return Group::registerOperation(std::type_index(typeid(OperationType)).hash_code());
}
}
}

#endif // smtk_operation_InternalGroup_h
