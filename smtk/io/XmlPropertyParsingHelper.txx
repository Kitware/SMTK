//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_io_XmlPropertyParsingHelper_txx
#define smtk_io_XmlPropertyParsingHelper_txx

#include "smtk/common/StringUtil.h"
#include "smtk/io/Logger.h"

/// These templates are for aiding XML classes that need to parse nodes that
/// represent SMTK Properties

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
int node_as<int>(const xml_node& node)
{
  return node.text().as_int();
}

template<>
long node_as<long>(const xml_node& node)
{
  std::string v = node.text().as_string();
  return std::stol(v);
}

template<>
std::string node_as<std::string>(const xml_node& node)
{
  std::string s = node.text().as_string();
  return smtk::common::StringUtil::trim(s);
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
void processProperties(T& object, xml_node& propertiesNode, smtk::io::Logger& logger)
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
    else if (propType == "vector[int]")
    {
      nodeToData(propNode, object->properties().template get<std::vector<int>>()[propName]);
    }
    else if (propType == "vector[long]")
    {
      nodeToData(propNode, object->properties().template get<std::vector<long>>()[propName]);
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
#endif
