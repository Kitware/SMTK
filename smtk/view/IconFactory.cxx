//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/IconFactory.h"

#include "smtk/common/Color.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/Resource.h"

#include <numeric>

namespace
{
// Key corresponding to the default icon constructor
const std::string defaultName = "_default";
}

namespace smtk
{
namespace view
{
bool IconFactory::registerIconConstructor(
  const std::string& typeName, IconConstructor&& iconConstructor)
{
  return m_iconConstructors.insert({ typeName, std::forward<IconConstructor>(iconConstructor) })
    .second;
}

bool IconFactory::registerDefaultIconConstructor(IconConstructor&& iconConstructor)
{
  return registerIconConstructor(defaultName, std::forward<IconConstructor>(iconConstructor));
}

bool IconFactory::unregisterIconConstructor(const std::string& typeName)
{
  return (m_iconConstructors.erase(typeName) > 0);
}

std::string IconFactory::createIcon(
  const smtk::resource::PersistentObject& object, const std::string& secondaryColor) const
{
  // If there is an IconConstructor registered directly to this
  // PersistentObject, use it.
  auto constructor = m_iconConstructors.find(object.typeName());
  if (constructor != m_iconConstructors.end())
  {
    return constructor->second(object, secondaryColor);
  }

  // If there is no IconConstructor directly registered to this
  // PersistentObject, check if <object> is a Resource or Component.
  const smtk::resource::Resource* resource;
  if (const smtk::resource::Component* component =
        dynamic_cast<const smtk::resource::Component*>(&object))
  {
    resource = component->resource().get();

    // Check if the Resource that owns <object> has an IconConstructor
    // registered directly to it.
    constructor = m_iconConstructors.find(resource->typeName());
    if (constructor != m_iconConstructors.end())
    {
      return constructor->second(object, secondaryColor);
    }
  }
  else
  {
    resource = dynamic_cast<const smtk::resource::Resource*>(&object);
  }

  // If <object> is a resource or component, check if any of the available
  // IconConstructors are assigned to an appropriate parent Resource.
  if (resource != nullptr)
  {
    std::string typeName;
    int nGenerations = std::accumulate(m_iconConstructors.begin(), m_iconConstructors.end(),
      std::numeric_limits<int>::max(),
      [&typeName, resource](int i, const std::pair<std::string, IconConstructor>& constructor) {
        int numberOfGenerations = resource->numberOfGenerationsFromBase(constructor.first);
        if (numberOfGenerations >= 0 && numberOfGenerations < i)
        {
          typeName = constructor.first;
          return numberOfGenerations;
        }
        return i;
      });

    if (nGenerations < std::numeric_limits<int>::max())
    {
      return m_iconConstructors.at(typeName)(object, secondaryColor);
    }
  }

  // If we still haven't found an IconConstructor, use the default one.
  constructor = m_iconConstructors.find(defaultName);
  if (constructor != m_iconConstructors.end())
  {
    return constructor->second(object, secondaryColor);
  }

  // If we don't have a default IconConstructor, there's not much we can do.
  return std::string();
}
}
}
