//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Logger.h"

#include "smtk/graph/operators/DeleteArc.h"
#include "smtk/graph/operators/DeleteArc_xml.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/operation/groups/ArcDeleter.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/StringUtil.h"

#include <sstream>

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

using namespace smtk::string::literals;

namespace smtk
{
namespace graph
{
namespace // anonymous
{

/// Hold on to supported types since we cannot control when
/// arc types are registered relative to the creation of
/// out specification instance(s).
///
/// (If an operation manager is used, the specification will
/// always pre-exist; if not, then each operation will create
/// a new specification before running.)
std::unordered_map<
  smtk::string::Token,
  std::unordered_map<smtk::string::Token, DeleteArc::EndpointTypes>>
  s_arcTypes;

/// Add "accepts" entries to \a item.
bool addAcceptEntries(
  const smtk::attribute::ReferenceItemDefinitionPtr& item,
  smtk::string::Token resourceFilter,
  const std::unordered_set<smtk::string::Token>& nodeTypes)
{
  if (!item)
  {
    return false;
  }
  std::string resourceClause(resourceFilter.valid() ? resourceFilter.data() : "*");
  const auto& acceptable = item->acceptableEntries();
  auto lower = acceptable.lower_bound(resourceClause);
  auto upper = acceptable.upper_bound(resourceClause);
  if (!nodeTypes.empty())
  {
    for (const auto& nodeFilter : nodeTypes)
    {
      bool preExists = false;
      for (auto it = lower; it != upper; ++it)
      {
        if (it->second == nodeFilter)
        {
          preExists = true;
          break;
        }
      }
      if (!preExists)
      {
        item->setAcceptsEntries(resourceClause, nodeFilter.data(), true);
        lower = acceptable.lower_bound(resourceClause);
        upper = acceptable.upper_bound(resourceClause);
      }
    }
  }
  else
  {
    // Add "*" if it is not present
    bool preExists = false;
    for (auto it = lower; it != upper; ++it)
    {
      if (it->second == "*")
      {
        preExists = true;
        break;
      }
    }
    if (!preExists)
    {
      item->setAcceptsEntries(resourceClause, "*", true);
    }
  }
  return true;
}

} // anonymous namespace

DeleteArc::DeleteArc() = default;

#if 0
bool DeleteArc::ableToOperate()
{
  bool ok = this->Superclass::ableToOperate();

  smtk::string::Token arcTypeName;
  smtk::graph::Component::Ptr from;
  smtk::graph::Component::Ptr to;
  ok &= this->fetchArcTypeAndEndpointsItem(arcTypeName, from, to);

  return ok;
}
#endif

bool DeleteArc::registerDeleter(
  smtk::string::Token arcType,
  const smtk::graph::ResourceBase* resourceExemplar,
  const std::shared_ptr<smtk::operation::Manager>& operationManager)
{
  const auto* arcs =
    resourceExemplar ? resourceExemplar->arcs().at<ArcImplementationBase>(arcType) : nullptr;
  if (!arcs)
  {
    return false;
  }
  std::unordered_set<smtk::string::Token> fnt = arcs->fromTypes();
  std::unordered_set<smtk::string::Token> tnt = arcs->toTypes();

  return DeleteArc::registerDeleter(
    arcType, resourceExemplar->typeToken(), fnt, tnt, operationManager);
}

bool DeleteArc::registerDeleter(
  smtk::string::Token arcType,
  smtk::string::Token resourceFilter,
  const std::unordered_set<smtk::string::Token>& fromTypes,
  const std::unordered_set<smtk::string::Token>& toTypes,
  const std::shared_ptr<smtk::operation::Manager>& operationManager)
{
  bool didRegister = false;
  if (!arcType.valid() || !resourceFilter.valid())
  {
    return didRegister;
  }

  didRegister = true;
  s_arcTypes[arcType][resourceFilter].fromTypes.insert(fromTypes.begin(), fromTypes.end());
  s_arcTypes[arcType][resourceFilter].toTypes.insert(toTypes.begin(), toTypes.end());

  if (operationManager)
  {
    auto index = std::type_index(typeid(DeleteArc)).hash_code();
    auto meta = operationManager->metadata().get<smtk::operation::IndexTag>().find(index);
    if (meta != operationManager->metadata().get<smtk::operation::IndexTag>().end())
    {
      didRegister &= DeleteArc::updateSpecification(
        meta->specification(), arcType, resourceFilter, EndpointTypes{ fromTypes, toTypes });
    }

    smtk::operation::ArcDeleter deleterGroup(operationManager);
    didRegister &= deleterGroup.registerOperation<DeleteArc>();
  }
  return didRegister;
}

bool DeleteArc::fetchArcTypeAndEndpointsItem(
  smtk::string::Token& arcTypeName,
  smtk::attribute::GroupItem::Ptr& endpoints)
{
  bool ok = false;
  auto arcTypeItem = this->parameters()->findString("arc type");
  if (!arcTypeItem || !arcTypeItem->isDiscrete())
  {
    return ok;
  }
  int arcTypeIdx = arcTypeItem->discreteIndex();
  if (arcTypeIdx < 0)
  {
    return ok;
  }

  std::size_t nn = arcTypeItem->numberOfActiveChildrenItems();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    endpoints = std::dynamic_pointer_cast<smtk::attribute::GroupItem>(
      arcTypeItem->activeChildItem(static_cast<int>(ii)));
    if (endpoints)
    {
      if (smtk::common::StringUtil::endsWith(endpoints->name(), "endpoints"))
      {
        // Stop at the first match.
        break;
      }
    }
  }

  if (endpoints)
  {
    arcTypeName =
      arcTypeItem->definitionAs<smtk::attribute::StringItemDefinition>()->discreteValue(arcTypeIdx);
    ok = true;
  }
  return ok;
}

DeleteArc::Result DeleteArc::operateInternal()
{
  smtk::string::Token arcTypeName;
  smtk::attribute::GroupItem::Ptr endpoints;
  std::set<smtk::graph::Component::Ptr> modified;
  if (!this->fetchArcTypeAndEndpointsItem(arcTypeName, endpoints))
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Now iterate over group entries deleting arcs:
  bool ok = true;
  std::size_t nn = endpoints->numberOfGroups();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto fromNode = std::dynamic_pointer_cast<smtk::graph::Component>(
      endpoints->findAs<smtk::attribute::ReferenceItem>(ii, "from")->value());
    auto toNode = std::dynamic_pointer_cast<smtk::graph::Component>(
      endpoints->findAs<smtk::attribute::ReferenceItem>(ii, "to")->value());
    if (!fromNode || !toNode)
    {
      continue; // TODO: Warn? ok = false?
    }
    auto api = fromNode->outgoing(arcTypeName);
    if (!api.valid())
    {
      continue; // TODO: Warn? ok = false?
    }
    if (api.disconnect(toNode))
    {
      modified.insert(fromNode);
      modified.insert(toNode);
    }
    else
    {
      ok = false;
      smtkErrorMacro(
        this->log(),
        "Could not disconnect \"" << fromNode->name() << "\" -> \"" << toNode->name() << "\".");
    }
  }

  auto result = this->createResult(
    ok ? smtk::operation::Operation::Outcome::SUCCEEDED
       : smtk::operation::Operation::Outcome::FAILED);
  result->findComponent("modified")->appendValues(modified.begin(), modified.end());
  return result;
}

DeleteArc::Specification DeleteArc::createSpecification()
{
  auto spec = this->Superclass::createSpecification();
  for (const auto& arcTypeEntry : s_arcTypes)
  {
    for (const auto& resourceTypeEntry : arcTypeEntry.second)
    {
      DeleteArc::updateSpecification(
        spec, arcTypeEntry.first, resourceTypeEntry.first, resourceTypeEntry.second);
    }
  }
  return spec;
}

const char* DeleteArc::xmlDescription() const
{
  return DeleteArc_xml;
}

bool DeleteArc::updateSpecification(
  const Specification& spec,
  smtk::string::Token arcType,
  smtk::string::Token resourceFilter,
  const EndpointTypes& endpointTypes)
{
  bool didAdd = false;
  auto paramDef =
    smtk::operation::extractParameterDefinition(spec, smtk::common::typeName<DeleteArc>());
  if (!paramDef)
  {
    return didAdd;
  }
  int dip = paramDef->findItemPosition("arc type");
  if (dip < 0)
  {
    return didAdd;
  }
  auto arcTypeItemDef =
    std::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(paramDef->itemDefinition(dip));
  if (!arcTypeItemDef)
  {
    return didAdd;
  }
  bool didFind = false;
  /// The easy way to determine whether our discrete value is registered:
  /// fetch its conditional items (which we need anyway).
  auto childNamesForValue = arcTypeItemDef->conditionalItems(arcType.data());
  const auto& childItemMap = arcTypeItemDef->childrenItemDefinitions();
  for (const auto& childName : childNamesForValue)
  {
    if (!smtk::common::StringUtil::endsWith(childName, "endpoints"))
    {
      // Skip items that don't match our naming scheme.
      continue;
    }
    auto it = childItemMap.find(childName);
    if (it != childItemMap.end())
    {
      auto groupDef = std::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(it->second);
      if (groupDef && groupDef->isExtensible())
      {
        int fromIdx = groupDef->findItemPosition("from");
        int toIdx = groupDef->findItemPosition("to");
        if (fromIdx >= 0 && toIdx >= 0)
        {
          auto fromDef = std::dynamic_pointer_cast<smtk::attribute::ReferenceItemDefinition>(
            groupDef->itemDefinition(fromIdx));
          auto toDef = std::dynamic_pointer_cast<smtk::attribute::ReferenceItemDefinition>(
            groupDef->itemDefinition(toIdx));
          if (
            addAcceptEntries(fromDef, resourceFilter, endpointTypes.fromTypes) &&
            addAcceptEntries(toDef, resourceFilter, endpointTypes.toTypes))
          {
            didFind = true;
            didAdd = true;
            break;
          }
        }
      }
    }
  }
#if 0
  std::size_t nn = arcTypeItemDef->numberOfDiscreteValues();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (arcTypeItemDef->discreteValue(ii) == arcType)
    {
      xxx
      // TODO: Verify that the GroupItem's "to" and "from" items
      // are ReferenceItemDefinitions which include the endpointTypes.fromTypes
      // and endpointTypes.toTypes, respectively.
      didFind = true;
      break;
    }
  }
#endif
  if (!didFind)
  {
    // Add the arc type to the discrete values.
    arcTypeItemDef->addDiscreteValue(arcType.data());
    // Create a new GroupItemDefinition for the endpoint items.
    // NB: groupName **must** terminate with "endpoints" per smtk::operation::ArcDeleter.
    std::string groupName(arcType.data() + " endpoints");
    auto groupDef =
      arcTypeItemDef->template addItemDefinition<smtk::attribute::GroupItemDefinition>(groupName);
    if (!groupDef)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not create group for arc type \"" << arcType.data() << "\".");
      return didAdd;
    }
    groupDef->setIsExtensible(true);
    groupDef->setNumberOfRequiredGroups(0);
    // Add "from" and "to" reference-item definitions to the group-item definition and
    // populate them with the list of types.
    auto fromDef =
      groupDef->template addItemDefinition<smtk::attribute::ReferenceItemDefinition>("from");
    didAdd &= addAcceptEntries(fromDef, resourceFilter, endpointTypes.fromTypes);
    auto toDef =
      groupDef->template addItemDefinition<smtk::attribute::ReferenceItemDefinition>("to");
    didAdd &= addAcceptEntries(toDef, resourceFilter, endpointTypes.toTypes);
    // Now add
    didAdd &= arcTypeItemDef->addConditionalItem(arcType.data(), groupName);
  }
  return didAdd;
}

} // namespace graph
} // namespace smtk
