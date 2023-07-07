//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlDocV6Parser.h"

#include "smtk/common/StringUtil.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk::attribute;
using namespace smtk::io;
using namespace smtk;

XmlDocV6Parser::XmlDocV6Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV5Parser(myResource, logger)
{
}

XmlDocV6Parser::~XmlDocV6Parser() = default;

bool XmlDocV6Parser::canParse(pugi::xml_document& doc)
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
  return versionNum == 6;
}

bool XmlDocV6Parser::canParse(pugi::xml_node& node)
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
  return versionNum == 6;
}

void XmlDocV6Parser::process(
  pugi::xml_node& rootNode,
  std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap)
{
  pugi::xml_attribute xatt;

  xatt = rootNode.attribute("TemplateType");
  if (xatt)
  {
    smtk::string::Token templateType = xatt.value();
    m_resource->setTemplateType(templateType);
  }

  unsigned long long tmp = 0;
  xatt = rootNode.attribute("TemplateVersion");
  if (xatt)
  {
    tmp = xatt.as_ullong(tmp);
  }
  std::size_t templateVersion = static_cast<std::size_t>(tmp);
  m_resource->setTemplateVersion(templateVersion);

  XmlDocV5Parser::process(rootNode, globalTemplateMap);
}

// This is to support specifying category inheritance via XML Attributes
void XmlDocV6Parser::processCategoryAtts(
  xml_node& node,
  Categories::Set& catSet,
  Categories::CombinationMode& inheritanceMode)
{
  attribute::Categories::Set::CombinationMode catMode;

  // The default inheritance mode is And
  inheritanceMode = Categories::CombinationMode::And;

  // Check for old style
  xml_attribute xatt = node.attribute("OkToInheritCategories");
  if (xatt && !xatt.as_bool())
  {
    inheritanceMode = Categories::CombinationMode::LocalOnly;
  }
  xatt = node.attribute("CategoryCheckMode");
  if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
  {
    catSet.setInclusionMode(catMode);
  }
}

void XmlDocV6Parser::processCategoryInfoNode(
  xml_node& node,
  Categories::Set& catSet,
  Categories::CombinationMode& inheritanceMode)
{
  attribute::Categories::Set::CombinationMode catMode;
  xml_node child;
  xml_attribute xatt;

  // How are we inheriting categories
  xatt = node.attribute("InheritanceMode");
  if (xatt && XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
  {
    inheritanceMode = catMode;
  }
  else // Check the old style
  {
    xatt = node.attribute("Inherit");
    if (xatt)
    {
      // In teh old style inheriting meant or'ing
      inheritanceMode =
        xatt.as_bool() ? Categories::CombinationMode::Or : Categories::CombinationMode::LocalOnly;
    }
  }

  // Lets get the overall combination mode
  xatt = node.attribute("Combination");
  if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
  {
    catSet.setCombinationMode(catMode);
  }
  // Get the Include set (if one exists)
  xml_node catGroup;
  catGroup = node.child("Include");
  if (catGroup)
  {
    // Lets get the include combination mode
    xatt = catGroup.attribute("Combination");
    if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
    {
      catSet.setInclusionMode(catMode);
    }
    for (child = catGroup.first_child(); child; child = child.next_sibling())
    {
      catSet.insertInclusion(child.text().get());
    }
  }
  catGroup = node.child("Exclude");
  if (catGroup)
  {
    // Lets get the include combination mode
    xatt = catGroup.attribute("Combination");
    if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
    {
      catSet.setExclusionMode(catMode);
    }
    for (child = catGroup.first_child(); child; child = child.next_sibling())
    {
      catSet.insertExclusion(child.text().get());
    }
  }
}
