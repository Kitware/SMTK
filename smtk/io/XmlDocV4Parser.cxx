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
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk::io;
using namespace smtk;

XmlDocV4Parser::XmlDocV4Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV3Parser(myResource, logger)
{
}

void XmlDocV4Parser::process(xml_document& doc)
{
  // Get the attribute resource node
  xml_node amnode = doc.child("SMTK_AttributeResource");

  // Check that there is content
  if (amnode.empty())
  {
    smtkWarningMacro(m_logger, "Missing SMTK_AttributeResource element");
    return;
  }

  this->process(amnode);
  this->processHints(amnode);
}

void XmlDocV4Parser::process(
  xml_node& rootNode,
  std::map<std::string, std::map<std::string, smtk::io::TemplateInfo>>& globalTemplateMap)
{
  XmlDocV3Parser::process(rootNode, globalTemplateMap);

  xml_node evaluatorsNode = rootNode.child("Evaluators");
  if (evaluatorsNode)
  {
    this->processEvaluators(evaluatorsNode);
  }
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

void XmlDocV4Parser::processDefinitionChildNode(
  pugi::xml_node& node,
  smtk::attribute::DefinitionPtr& def)
{
  std::string nodeName = node.name();

  if (nodeName == "AssociationRule")
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
      smtkErrorMacro(
        m_logger,
        "AssociationRule for definition type \"" << def->type()
                                                 << "\" does not have \"Name\" attribute.");
    }
    return;
  }

  if (nodeName == "DissociationRule")
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
      smtkErrorMacro(
        m_logger,
        "DissociationRule for definition type \"" << def->type()
                                                  << "\" does not have \"Name\" attribute.");
    }
    return;
  }

  XmlDocV3Parser::processDefinitionChildNode(node, def);
}

void XmlDocV4Parser::processItemDefChildNode(
  pugi::xml_node& node,
  const smtk::attribute::ItemDefinitionPtr& idef)
{
  std::string nodeName = node.name();
  // Adding Tag support for item defs
  if (nodeName == "Tags")
  {
    for (xml_node tagNode = node.child("Tag"); tagNode; tagNode = tagNode.next_sibling("Tag"))
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
    return;
  }
  this->XmlDocV3Parser::processItemDefChildNode(node, idef);
}

void XmlDocV4Parser::processItem(pugi::xml_node& node, smtk::attribute::ItemPtr item)
{
  this->XmlDocV3Parser::processItem(node, item);
  xml_attribute xatt = node.attribute("ForceRequired");
  if (xatt)
  {
    item->setForceRequired(xatt.as_bool());
  }
  xatt = node.attribute("IsIgnored");
  if (xatt)
  {
    item->setIsIgnored(xatt.as_bool());
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

void XmlDocV4Parser::processEvaluators(xml_node& evaluatorsNode)
{
  for (const xml_node& evaluatorNode : evaluatorsNode.children("Evaluator"))
  {
    const xml_attribute evaluatorNameAttribute = evaluatorNode.attribute("Name");
    if (!evaluatorNameAttribute)
    {
      smtkWarningMacro(m_logger, "Missing Name xml attribute in Evaluator xml node.");
      continue;
    }
    const std::string evaluatorAlias = evaluatorNameAttribute.value();

    if (!evaluatorNode.child("Definition"))
    {
      smtkWarningMacro(m_logger, "Evaluator does not specify any Definitions.");
      continue;
    }

    for (const xml_node& definitionNode : evaluatorNode.children("Definition"))
    {
      const xml_attribute definitionTypeAttribute = definitionNode.attribute("Type");
      if (!definitionTypeAttribute)
      {
        smtkWarningMacro(
          m_logger,
          "Missing Type xml attribute in Definition xml node for Evaluator"
          "specification.");
        continue;
      }

      const std::string evaluatorDefinition = definitionTypeAttribute.value();
      if (!m_resource->findDefinition(evaluatorDefinition))
      {
        smtkWarningMacro(
          m_logger,
          "Missing definition of type \"" << evaluatorDefinition << "\" in Attribute Resource.");
      }

      const bool defWasSet = m_resource->evaluatorFactory().addDefinitionForEvaluator(
        evaluatorAlias, evaluatorDefinition);
      if (!defWasSet)
      {
        smtkWarningMacro(
          m_logger,
          "Evaluator with alias \"" << evaluatorAlias
                                    << "\" was not found while setting definition "
                                    << evaluatorDefinition);
      }
    }
  }
}
