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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"

namespace smtk
{
namespace operation
{
ResourceAccessMap extractResourcesAndPermissions(NewOp::Specification specification)
{
  ResourceAccessMap resourcesAndPermissions;

  auto permission = [](bool isWritable) {
    return (isWritable ? smtk::resource::Permission::Write : smtk::resource::Permission::Read);
  };

  // Gather all of the component itmes in the specification.
  std::vector<smtk::attribute::ComponentItem::Ptr> componentItems;

  {
    // First, gather all of the attributes in the specification.
    std::vector<smtk::attribute::AttributePtr> attributes;
    specification->attributes(attributes);

    // Find the result attribute (we will skip all attributes that derive from
    // it).
    auto resultAttribute = specification->findDefinition("result");

    // For each attribute, gather all of the components using a filter.
    auto componentFilter = [](smtk::attribute::ComponentItem::Ptr) { return true; };
    for (auto& attribute : attributes)
    {
      if (attribute->definition()->isA(resultAttribute))
      {
        continue;
      }
      attribute->filterItems(componentItems, componentFilter, false);
    }
  }

  // For each component item found...
  for (auto& componentItem : componentItems)
  {
    // ...for each component in a component item...
    for (std::size_t i = 0; i < componentItem->numberOfValues(); i++)
    {
      // (no need to look at components that cannot be resolved)
      if (!componentItem->isValid() || componentItem->value(i) == nullptr)
      {
        continue;
      }

      // ...access the component's resource.
      smtk::resource::ResourcePtr resource = componentItem->value(i)->resource();

      auto it = resourcesAndPermissions.find(resource);
      if (it == resourcesAndPermissions.end())
      {
        // If the resource is not yet in our map, add it and set its permission.
        resourcesAndPermissions[resource] = permission(componentItem->isWritable());
      }
      else
      {
        // If the resource is already in our map, elevate its permission if
        // necessary.
        if (componentItem->isWritable() == true)
        {
          it->second = permission(true);
        }
      }
    }
  }

  return resourcesAndPermissions;
}

ComponentDefinitionVector extractComponentDefinitions(NewOp::Specification specification)
{
  // If we are passed a bad specification, then nothing its associated operator
  // can accept no definitions. We return an empty vector of component
  // definitions.
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
    auto componentItemDefinitionFilter = [](
      smtk::attribute::ComponentItemDefinition::Ptr) { return true; };
    for (auto& definition : definitions)
    {
      definition->filterItemDefinitions(componentItemDefinitions, componentItemDefinitionFilter);
    }
  }

  return componentItemDefinitions;
}

} // operation namespace
} // smtk namespace
