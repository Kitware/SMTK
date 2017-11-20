//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_jsonView_h
#define smtk_view_jsonView_h

#include "smtk/view/View.h"

#include "nlohmann/json.hpp"

#include <sstream>

namespace smtk
{
namespace view
{

inline void to_json(nlohmann::json& j, const smtk::view::View::Component& comp)
{
  j = { { "name", comp.name() } };
  if (!comp.contents().empty())
  {
    j["contents"] = comp.contents();
  }
  if (!comp.attributes().empty())
  {
    j["attributes"] = comp.attributes();
  }
  if (!comp.children().empty())
  {
    j["children"] = comp.children();
  }
}

inline void from_json(const nlohmann::json& j, View::Component& comp)
{
  comp = smtk::view::View::Component(j.at("name").get<std::string>());
  nlohmann::json::const_iterator it;
  if ((it = j.find("contents")) != j.end())
  {
    comp.setContents(it->get<std::string>());
  }
  if ((it = j.find("attributes")) != j.end())
  {
    for (auto attribute = it->begin(); attribute != it->end(); ++attribute)
    {
      //std::cout << "key " << attribute.key() << " val " << attribute.value().dump(2) << "\n";
      std::string val;
      if (attribute.value().is_string())
      {
        val = attribute.value();
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
  if ((it = j.find("children")) != j.end())
  {
    for (auto child = it->begin(); child != it->end(); ++child)
    {
      smtk::view::View::Component cc = *child;
      comp.addChild(cc.name()) = cc;
    }
  }
}

inline void to_json(nlohmann::json& j, const ViewPtr& view)
{
  j = { { "type", view->type() }, { "title", view->title() }, { "component", view->details() } };
  if (!view->iconName().empty())
  {
    j["icon"] = view->iconName();
  }
}

inline void from_json(const nlohmann::json& j, smtk::view::ViewPtr& view)
{
  view = smtk::view::View::New(j["type"].get<std::string>(), j["title"].get<std::string>());
  nlohmann::json::const_iterator it;
  if ((it = j.find("icon")) != j.end())
  {
    view->setIconName(it->get<std::string>());
  }
  if ((it = j.find("component")) != j.end())
  {
    view->details() = *it;
  }
}
}
}

#endif
