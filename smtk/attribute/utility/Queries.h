//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_utility_Queries_h
#define smtk_attribute_utility_Queries_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
namespace utility
{

/// Filter out Resource Components that fail a ComponentItem's Uniqueness Criteria
SMTKCORE_EXPORT
std::set<smtk::resource::PersistentObjectPtr> checkUniquenessCondition(
  const ComponentItemPtr& compItem,
  const std::set<smtk::resource::PersistentObjectPtr>& objSet);

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
SMTKCORE_EXPORT
std::set<smtk::resource::PersistentObjectPtr> associatableObjects(
  const ReferenceItemPtr& refItem,
  smtk::resource::ManagerPtr& resourceManager,
  bool useAttributeAssociations = false,
  const smtk::common::UUID& ignoreResource = smtk::common::UUID::null());
///\brief Get a set of objects that could be assigned based on a reference item definition.
///
/// If ignoreResource is specified the corresponding resource will not participate in determining
/// which objects can be associated. The main use case would be updating the widget because a
/// resource is about to be removed from the system.  Since it is still in memory we needed a way to ignore it.
/// There are 2 possible sources of PersistentObjects:
/// If a resourceManager is not provided or if the Attribute has Resources associated with it
/// then only those Resources will be considered.
/// Else the Resources within the Resource Manager are considered.
SMTKCORE_EXPORT
std::set<smtk::resource::PersistentObjectPtr> associatableObjects(
  const ConstReferenceItemDefinitionPtr& refItemDef,
  smtk::attribute::ResourcePtr& attResource,
  smtk::resource::ManagerPtr& resManager,
  const smtk::common::UUID& ignoreResource = smtk::common::UUID::null());
///\brief Find an Attribute Resource that contains an Attribute Definition type.
///
/// If ignoreResource is specified the corresponding resource will not participate in determining
/// which objects can be associated. The main use case would be updating the widget because a
/// resource is about to be removed from the system.  Since it is still in memory we needed a way to ignore it.
/// There are 3 possible sources of Attribute Resources:
///
/// * The sourceAttResource (if specified) contains the Definition - in that case, it is returned
/// * One of the Attribute Resources associated with the sourceAttResource (if specified), contains the Definition
/// * The resManager (if specified) has an Attribute Resource that contains the Definition
/// The above is also the order of precedence in terms of the search order
SMTKCORE_EXPORT
smtk::attribute::ResourcePtr findResourceContainingDefinition(
  const std::string& defType,
  smtk::attribute::ResourcePtr& sourceAttResource,
  smtk::resource::ManagerPtr& resManager,
  const smtk::common::UUID& ignoreResource = smtk::common::UUID::null());
} // namespace utility
} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_utility_Queries_h
