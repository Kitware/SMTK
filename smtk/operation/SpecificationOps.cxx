//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/Tag.h"

#include <array>

namespace smtk
{
namespace operation
{

Operation::Parameters createParameters(
  Operation::Specification specification,
  const std::string& operatorName,
  const std::string& parametersName)
{
  Operation::Definition parameterDefinition =
    extractParameterDefinition(specification, operatorName);

  if (parameterDefinition != nullptr)
  {
    // Now that we have our operation definition, create our parameters attribute.
    return specification->createAttribute(parametersName, parameterDefinition);
  }

  // If we cannot find the parameter definition, we cannot create the parameters.
  return Operation::Parameters();
}

Operation::Parameters extractParameters(
  Operation::Specification specification,
  const std::string& operatorName)
{
  Operation::Definition parameterDefinition =
    extractParameterDefinition(specification, operatorName);

  if (parameterDefinition != nullptr)
  {
    // Now that we have our operation definition, we access our parameters
    // attribute.

    // Access all parameters created using this definition.
    std::vector<Operation::Parameters> parameters;
    specification->findAttributes(parameterDefinition, parameters);

    if (!parameters.empty())
    {
      // If an instance of our parameters exist, use it.
      return parameters[0];
    }
    else
    {
      // If no instance of our parameters exist, create one.
      return specification->createAttribute(parameterDefinition);
    }
  }
  else
  {
    // If we cannot access the parameter definition, we cannot access the
    // parameters either.
    return Operation::Parameters();
  }
}

Operation::Definition extractParameterDefinition(
  Operation::Specification specification,
  const std::string& operatorName)
{
  Operation::Definition parameterDefinition;

  // Access the base operation definition.
  Operation::Definition operationBase = specification->findDefinition("operation");

  // Find all definitions that derive from the operation definition.
  std::vector<Operation::Definition> parameterDefinitions;
  specification->findAllDerivedDefinitions(operationBase, true, parameterDefinitions);

  // If there is only one derived definition, then it is the one we want.
  if (parameterDefinitions.size() == 1)
  {
    parameterDefinition = parameterDefinitions[0];
  }
  else if (!parameterDefinitions.empty())
  {
    // If there are more than one derived definitions, then we pick the one
    // with the same name as our class.
    for (auto& def : parameterDefinitions)
    {
      if (def->type() == operatorName)
      {
        parameterDefinition = def;
        break;
      }
    }
  }
  return parameterDefinition;
}

Operation::Definition extractResultDefinition(
  Operation::Specification specification,
  const std::string& operatorName)
{
  smtk::attribute::DefinitionPtr resultDefinition;

  // Access the base result definition.
  Operation::Definition resultBase = specification->findDefinition("result");

  // Find all definitions that derive from the result definition.
  std::vector<Operation::Definition> resultDefinitions;
  specification->findAllDerivedDefinitions(resultBase, true, resultDefinitions);

  // If there is only one derived definition, then it is the one we want.
  if (resultDefinitions.size() == 1)
  {
    resultDefinition = resultDefinitions[0];
  }
  else if (!resultDefinitions.empty())
  {
    // If there are more than one derived definitions, then we pick the one
    // whose type name is keyed for our operation.
    std::string resultClassName;
    {
      std::stringstream s;
      s << "result(" << operatorName << ")";
      resultClassName = s.str();
    }

    for (auto& def : resultDefinitions)
    {
      if (def->type() == resultClassName)
      {
        resultDefinition = def;
        break;
      }
    }
  }
  return resultDefinition;
}

namespace
{
void resourcesFromItem(
  ResourceAccessMap& resourcesAndLockTypes,
  const smtk::attribute::ReferenceItem::Ptr& item)
{
  for (std::size_t i = 0; i < item->numberOfValues(); i++)
  {
    // no need to look at items that cannot be resolved
    if (item->value(i) == nullptr)
    {
      continue;
    }

    // ...access the associated resource.
    smtk::resource::ResourcePtr resource =
      std::dynamic_pointer_cast<smtk::resource::Resource>(item->value(i));

    // If the object is actually a component, access its associated resource.
    if (resource == nullptr)
    {
      resource = std::dynamic_pointer_cast<smtk::resource::Component>(item->value(i))->resource();
    }

    auto it = resourcesAndLockTypes.find(resource);
    if (it == resourcesAndLockTypes.end())
    {
      // If the resource is not yet in our map, add it and set its lock type.
      resourcesAndLockTypes[resource] = item->lockType();
    }
    else
    {
      // If the resource is already in our map, elevate its lock type if
      // necessary.
      if (item->lockType() > it->second)
      {
        it->second = item->lockType();
      }
    }
  }
}
} // namespace

std::set<
  std::weak_ptr<smtk::resource::Resource>,
  std::owner_less<std::weak_ptr<smtk::resource::Resource>>>
extractResources(Operation::Result result)
{
  ResourceAccessMap resourcesAndLockTypes;

  // Gather all of the resource and component items in the specification.
  std::vector<smtk::attribute::Item::Ptr> items;

  // Gather all of the components using a filter.
  auto filter = [](smtk::attribute::Item::Ptr item) {
    return item->type() == smtk::attribute::Item::ReferenceType ||
      item->type() == smtk::attribute::Item::ResourceType ||
      item->type() == smtk::attribute::Item::ComponentType;
  };
  result->filterItems(items, filter, false);

  // Also gather the association.
  if (result->associations() != nullptr)
  {
    items.push_back(result->associations());
  }

  // For each item found...
  for (auto& item : items)
  {
    if (item->name() == "resourcesToExpunge")
    {
      // Skip this item since these are resources explicitly being removed from management.
      continue;
    }
    // Extract the resources and lock types.
    auto resourceItem = std::static_pointer_cast<smtk::attribute::ReferenceItem>(item);
    resourcesFromItem(resourcesAndLockTypes, resourceItem);
  }

  // We are only interested in the resources, so we strip away the lock types.
  std::set<
    std::weak_ptr<smtk::resource::Resource>,
    std::owner_less<std::weak_ptr<smtk::resource::Resource>>>
    resources;
  for (const auto& resourceAndLockType : resourcesAndLockTypes)
  {
    resources.insert(resourceAndLockType.first);
  }

  return resources;
}

ResourceAccessMap extractResourcesAndLockTypes(const Operation::Parameters parameters)
{
  ResourceAccessMap resourcesAndLockTypes;

  // Gather all of the resource and component items in the specification.
  std::vector<smtk::attribute::Item::Ptr> items;

  {
    // For each attribute, gather all of the components using a filter.
    auto filter = [](smtk::attribute::Item::Ptr item) {
      return item->type() == smtk::attribute::Item::ReferenceType ||
        item->type() == smtk::attribute::Item::ResourceType ||
        item->type() == smtk::attribute::Item::ComponentType;
    };
    parameters->filterItems(items, filter, false);

    // Also gather the association.
    if (parameters->associations() != nullptr)
    {
      items.push_back(parameters->associations());
    }
  }

  // For each item found...
  for (auto& item : items)
  {
    // Extract the resources and lock types.
    auto resourceItem = std::static_pointer_cast<smtk::attribute::ReferenceItem>(item);
    resourcesFromItem(resourcesAndLockTypes, resourceItem);
  }

  return resourcesAndLockTypes;
}

ResourceAccessMap extractResourcesAndLockTypes(Operation::Specification specification)
{
  ResourceAccessMap resourcesAndLockTypes;

  // Gather all of the resource and component items in the specification.
  std::vector<smtk::attribute::Item::Ptr> items;

  {
    // First, gather all of the attributes in the specification.
    std::vector<smtk::attribute::AttributePtr> attributes;
    specification->attributes(attributes);

    // Find the result attribute (we will skip all attributes that derive from
    // it).
    auto resultAttribute = specification->findDefinition("result");

    // For each attribute, gather all of the components using a filter.
    auto filter = [](smtk::attribute::Item::Ptr item) {
      return item->type() == smtk::attribute::Item::ReferenceType ||
        item->type() == smtk::attribute::Item::ResourceType ||
        item->type() == smtk::attribute::Item::ComponentType;
    };
    for (auto& attribute : attributes)
    {
      if (attribute->definition()->isA(resultAttribute))
      {
        continue;
      }
      attribute->filterItems(items, filter, false);

      // Also gather the association.
      if (attribute->associations() != nullptr)
      {
        items.push_back(attribute->associations());
      }
    }
  }

  // For each item found...
  for (auto& item : items)
  {
    // Extract the resources and lock types.
    auto resourceItem = std::static_pointer_cast<smtk::attribute::ReferenceItem>(item);
    resourcesFromItem(resourcesAndLockTypes, resourceItem);
  }

  return resourcesAndLockTypes;
}

ComponentDefinitionVector extractComponentDefinitions(Operation::Specification specification)
{
  // If we are passed a bad specification, then its associated operation can
  // accept no definitions. We return an empty vector of component definitions.
  if (specification == nullptr)
  {
    return ComponentDefinitionVector();
  }

  ComponentDefinitionVector componentItemDefinitions;

  {
    // First, gather all of the definitions in the specification.
    std::vector<smtk::attribute::DefinitionPtr> definitions;
    specification->definitions(definitions);

    // For each definition, gather all of the components using a filter.
    auto componentItemDefinitionFilter =
      [](smtk::attribute::ComponentItemDefinition::Ptr /*unused*/) { return true; };
    for (auto& definition : definitions)
    {
      definition->filterItemDefinitions(componentItemDefinitions, componentItemDefinitionFilter);
    }
  }

  return componentItemDefinitions;
}

std::set<std::string> extractTagNames(Operation::Specification specification)
{
  std::set<std::string> tagNames;

  // If we are passed a bad specification, then its associated operation cannot
  //  be in a tag. We return an empty set of strings.
  if (specification == nullptr)
  {
    return tagNames;
  }

  smtk::attribute::DefinitionPtr operationBase = specification->findDefinition("operation");

  // Find all definitions that derive from the operation definition.
  std::vector<smtk::attribute::DefinitionPtr> definitions;
  specification->findAllDerivedDefinitions(operationBase, true, definitions);

  // For each definition, access the tags.
  for (auto& definition : definitions)
  {
    auto tags = definition->tags();
    for (const auto& tag : tags)
    {
      tagNames.insert(tag.name());
    }
  }

  return tagNames;
}

/// @cond dox_ignore
/// (quiet Doxygen warnings from Action enum)
namespace
{
enum class Action
{
  ADD,
  REMOVE,
  QUERY
};

std::set<std::string> emptyTagValues;

bool actOnTag(
  Operation::Specification specification,
  const std::string& tagName,
  Action action,
  const std::set<std::string>& tagValues)
{
  // If we are passed a bad specification, then its associated operation cannot
  //  be in a tag.
  if (specification == nullptr)
  {
    return false;
  }

  smtk::attribute::DefinitionPtr operationBase = specification->findDefinition("operation");

  // Find all definitions that derive from the operation definition.
  std::vector<smtk::attribute::DefinitionPtr> definitions;
  specification->findAllDerivedDefinitions(operationBase, true, definitions);

  bool modified = false;

  // For each definition, access the tag.
  for (auto& definition : definitions)
  {
    auto* tag = definition->tag(tagName);
    // If we are adding...
    if (action == Action::ADD)
    {
      /// ...and the tag doesn't already exist, create it.
      if (!tag)
      {
        modified |= definition->addTag(smtk::attribute::Tag(tagName, tagValues));
      }
      else
      {
        // ...and the tag already exists, add the new values to the existing tag.
        for (const auto& tagValue : tagValues)
        {
          modified |= tag->add(tagValue);
        }
      }
    }
    else if (action == Action::REMOVE)
    {
      // If we are removing and the tag exists, remove the tag.
      if (tag)
      {
        modified |= definition->removeTag(tagName);
      }
    }
    else if (action == Action::QUERY)
    {
      // If we are querying and the tag exists, set the tag values. We
      // const-cast here because the input set of strings is local to the
      // calling method, and there is no out-facing API to access this function.
      if (tag)
      {
        const_cast<std::set<std::string>&>(tagValues) = tag->values();
        modified = true;
      }
    }
  }

  return modified;
}
} // namespace
/// @endcond

bool addTag(Operation::Specification specification, const std::string& tagName)
{
  return actOnTag(specification, tagName, Action::ADD, emptyTagValues);
}

bool addTag(
  Operation::Specification specification,
  const std::string& tagName,
  const std::set<std::string>& tagValues)
{
  return actOnTag(specification, tagName, Action::ADD, tagValues);
}

bool removeTag(Operation::Specification specification, const std::string& tagName)
{
  return actOnTag(specification, tagName, Action::REMOVE, emptyTagValues);
}

std::set<std::string> tagValues(Operation::Specification specification, const std::string& tagName)
{
  std::set<std::string> tagValues;
  actOnTag(specification, tagName, Action::QUERY, tagValues);
  return tagValues;
}

} // namespace operation
} // namespace smtk
