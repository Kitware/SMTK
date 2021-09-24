//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/utility/Queries.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Manager.h"

using namespace smtk::attribute;

namespace smtk
{
namespace attribute
{
namespace utility
{
std::set<smtk::resource::PersistentObjectPtr> checkUniquenessCondition(
  const ComponentItemPtr& compItem,
  const std::set<smtk::resource::PersistentObjectPtr>& objSet)
{
  // Uniqueness condition only applies to component items (not resource items)
  if (compItem == nullptr)
  {
    return objSet;
  }

  auto theAttribute = compItem->attribute();
  auto attResource = theAttribute->attributeResource();
  auto compDef = compItem->definitionAs<ComponentItemDefinition>();
  auto role = compDef->role();
  // If the role involved is not unique then just return the original set
  if (!attResource->isRoleUnique(role))
  {
    return objSet;
  }

  std::set<smtk::resource::PersistentObjectPtr> result;
  for (const auto& obj : objSet)
  {
    auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
    if ((comp != nullptr) && compItem->isValueValid(comp))
    {
      result.insert(comp);
    }
    else if (comp != nullptr)
    {
      auto otherAtt = attResource->findAttribute(comp, compDef->role());
      if (otherAtt == theAttribute)
      {
        result.insert(comp);
      }
#if !defined(NDEBUG)
      else
      {
        std::cerr << "attribute::utility::checkUniquenessCondition - comp:" << comp->name()
                  << " is not allowed due to: " << otherAtt->name() << std::endl;
      }
#endif
    }
  }
  return result;
}

std::set<smtk::resource::PersistentObjectPtr> associatableObjects(
  const ConstReferenceItemDefinitionPtr& refItemDef,
  smtk::attribute::ResourcePtr& attResource,
  smtk::resource::ManagerPtr& resManager,
  const smtk::common::UUID& ignoreResource)
{
  std::set<smtk::resource::PersistentObjectPtr> candidates;

  // if there is no reference Item Def, then there are no candidates
  if (refItemDef == nullptr)
  {
    return candidates;
  }

  auto assocMap = refItemDef->acceptableEntries();

  // Are we dealing with the case where the attribute resource has resources directly associated
  // with it (or if we don't have a resource manager)
  if (attResource->hasAssociations() || (resManager == nullptr))
  {
    auto resources = attResource->associations();
    // we should always consider the attribute resource itself as well
    resources.insert(attResource);
    // Iterate over the acceptable entries
    decltype(assocMap.equal_range("")) range;
    for (auto i = assocMap.begin(); i != assocMap.end(); i = range.second)
    {
      // Get the range for the current key
      range = assocMap.equal_range(i->first);

      // Lets see if any of the resources match this type
      for (const auto& resource : resources)
      {
        if (resource->id() == ignoreResource)
        {
          continue;
        }

        if (resource->isOfType(i->first))
        {
          // We need to find all of the component types for
          // this resource.  If a string is empty then the resource
          // itself can be associated with the attribute
          for (auto j = range.first; j != range.second; ++j)
          {
            if (j->second.empty() && refItemDef->isValueValid(resource))
            {
              candidates.insert(resource);
            }
            else
            {
              auto comps = resource->find(j->second);
              for (auto comp = comps.begin(); comp != comps.end(); ++comp)
              {
                if (*comp && refItemDef->isValueValid(*comp))
                {
                  candidates.insert(*comp);
                }
              }
              //candidates.insert(comps.begin(), comps.end());
            }
          }
        }
      }
    }
  }
  else // we need to use the resource manager
  {
    decltype(assocMap.equal_range("")) range;
    for (auto i = assocMap.begin(); i != assocMap.end(); i = range.second)
    {
      // Get the range for the current key
      range = assocMap.equal_range(i->first);

      // As the resource manager to get all appropriate resources
      auto resources = resManager->find(i->first);
      // Need to process all of these resources
      for (const auto& resource : resources)
      {
        if (resource->id() == ignoreResource)
        {
          continue;
        }
        // We need to find all of the component types for
        // this resource.  If a string is empty then the resource
        // itself can be associated with the attribute
        for (auto j = range.first; j != range.second; ++j)
        {
          if (j->second.empty() && refItemDef->isValueValid(resource))
          {
            candidates.insert(resource);
          }
          else
          {
            auto comps = resource->find(j->second);
            for (auto comp = comps.begin(); comp != comps.end(); ++comp)
            {
              if (*comp && refItemDef->isValueValid(*comp))
              {
                candidates.insert(*comp);
              }
            }
          }
        }
      }
    }
  }
  return candidates;
}

std::set<smtk::resource::PersistentObjectPtr> associatableObjects(
  const ReferenceItemPtr& refItem,
  smtk::resource::ManagerPtr& resManager,
  bool useAttributeAssociations,
  const smtk::common::UUID& ignoreResource)
{
  std::set<smtk::resource::PersistentObjectPtr> candidates;

  if (refItem == nullptr)
  {
    return candidates;
  }

  auto theAttribute = refItem->attribute();

  if (theAttribute == nullptr)
  {
    return candidates;
  }

  auto attResource = theAttribute->attributeResource();
  auto compItem = std::dynamic_pointer_cast<ComponentItem>(refItem);
  // There are 3 possible sources of Persistent Objects:
  // 1. Those associated with the attribute this refItem is a member of
  // 2. Based on the resources associated with the attribute resource the refItem's attribute
  // is a component of
  // 3. The resources contained in the resource manager associated with the UIManager
  if (useAttributeAssociations)
  {
    // We must access elements of the association carefully, since this method could be called
    // in the middle of a resource's removal logic. By accessing the associations' keys
    // instead of the associations themselves, we avoid triggering the association's
    // resolve() method (which will attempt to read in the resource being removed).
    auto associations = theAttribute->associations();
    for (std::size_t i = 0; i < associations->numberOfValues(); ++i)
    {
      if (!associations->isSet(i))
      {
        continue;
      }
      ReferenceItem::Key key = theAttribute->associations()->objectKey(i);
      auto& surrogate = theAttribute->resource()->links().data().value(key.first);
      if (surrogate.id() != ignoreResource)
      {
        if (auto object = associations->value(i))
        {
          candidates.insert(object);
        }
      }
    }
    if (compItem == nullptr)
    {
      // There is no possible uniqueness condition to check
      return candidates;
    }
    return checkUniquenessCondition(compItem, candidates);
  }

  auto refItemDef = refItem->definitionAs<ReferenceItemDefinition>();
  candidates = associatableObjects(refItemDef, attResource, resManager, ignoreResource);
  if (compItem == nullptr)
  {
    // There is no possible uniqueness condition to check
    return candidates;
  }
  return checkUniquenessCondition(compItem, candidates);
}

smtk::attribute::ResourcePtr findResourceContainingDefinition(
  const std::string& defType,
  smtk::attribute::ResourcePtr& sourceAttResource,
  smtk::resource::ManagerPtr& resManager,
  const smtk::common::UUID& ignoreResource)
{
  if (defType.empty())
  {
    return nullptr; // No definition was given
  }

  // Are we dealing with a source attribute resource?
  if (sourceAttResource)
  {
    // Does it contain the definition?
    if (sourceAttResource->findDefinition(defType))
    {
      return sourceAttResource;
    }
    // Are there Resources associated with the sourceAttResource?
    if (sourceAttResource->hasAssociations())
    {
      auto resources = sourceAttResource->associations();
      // Lets see if any of the resources are attribute resources
      for (const auto& resource : resources)
      {
        if (resource->id() == ignoreResource)
        {
          continue;
        }
        smtk::attribute::ResourcePtr attRes =
          std::dynamic_pointer_cast<smtk::attribute::Resource>(resource);
        if (attRes && attRes->findDefinition(defType))
        {
          return attRes;
        }
      }
    }
  }
  if (resManager == nullptr) // There is no other place to search
  {
    return nullptr;
  }
  // Get all of the Attribute Resources stored in the Manager
  auto managedResources = resManager->find(smtk::common::typeName<attribute::Resource>());
  for (const auto& resource : managedResources)
  {
    if (resource->id() == ignoreResource)
    {
      continue;
    }
    smtk::attribute::ResourcePtr attRes =
      std::dynamic_pointer_cast<smtk::attribute::Resource>(resource);
    if (attRes && attRes->findDefinition(defType))
    {
      return attRes;
    }
  }
  // Couldn't find it
  return nullptr;
}
} // namespace utility
} // namespace attribute
} // namespace smtk
