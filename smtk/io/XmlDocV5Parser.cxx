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

namespace
{

template<typename ValueType>
ValueType node_as(const xml_node& node);

template<>
double node_as<double>(const xml_node& node)
{
  return node.text().as_double();
}

template<>
std::string node_as<std::string>(const xml_node& node)
{
  std::string s = node.text().as_string();
  return common::StringUtil::trim(s);
}

template<typename Container, typename ValueType = typename Container::value_type>
void nodeToData(const xml_node& node, Container& container)
{
  for (const xml_node& child : node.children("Value"))
  {
    container.insert(container.end(), node_as<ValueType>(child));
  }
}

template<typename T>
void processProperties(T& object, xml_node& propertiesNode, Logger& logger)
{
  for (const xml_node& propNode : propertiesNode.children("Property"))
  {
    const xml_attribute propNameAtt = propNode.attribute("Name");
    if (!propNameAtt)
    {
      smtkWarningMacro(logger, "Missing Name xml attribute in Property xml node.");
      continue;
    }
    const xml_attribute propTypeAtt = propNode.attribute("Type");
    if (!propTypeAtt)
    {
      smtkWarningMacro(logger, "Missing Type xml attribute in Property xml node.");
      continue;
    }

    // Convert the type to lower case
    std::string attVal = propTypeAtt.value();
    std::string propType = smtk::common::StringUtil::lower(attVal);

    std::string propName = propNameAtt.value();

    if (propType == "int")
    {
      object->properties().template get<int>()[propName] = propNode.text().as_int();
    }
    else if (propType == "double")
    {
      object->properties().template get<double>()[propName] = propNode.text().as_double();
    }
    else if (propType == "string")
    {
      object->properties().template get<std::string>()[propName] = propNode.text().as_string();
    }
    else if (propType == "vector[string]")
    {
      nodeToData(propNode, object->properties().template get<std::vector<std::string>>()[propName]);
    }
    else if (propType == "vector[double]")
    {
      nodeToData(propNode, object->properties().template get<std::vector<double>>()[propName]);
    }
    else if (propType == "bool")
    {
      std::string sval = propNode.text().as_string();
      bool bval;
      if (smtk::common::StringUtil::toBoolean(sval, bval))
      {
        object->properties().template get<bool>()[propName] = bval;
      }
      else
      {
        smtkWarningMacro(
          logger,
          "Invalid Boolean Property Value:" << propNode.text().as_string() << " for Property: "
                                            << propName << " of Object: " << object->name() << ".");
      }
    }
    else
    {
      smtkWarningMacro(
        logger,
        "Unsupported Type:" << propTypeAtt.value()
                            << " in Property xml node for Object: " << object->name() << ".");
    }
  }
}
}; // namespace

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

void XmlDocV5Parser::processAttribute(pugi::xml_node& attNode)
{
  XmlDocV4Parser::processAttribute(attNode);

  xml_node propertiesNode = attNode.child("Properties");
  if (propertiesNode)
  {
    xml_attribute xatt = attNode.attribute("Name");
    if (!xatt)
    {
      return;
    }
    std::string name = xatt.value();
    auto att = m_resource->findAttribute(name);
    if (att)
    {
      processProperties(att, propertiesNode, m_logger);
    }
  }
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
  if (xatt && xatt.as_bool())
  {
    m_resource->properties().get<bool>()["smtk.attribute_panel.display_hint"] = true;
  }
}
