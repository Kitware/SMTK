//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_GroupingGroup_h
#define smtk_operation_GroupingGroup_h

#include "smtk/CoreExports.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/ResourceIOGroup.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

/**\brief A group that holds operations which can group things.
  *
  * All operations added to this group must allow associations to
  * things that they can group.
  *
  * It is best if they (1) have defaults for any parameters and if
  * they have extensible associations
  */
class SMTKCORE_EXPORT GroupingGroup : protected Group
{
public:
  static constexpr const char* const type_name = "group";

  GroupingGroup(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
  {
  }

  using Group::registerOperation;
  using Group::unregisterOperation;

  /// Given objects return an operation that can group it.
  Operation::Index matchingOperation(const smtk::resource::PersistentObject& obj) const;
};
} // namespace operation
} // namespace smtk

#endif // smtk_operation_GroupingGroup_h
