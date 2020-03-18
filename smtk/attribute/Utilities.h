//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_Utilities_h
#define __smtk_attribute_Utilities_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{

///\brief A set of utility functions for attribute based resources.
class SMTKCORE_EXPORT Utilities
{
public:
  /// Filter out Resource Components that fail a ComponentItem's Uniqueness Criteria
  static std::set<smtk::resource::PersistentObjectPtr> checkUniquenessCondition(
    const ComponentItemPtr& compItem, const std::set<smtk::resource::PersistentObjectPtr>& objSet);

  ///\brief Get a set of objects that could be assigned to a reference item.
  ///
  /// If ignoreResource is specified the corresponding resource will not participate in determining
  /// which objects can be associated. The main use case would be updating the widget because a
  /// resource is about to be removed from the system.  Since it is still in memory we needed a way to ignore it.
  /// There are 3 possible sources of PersistentObjects:
  /// If useAttributeAssociations is true then only the objects directly associated with refItem's Attribute will
  /// be considered.
  /// Else If a resourceManager is not provided or if the Resource of the refItem's Attribute has Resources associated with it
  /// then only those Resources will be considered.
  /// Else the Resources within the Resource Manager are considered.
  static std::set<smtk::resource::PersistentObjectPtr> associatableObjects(
    const ReferenceItemPtr& refItem, smtk::resource::ManagerPtr& resourceManager,
    bool useAttributeAssociations = false,
    const smtk::common::UUID& ignoreResource = smtk::common::UUID::null());
};

} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_Utilities_h
