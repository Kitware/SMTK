//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_NamingGroup_h
#define smtk_operation_NamingGroup_h

#include "smtk/CoreExports.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/operation/groups/ResourceIOGroup.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

/**\brief A group that holds operations which can name Component.
  *
  * All operations added to this group must have a StringItem whose name is "name" and only 1 component association
  *
  */
class SMTKCORE_EXPORT NamingGroup : protected Group
{
public:
  static constexpr const char* const type_name = "naming";

  NamingGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
  {
  }

  template<typename ResourceType, typename OperationType>
  bool registerOperation();

  using Group::unregisterOperation;

  /// Given an object return an operation that can assign a name to it.
  Operation::Index matchingOperation(const smtk::resource::PersistentObject& obj) const;
};

template<typename ResourceType, typename OperationType>
bool NamingGroup::registerOperation()
{
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
  if (
    parameters == nullptr || parameters->findString("name") == nullptr ||
    parameters->associations() == nullptr ||
    parameters->associations()->numberOfRequiredValues() != 1)
  {
    return false;
  }
  return Group::registerOperation(
    std::type_index(typeid(OperationType)).hash_code(), { smtk::common::typeName<ResourceType>() });
}
} // namespace operation
} // namespace smtk

#endif // smtk_operation_NamingGroup_h
