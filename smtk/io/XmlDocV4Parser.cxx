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

void XmlDocV4Parser::processDefinition(pugi::xml_node& defNode, smtk::attribute::DefinitionPtr def)
{
  XmlDocV3Parser::processDefinition(defNode, def);

  xml_node node = defNode.child("AssociationRule");
  if (node)
  {
    pugi::xml_attribute xatt = node.attribute("Name");

    if (xatt)
    {
      std::string name = node.attribute("Name").value();
      auto it = m_resource->associationRules().associationRuleContainer().find(name);
      if (it == m_resource->associationRules().associationRuleContainer().end())
      {
        smtkErrorMacro(m_logger, "Could not find association rule \"" << name << "\"");
      }
      else
      {
        m_resource->associationRules().associationRulesForDefinitions().emplace(
          std::make_pair(def->type(), name));
      }
    }
    else
    {
      smtkErrorMacro(m_logger, "AssociationRule for definition type \""
          << def->type() << "\" does not have \"Name\" attribute.");
    }
  }

  node = defNode.child("DissociationRule");
  if (node)
  {
    pugi::xml_attribute xatt = node.attribute("Name");

    if (xatt)
    {
      std::string name = node.attribute("Name").value();
      auto it = m_resource->associationRules().dissociationRuleContainer().find(name);
      if (it == m_resource->associationRules().dissociationRuleContainer().end())
      {
        smtkErrorMacro(m_logger, "Could not find dissociation rule \"" << name << "\"");
      }
      else
      {
        m_resource->associationRules().dissociationRulesForDefinitions().emplace(
          std::make_pair(def->type(), name));
      }
    }
    else
    {
      smtkErrorMacro(m_logger, "DissociationRule for definition type \""
          << def->type() << "\" does not have \"Name\" attribute.");
    }
  }
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

void XmlDocV4Parser::processViews(xml_node& root)
{
  xml_node styles = root.child("Styles");
  if (!styles)
  {
    XmlDocV3Parser::processViews(root);
    return;
  }

  xml_node child;
  for (child = styles.first_child(); child; child = child.next_sibling())
  {
    xml_attribute xatt;
    xatt = child.attribute("Type");
    if (xatt)
    {
      // this is a valid style group for a smtk::attribute::Definition so lets
      // process all of its children
      xml_node styleNode;
      for (styleNode = child.first_child(); styleNode; styleNode = styleNode.next_sibling())
      {
        xml_attribute xstyle;
        xstyle = styleNode.attribute("Name");
        if (xstyle)
        {
          smtk::view::Configuration::Component style(xstyle.value());
          this->processViewComponent(style, styleNode, false);
          m_resource->addStyle(xatt.value(), style);
        }
        else
        {
          // This is a default style (no name specified)
          smtk::view::Configuration::Component style("");
          this->processViewComponent(style, styleNode, false);
          m_resource->addStyle(xatt.value(), style);
        }
      }
    }
    else
    {
      smtkErrorMacro(m_logger, "Could not find Style's attribute Type - skipping it!");
      continue;
    }
  }
  XmlDocV3Parser::processViews(root);
}

void XmlDocV4Parser::processAssociationRules(pugi::xml_node& root)
{
  xml_node associationRules = root.child("AssociationRules");
  if (associationRules)
  {
    xml_node child;
    for (child = associationRules.first_child(); child; child = child.next_sibling())
    {
      if (!m_resource->associationRules().associationRuleFactory().containsAlias(child.name()))
      {
        smtkErrorMacro(
          m_logger, "Could not find association rule Alias \"" << child.name() << "\"");
        continue;
      }

      std::unique_ptr<smtk::attribute::Rule> associationRule =
        m_resource->associationRules().associationRuleFactory().createFromAlias(child.name());
      (*associationRule) << child;
      m_resource->associationRules().associationRuleContainer().emplace(
        std::make_pair(child.attribute("Name").as_string(), std::move(associationRule)));
    }
  }

  xml_node dissociationRules = root.child("DissociationRules");
  if (dissociationRules)
  {
    xml_node child;
    for (child = dissociationRules.first_child(); child; child = child.next_sibling())
    {
      if (!m_resource->associationRules().dissociationRuleFactory().containsAlias(child.name()))
      {
        smtkErrorMacro(
          m_logger, "Could not find dissociation rule Alias \"" << child.name() << "\"");
        continue;
      }

      std::unique_ptr<smtk::attribute::Rule> dissociationRule =
        m_resource->associationRules().dissociationRuleFactory().createFromAlias(child.name());
      (*dissociationRule) << child;
      m_resource->associationRules().dissociationRuleContainer().emplace(
        std::make_pair(child.attribute("Name").as_string(), std::move(dissociationRule)));
    }
  }
}
