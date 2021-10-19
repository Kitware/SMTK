//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlDocV5Parser.h"

#include "smtk/common/StringUtil.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk::io;
using namespace smtk;

XmlDocV5Parser::XmlDocV5Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV4Parser(myResource, logger)
{
}

XmlDocV5Parser::~XmlDocV5Parser() = default;

bool XmlDocV5Parser::canParse(pugi::xml_document& doc)
{
  // Get the attribute resource node
  xml_node amnode = doc.child("SMTK_AttributeResource");
  if (amnode.empty())
  {
    return false;
  }

  pugi::xml_attribute xatt = amnode.attribute("Version");
  if (!xatt)
  {
    return false;
  }

  int versionNum = xatt.as_int();
  return versionNum == 5;
}

bool XmlDocV5Parser::canParse(pugi::xml_node& node)
{
  // Check the name of the node
  std::string name = node.name();
  if (name != "SMTK_AttributeResource")
  {
    return false;
  }

  pugi::xml_attribute xatt = node.attribute("Version");
  if (!xatt)
  {
    return false;
  }

  int versionNum = xatt.as_int();
  return versionNum == 5;
}

void XmlDocV5Parser::processHints(pugi::xml_node& root)
{
  pugi::xml_attribute xatt = root.attribute("DisplayHint");
  if (xatt && xatt.as_bool())
  {
    m_resource->properties().get<bool>()["smtk.attribute_panel.display_hint"] = true;
  }
}
