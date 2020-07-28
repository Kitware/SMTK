//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlDocV4Parser.h"

#include "smtk/common/StringUtil.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk::io;
using namespace smtk;

XmlDocV4Parser::XmlDocV4Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV3Parser(myResource, logger)
{
}

XmlDocV4Parser::~XmlDocV4Parser() = default;

bool XmlDocV4Parser::canParse(pugi::xml_document& doc)
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
  return versionNum == 4;
}

bool XmlDocV4Parser::canParse(pugi::xml_node& node)
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
  return versionNum == 4;
}

pugi::xml_node XmlDocV4Parser::getRootNode(pugi::xml_document& doc)
{
  xml_node amnode = doc.child("SMTK_AttributeResource");
  return amnode;
}

void XmlDocV4Parser::processItemDef(pugi::xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
{
  this->XmlDocV3Parser::processItemDef(node, idef);
  // Adding Tag support for item defs
  xml_node tagsNode = node.child("Tags");
  if (tagsNode)
  {
    for (xml_node tagNode = tagsNode.child("Tag"); tagNode; tagNode = tagNode.next_sibling("Tag"))
    {
      xml_attribute name_att = tagNode.attribute("Name");
      std::string values = tagNode.text().get();
      if (values.empty())
      {
        bool success = idef->addTag(smtk::attribute::Tag(name_att.value()));
        if (!success)
        {
          smtkWarningMacro(m_logger, "Could not add tag \"" << name_att.value() << "\"");
        }
      }
      else
      {
        xml_attribute sep_att = tagNode.attribute("Sep");
        std::string sep = sep_att ? sep_att.value() : ",";
        std::vector<std::string> vals = smtk::common::StringUtil::split(values, sep, false, false);
        bool success = idef->addTag(
          smtk::attribute::Tag(name_att.value(), std::set<std::string>(vals.begin(), vals.end())));
        if (!success)
        {
          smtkWarningMacro(m_logger, "Could not add tag \"" << name_att.value() << "\"");
        }
      }
    }
  }
}

void XmlDocV4Parser::processItem(pugi::xml_node& node, smtk::attribute::ItemPtr item)
{
  this->XmlDocV3Parser::processItem(node, item);
  xml_attribute xatt = node.attribute("ForceRequired");
  if (xatt)
  {
    item->setForceRequired(xatt.as_bool());
  }
}
