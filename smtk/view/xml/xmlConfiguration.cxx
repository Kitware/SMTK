//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/xml/xmlConfiguration.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

namespace smtk
{
namespace view
{

using namespace pugi;

void from_xml(const pugi::xml_node& node, std::shared_ptr<smtk::view::Configuration>& view)
{
  xml_attribute xatt;
  std::string name, vtype, icon;
  xatt = node.attribute("Name");
  if (xatt)
  {
    name = xatt.value();
  }
  else
  {
    xatt = node.attribute("Title");
    if (xatt)
    {
      name = xatt.value();
    }
    else
    {
      view = nullptr;
      return;
    }
  }

  xatt = node.attribute("Type");
  if (xatt)
  {
    vtype = xatt.value();
  }
  else
  {
    view = nullptr;
    return;
  }

  view = smtk::view::Configuration::New(vtype, name);
  xatt = node.attribute("Icon");
  if (xatt)
  {
    icon = xatt.value();
    view->setIconName(icon);
  }
  from_xml(node, view->details(), true);
}

void from_xml(const xml_node& node, smtk::view::Configuration::Component& comp, bool isTopComp)
{
  // Add the attributes of the node to the component
  xml_attribute xatt;
  std::string name;

  for (xatt = node.first_attribute(); xatt; xatt = xatt.next_attribute())
  {
    // If this is the top View comp then skip Title, Name and Type Attributes
    name = xatt.name();
    if (
      isTopComp && ((name == "Name") || (name == "Title") || (name == "Type") || (name == "Icon")))
    {
      continue;
    }
    comp.setAttribute(name, xatt.value());
  }
  // if the node has text then save it in the component's contents
  // else process the node's children
  if (!node.text().empty())
  {
    comp.setContents(node.text().get());
  }
  else
  {
    xml_node child;
    for (child = node.first_child(); child; child = child.next_sibling())
    {
      from_xml(child, comp.addChild(child.name()), false);
    }
  }
}

} // namespace view
} // namespace smtk
