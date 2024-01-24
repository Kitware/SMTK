//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/groups/ArcDeleter.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"

#include "smtk/common/StringUtil.h"

#include <cassert>
#include <limits>

namespace smtk
{
namespace operation
{

bool ArcDeleter::registerOperation(Operation::Index operationIndex)
{
  auto manager = m_manager.lock();
  if (manager == nullptr)
  {
    return false;
  }

  auto metadata = manager->metadata().get<IndexTag>().find(operationIndex);
  if (metadata == manager->metadata().get<IndexTag>().end())
  {
    return false;
  }

  Operation::Specification spec = specification(metadata->typeName());
  if (spec == nullptr)
  {
    return false;
  }

  Operation::Definition parameterDef = extractParameterDefinition(spec, metadata->typeName());

  std::set<std::string> arcTypes;
  if (!ArcDeleter::introspectArcTypes(parameterDef, arcTypes))
  {
    return false;
  }

  // Only register the item name if it is not already registered.
  auto preExisting = this->values(operationIndex);
  preExisting.insert(
    arcTypes.begin(), arcTypes.end()); // Union pre-existing arc types with the new ones (if any).
  return Group::registerOperation(operationIndex, preExisting);
}

std::set<std::string> ArcDeleter::allArcTypes() const
{
  std::set<std::string> arcTypes;
  for (const auto& candidate : this->operations())
  {
    auto candidateArcTypes = this->values(candidate);
    arcTypes.insert(candidateArcTypes.begin(), candidateArcTypes.end());
  }
  return arcTypes;
}

std::set<std::pair<std::string, Operation::Index>> ArcDeleter::allArcDeleters() const
{
  std::set<std::pair<std::string, Operation::Index>> result;
  for (const auto& candidate : this->operations())
  {
    auto candidateArcTypes = this->values(candidate);
    for (const auto& arcType : candidateArcTypes)
    {
      result.emplace(arcType, candidate);
    }
  }
  return result;
}

std::unordered_set<Operation::Index> ArcDeleter::matchingOperations(
  smtk::string::Token arcType,
  const smtk::resource::PersistentObject& from,
  const smtk::resource::PersistentObject& to) const
{
  std::unordered_set<Operation::Index> result;
  auto manager = m_manager.lock();
  if (!manager)
  {
    return result;
  }

  for (const auto& candidate : this->operations())
  {
    auto candidateArcTypes = this->values(candidate);
    if (candidateArcTypes.find(arcType.data()) == candidateArcTypes.end())
    {
      // This operation does not support the requested arc type.
      continue;
    }

    auto metadata = manager->metadata().get<IndexTag>().find(candidate);
    if (metadata == manager->metadata().get<IndexTag>().end())
    {
      continue;
    }

    Operation::Specification spec = specification(metadata->typeName());
    if (!spec)
    {
      continue;
    }

    Operation::Definition parameterDef = extractParameterDefinition(spec, metadata->typeName());

    auto arcTypeDef = this->endpointItemDefs(parameterDef, arcType);
    auto fromRule = arcTypeDef.first;
    if (fromRule && fromRule->isValueValid(from.shared_from_this()))
    {
      auto toRule = arcTypeDef.second;
      if (toRule && toRule->isValueValid(to.shared_from_this()))
      {
        result.insert(candidate);
      }
    }
  }

  return result;
}

bool ArcDeleter::appendArc(
  smtk::operation::Operation& op,
  smtk::string::Token arcType,
  const std::shared_ptr<smtk::resource::PersistentObject>& from,
  const std::shared_ptr<smtk::resource::PersistentObject>& to)
{
  auto arcTypeItem = op.parameters()->findString("arc type");
  if (!arcTypeItem)
  {
    return false;
  }

  if (arcTypeItem->isSet() && arcTypeItem->value() != arcType.data())
  {
    smtkErrorMacro(op.log(), "Operation cannot be configured to delete arcs of multiple types.");
    return false;
  }
  if (!arcTypeItem->setValue(arcType.data()))
  {
    smtkErrorMacro(op.log(), "Could not set arc type to \"" << arcType.data() << "\".");
    return false;
  }

  smtk::attribute::GroupItem::Ptr endpoints;
  std::size_t nn = arcTypeItem->numberOfActiveChildrenItems();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto group =
      std::dynamic_pointer_cast<smtk::attribute::GroupItem>(arcTypeItem->activeChildItem(ii));
    if (group && smtk::common::StringUtil::endsWith(group->name(), "endpoints"))
    {
      endpoints = group;
      break;
    }
  }
  if (!endpoints)
  {
    return false;
  }
  nn = endpoints->numberOfGroups();
  if (!endpoints->appendGroup())
  {
    return false;
  }
  // TODO: Check whether findAs<>() returns null:
  endpoints->findAs<smtk::attribute::ReferenceItem>(nn, "from")->setValue(from);
  endpoints->findAs<smtk::attribute::ReferenceItem>(nn, "to")->setValue(to);
  return true;
}

bool ArcDeleter::introspectArcTypes(
  Operation::Definition parameterDefinition,
  std::set<std::string>& arcTypes)
{
  bool result = false;
  if (!parameterDefinition)
  {
    return result;
  }

  int ii = parameterDefinition->findItemPosition("arc type");
  if (ii < 0)
  {
    return result;
  }
  auto stringDef = std::dynamic_pointer_cast<const smtk::attribute::StringItemDefinition>(
    parameterDefinition->itemDefinition(ii));
  if (!stringDef || !stringDef->isDiscrete())
  {
    return result;
  }

  for (std::size_t ii = 0; ii < stringDef->numberOfDiscreteValues(); ++ii)
  {
    result = true; // We got at least one value.
    arcTypes.insert(stringDef->discreteValue(ii));
  }
  return result;
}

std::pair<
  smtk::attribute::ConstReferenceItemDefinitionPtr,
  smtk::attribute::ConstReferenceItemDefinitionPtr>
ArcDeleter::endpointItemDefs(Operation::Definition parameterDefinition, smtk::string::Token arcType)
  const
{
  using smtk::attribute::ConstReferenceItemDefinitionPtr;
  std::pair<ConstReferenceItemDefinitionPtr, ConstReferenceItemDefinitionPtr> result;

  if (!parameterDefinition)
  {
    return result;
  }

  int ii = parameterDefinition->findItemPosition("arc type");
  if (ii < 0)
  {
    return result;
  }
  auto stringDef = std::dynamic_pointer_cast<const smtk::attribute::StringItemDefinition>(
    parameterDefinition->itemDefinition(ii));
  if (!stringDef || stringDef->findDiscreteIndex(arcType.data()) < 0)
  {
    return result;
  }

  // The map of all the discrete item's children:
  const auto& childDefs = stringDef->childrenItemDefinitions();
  // The names in the map that are active when our arc type is selected:
  auto discreteItemNames = stringDef->conditionalItems(arcType.data());
  for (const auto& discreteItemName : discreteItemNames)
  {
    // Stop at the first matching GroupItemDefinition that produces
    // a valid result.
    if (!smtk::common::StringUtil::endsWith(discreteItemName, "endpoints"))
    {
      // We only accept discrete items whose name ends with "endpoints".
      continue;
    }
    auto it = childDefs.find(discreteItemName);
    // The item must be an immediate child:
    if (it == childDefs.end())
    {
      continue;
    }
    // The item must be a group-item:
    if (
      auto groupDef =
        std::dynamic_pointer_cast<const smtk::attribute::GroupItemDefinition>(it->second))
    {
      if (!groupDef->isExtensible())
      {
        continue;
      }
      int fromIdx = groupDef->findItemPosition("from");
      int toIdx = groupDef->findItemPosition("to");
      if (fromIdx < 0 || toIdx < 0)
      {
        continue;
      }
      auto fromItem = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
        groupDef->itemDefinition(fromIdx));
      auto toItem = std::dynamic_pointer_cast<const smtk::attribute::ReferenceItemDefinition>(
        groupDef->itemDefinition(toIdx));
      if (fromItem && toItem)
      {
        result.first = fromItem;
        result.second = toItem;
        return result;
      }
    }
  }
  return result;
}

} // namespace operation
} // namespace smtk
