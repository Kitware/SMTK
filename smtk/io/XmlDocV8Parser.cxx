//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlDocV8Parser.h"

#include "smtk/common/StringUtil.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

using namespace pugi;

#include "smtk/io/XmlPropertyParsingHelper.txx"

using namespace smtk::attribute;
using namespace smtk::io;
using namespace smtk;

XmlDocV8Parser::XmlDocV8Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV7Parser(myResource, logger)
{
}

XmlDocV8Parser::~XmlDocV8Parser() = default;

bool XmlDocV8Parser::canParse(pugi::xml_document& doc)
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
  return versionNum == 8;
}

bool XmlDocV8Parser::canParse(pugi::xml_node& node)
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
  return versionNum == 8;
}

void XmlDocV8Parser::processCategoryExpressionNode(
  xml_node& node,
  Categories::Expression& catExp,
  Categories::CombinationMode& inheritanceMode)
{
  xml_attribute xatt;
  Categories::CombinationMode catMode;
  // How are we inheriting categories
  xatt = node.attribute("InheritanceMode");
  if (xatt && XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
  {
    inheritanceMode = catMode;
  }

  // Are we dealing with a Pass/Reject All Expression?
  xatt = node.attribute("PassMode");
  if (xatt)
  {
    std::string pval = xatt.value();
    if (pval == "None")
    {
      catExp.setAllReject();
    }
    else if (pval == "All")
    {
      catExp.setAllPass();
    }
    else
    {
      smtkErrorMacro(m_logger, "Unsupported Category PassMode: " << xatt.value());
    }
  }
  else
  {
    std::string exp = node.text().get();
    if (!exp.empty())
    {
      if (!catExp.setExpression(exp))
      {
        smtkErrorMacro(m_logger, "Invalid Category Expression: " << exp);
      }
    }
  }
}

void XmlDocV8Parser::processItemDefChildNode(xml_node& node, const ItemDefinitionPtr& idef)
{
  std::string nodeName = node.name();

  // Are we dealing with Category Expressions
  if (nodeName == "CategoryExpression")
  {
    this->processItemDefCategoryExpressionNode(node, idef);
    return;
  }
  this->XmlDocV7Parser::processItemDefChildNode(node, idef);
}

void XmlDocV8Parser::processItemDefCategoryExpressionNode(xml_node& node, ItemDefinitionPtr idef)
{
  Categories::CombinationMode inheritanceMode = idef->categoryInheritanceMode();
  this->processCategoryExpressionNode(node, idef->localCategories(), inheritanceMode);
  idef->setCategoryInheritanceMode(inheritanceMode);
}

void XmlDocV8Parser::processDefinitionChildNode(xml_node& node, DefinitionPtr& def)
{
  std::string nodeName = node.name();

  // Are we dealing with Properties
  if (nodeName == "Properties")
  {
    processProperties(def, node, m_logger);
    return;
  }

  // Are we dealing with Category Expressions
  if (nodeName == "CategoryExpression")
  {
    this->processDefCategoryExpressionNode(node, def);
    return;
  }
  this->XmlDocV7Parser::processDefinitionChildNode(node, def);
}

void XmlDocV8Parser::processDefCategoryExpressionNode(xml_node& node, DefinitionPtr& def)
{
  Categories::CombinationMode inheritanceMode = def->categoryInheritanceMode();
  this->processCategoryExpressionNode(node, def->localCategories(), inheritanceMode);
  def->setCategoryInheritanceMode(inheritanceMode);
}

void XmlDocV8Parser::processDefinitionAtts(xml_node& defNode, smtk::attribute::DefinitionPtr& def)
{
  pugi::xml_attribute xatt = defNode.attribute("Units");
  if (xatt)
  {
    def->setLocalUnits(xatt.value());
  }

  this->XmlDocV7Parser::processDefinitionAtts(defNode, def);
}

smtk::attribute::AttributePtr XmlDocV8Parser::processAttribute(pugi::xml_node& attNode)
{
  auto att = XmlDocV7Parser::processAttribute(attNode);

  xml_attribute xatt = attNode.attribute("Units");
  if (att && xatt)
  {
    att->setLocalUnits(xatt.value());
  }

  return att;
}

smtk::common::UUID XmlDocV8Parser::getDefinitionID(xml_node& attNode)
{
  xml_attribute xatt;
  smtk::common::UUID id;

  xatt = attNode.attribute("ID");
  if (!xatt)
  {
    id = smtk::common::UUID::null();
  }
  else
  {
    id = smtk::common::UUID(xatt.value());
  }

  return id;
}
