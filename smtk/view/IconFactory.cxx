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
#include "smtk/resource/PersistentObject.h"

namespace
{
// Key corresponding to the default icon constructor
static const std::string defaultName = "_default";

// Color is stored as a std::vector<double> property associated with this name
static const std::string colorName = "color";

// If an object has no color property, use black (following the convention of
// smtk::model::EntityRef)
static const std::string defaultFillColor = "#000000";
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
  const std::string& typeName, const std::string& lineColor, const std::string& fillColor) const
{
  auto constructor = m_iconConstructors.find(typeName);
  if (constructor == m_iconConstructors.end())
  {
    constructor = m_iconConstructors.find(defaultName);
  }

  if (constructor != m_iconConstructors.end())
  {
    return constructor->second(lineColor, fillColor);
  }

  return "";
}

std::string IconFactory::createIcon(
  const smtk::resource::PersistentObject& object, const std::string& lineColor) const
{
  if (object.properties().contains<std::vector<double> >(colorName))
  {
    return createIcon(
      object.typeName(), lineColor, smtk::common::Color::floatRGBToString(
                                      &object.properties().at<std::vector<double> >(colorName)[0]));
  }
  else
  {
    return createIcon(object.typeName(), lineColor, defaultFillColor);
  }
}
}
}
