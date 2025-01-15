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

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"
using namespace pugi;

#include "smtk/io/XmlPropertyParsingHelper.txx"

using namespace smtk::io;
using namespace smtk;

XmlDocV5Parser::XmlDocV5Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV4Parser(myResource, logger)
{
}

XmlDocV5Parser::~XmlDocV5Parser() = default;

void XmlDocV5Parser::process(
  xml_node& rootNode,
  std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap)
{
  pugi::xml_attribute xatt = rootNode.attribute("NameSeparator");

  if (xatt)
  {
    std::string nameSep = xatt.value();
    m_resource->setDefaultNameSeparator(nameSep);
  }

  XmlDocV4Parser::process(rootNode, globalTemplateMap);

  xml_node propertiesNode = rootNode.child("Properties");
  if (propertiesNode)
  {
    processProperties(m_resource, propertiesNode, m_logger);
  }
}

smtk::attribute::AttributePtr XmlDocV5Parser::processAttribute(pugi::xml_node& attNode)
{
  auto att = XmlDocV4Parser::processAttribute(attNode);

  if (att)
  {
    xml_node propertiesNode = attNode.child("Properties");
    if (propertiesNode)
    {
      xml_attribute xatt = attNode.attribute("Name");
      if (xatt)
      {
        std::string name = xatt.value();
        processProperties(att, propertiesNode, m_logger);
      }
    }
  }
  return att;
}

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
  m_resource->properties().get<bool>()["smtk.attribute_panel.display_hint"] =
    xatt && xatt.as_bool();
}
