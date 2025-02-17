//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/Utility.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

ResourceMap objectsToResourceMap(const std::set<smtk::resource::PersistentObject*>& objects)
{
  std::unordered_map<smtk::resource::Resource*, std::unordered_set<smtk::resource::Component*>>
    visibilityMap;
  for (const auto& object : objects)
  {
    if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
    {
      if (auto* rsrc = comp->parentResource())
      {
        visibilityMap[rsrc].insert(comp);
      }
    }
    else if (auto* rsrc = dynamic_cast<smtk::resource::Resource*>(object))
    {
      visibilityMap[rsrc].insert(nullptr); // Indicate the resource with a null component.
    }
  }
  return visibilityMap;
}

SharedResourceMap objectsToSharedResourceMap(
  const std::set<smtk::resource::PersistentObject*>& objects)
{
  SharedResourceMap visibilityMap;
  for (const auto& object : objects)
  {
    if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
    {
      if (auto rsrc = comp->resource())
      {
        auto vit = visibilityMap.find(rsrc->id());
        if (vit == visibilityMap.end())
        {
          vit = visibilityMap.insert({ rsrc->id(), { rsrc, {} } }).first;
        }
        vit->second.m_components.insert(comp->shared_from_this());
      }
    }
    else if (auto* rsrc = dynamic_cast<smtk::resource::Resource*>(object))
    {
      auto vit = visibilityMap.find(rsrc->id());
      if (vit == visibilityMap.end())
      {
        vit = visibilityMap.insert({ rsrc->id(), {} }).first;
        vit->second.m_resource = rsrc->shared_from_this();
      }
      // Indicate the resource with a null component.
      vit->second.m_components.insert(nullptr);
    }
  }
  return visibilityMap;
}

} // namespace view
} // namespace smtk
