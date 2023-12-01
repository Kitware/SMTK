//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/ResultOps.h"

#include "smtk/project/Project.h"
#include "smtk/project/ResourceContainer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ResourceItem.h"

namespace smtk
{
namespace operation
{

void addResourcesOfReferenceItem(
  const smtk::attribute::ReferenceItem::Ptr& item,
  std::set<smtk::resource::Resource::Ptr>& resources,
  bool includeProjectChildren)
{
  for (const auto& obj : *item)
  {
    if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
    {
      resources.insert(rsrc);
      if (includeProjectChildren)
      {
        if (auto proj = std::dynamic_pointer_cast<smtk::project::Project>(rsrc))
        {
          // Also add resources of project
          // TODO: Note that this will not recurse if a project has child projects.
          for (const auto& pr : proj->resources())
          {
            resources.insert(pr);
          }
        }
      }
    }
  }
}

std::set<smtk::resource::Resource::Ptr> createdResourcesOfResult(
  const Operation::Result& result,
  bool includeProjectChildren)
{
  std::set<smtk::resource::Resource::Ptr> resources;

  std::vector<std::string> addResourceItemNames = {
    { "resource", "resources", "resourcesCreated" }
  };
  for (const auto& itemName : addResourceItemNames)
  {
    auto rsrcItem = result->findResource(itemName);
    if (rsrcItem)
    {
      addResourcesOfReferenceItem(rsrcItem, resources, includeProjectChildren);
    }
  }
  return resources;
}

std::set<smtk::resource::Resource::Ptr> expungedResourcesOfResult(
  const Operation::Result& result,
  bool includeProjectChildren)
{
  std::set<smtk::resource::Resource::Ptr> resources;

  std::vector<std::string> delResourceItemNames = { { "resourcesToExpunge" } };
  for (const auto& itemName : delResourceItemNames)
  {
    auto rsrcItem = result->findResource(itemName);
    if (rsrcItem)
    {
      addResourcesOfReferenceItem(rsrcItem, resources, includeProjectChildren);
    }
  }
  return resources;
}

} // namespace operation
} // namespace smtk
