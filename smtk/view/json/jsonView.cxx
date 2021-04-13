//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonView.h"
#include "smtk/CoreExports.h"
#include "smtk/view/Configuration.h"

#include "nlohmann/json.hpp"

#include <sstream>

namespace smtk
{
namespace view
{

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::view::Configuration::Component& comp)
{
  j = { { "Name", comp.name() } };
  if (!comp.attributes().empty())
  {
    j["Attributes"] = comp.attributes();
  }
  // if the comp has contents then save it in the node's text
  // else process the comp's children
  if (!comp.contents().empty())
  {
    j["Contents"] = comp.contents();
  }
  else if (!comp.children().empty())
  {
    j["Children"] = comp.children();
  }
}

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::view::Configuration::Component& comp)
{
  comp = smtk::view::Configuration::Component(j.at("Name").get<std::string>());
  nlohmann::json::const_iterator it;
  if ((it = j.find("Contents")) != j.end())
  {
    comp.setContents(it->get<std::string>());
  }
  if ((it = j.find("Attributes")) != j.end())
  {
    for (auto attribute = it->begin(); attribute != it->end(); ++attribute)
    {
      //std::cout << "key " << attribute.key() << " val " << attribute.value().dump(2) << "\n";
      std::string val;
      if (attribute.value().is_string())
      {
        val = attribute.value().get<std::string>();
      }
      else
      {
        std::ostringstream aval;
        aval << attribute.value();
        val = aval.str();
      }
      comp.setAttribute(attribute.key(), val);
    }
  }
  if ((it = j.find("Children")) != j.end())
  {
    for (auto child = it->begin(); child != it->end(); ++child)
    {
      smtk::view::Configuration::Component cc = *child;
      comp.addChild(cc.name()) = cc;
    }
  }
}

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::view::ConfigurationPtr& config)
{
  j = { { "Type", config->type() },
        { "Name", config->name() },
        { "Component", config->details() } };
  if (!config->iconName().empty())
  {
    j["Icon"] = config->iconName();
  }
}

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::view::ConfigurationPtr& config)
{
  config =
    smtk::view::Configuration::New(j["Type"].get<std::string>(), j["Name"].get<std::string>());
  nlohmann::json::const_iterator it;
  if ((it = j.find("Icon")) != j.end())
  {
    config->setIconName(it->get<std::string>());
  }
  if ((it = j.find("Component")) != j.end())
  {
    config->details() = *it;
  }
}
} // namespace view
} // namespace smtk
