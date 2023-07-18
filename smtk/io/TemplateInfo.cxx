//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/TemplateInfo.h"

#include "smtk/io/Logger.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

#include <fmt/args.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <iostream>
#include <sstream>

using namespace pugi;
using namespace smtk::io;

bool TemplateInfo::define(
  const std::string& globalNameSpace,
  pugi::xml_node& node,
  bool& isToBeExported,
  smtk::io::Logger& log)
{
  // Reset all of the member variables
  m_name.clear();
  m_nameSpace.clear();
  m_contents.clear();
  m_parameters.clear();
  m_paramValues.clear();

  std::string nodeName = node.name();
  if (!((nodeName == "Block") || (nodeName == "Template")))
  {
    smtkErrorMacro(log, "Unexpected XML Element named: " << nodeName);
    return false; // Don't know how to process this
  }

  xml_attribute xatt = node.attribute("Name");
  if (xatt)
  {
    m_name = xatt.value();
  }
  else
  {
    smtkErrorMacro(log, "Block/Template is Missing Name attribute");
    return false;
  }
  // See if the node has an explicit namespace set.  If it does not
  // it will inherit the global namespace

  xatt = node.attribute("Namespace");
  m_nameSpace = (xatt) ? xatt.value() : globalNameSpace;

  // Is the block marked for export?
  xatt = node.attribute("Export");
  isToBeExported = (xatt) ? xatt.as_bool() : false;

  // If this is a Block element (no template parameters), then there should only
  // be a single child node for its contents, else if it is a template then there
  // should be a Contents child and an optional Parameters Child.
  xml_node contents;
  if (nodeName == "Block")
  {
    contents = node.first_child();
  }
  else
  {
    contents = node.child("Contents");
    std::string paramName, defVal;
    // Lets see if there are Parameters to process
    xml_node params = node.child("Parameters");
    if (params)
    {
      for (xml_node pnode = params.child("Param"); pnode; pnode = pnode.next_sibling("Param"))
      {
        xatt = pnode.attribute("Name");
        if (xatt)
        {
          paramName = xatt.value();
          //Lets see if this parameter is allowed to be empty
          xatt = pnode.attribute("OkToBeEmpty");
          m_parameters.insert(paramName);
          // Is there a default value for this parameter?
          if (!pnode.text().empty())
          {
            defVal = pnode.text().get();
            m_paramValues[paramName] = defVal;
          }
          else if (xatt && xatt.as_bool())
          {
            m_paramValues[paramName] = "";
          }
        }
        else
        {
          smtkErrorMacro(log, "Template Param Element is Missing Name attribute");
        }
      }
    }
  }
  if (!contents)
  {
    smtkErrorMacro(log, "Template/Block contains no contents");
    return false;
  }
  std::stringstream temp;
  contents.print(temp);
  m_contents = temp.str();
  return true;
}

xml_node TemplateInfo::instantiate(
  pugi::xml_node& instancedNode,
  pugi::xml_node& storageNode,
  smtk::io::Logger& log)
{
  std::string processed;
  xml_node emptyNode;
  // Do we have parameters we need to deal with?
  if (m_parameters.empty())
  {
    // nothing to convert
    processed = m_contents;
  }
  else
  {
    // Lets make a copy of the parameter values
    std::map<std::string, std::string> paramValues = m_paramValues;
    std::string paramName, value;
    // Add the parameters defined in the instancedNode
    for (xml_node pnode = instancedNode.child("Param"); pnode; pnode = pnode.next_sibling("Param"))
    {
      xml_attribute xatt = pnode.attribute("Name");
      if ((xatt) && (!pnode.text().empty()))
      {
        paramName = xatt.value();
        value = pnode.text().get();
        paramValues[paramName] = value;
      }
      else
      {
        smtkErrorMacro(log, "Template Param Element is Missing Name attribute");
      }
    }
    // Now build up an arguments list in fmt for the defined parameters
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    bool failed = false;
    for (const auto& param : m_parameters)
    {
      auto paramInfo = paramValues.find(param);
      if (paramInfo == paramValues.end())
      {
        smtkErrorMacro(log, "Parameter: " << param << " was not defined for Template: " << m_name);
        failed = true;
        continue;
      }
      store.push_back(fmt::arg(param.c_str(), paramInfo->second.c_str()));
    }
    if (failed)
    {
      smtkErrorMacro(
        log, "Instantiation of Template: " << m_name << " failed due to missing parameters");
      return emptyNode;
    }
    processed = fmt::vformat(m_contents, store);
  }
  // Convert string into XML element
  xml_document doc;
  if (doc.load_buffer(processed.c_str(), processed.size()))
  {
    xml_node inode = doc.first_child(); //there should only be one node created
    if (inode)
    {
      // Lets add a template name and a template namespace attribute
      inode.append_attribute("TemplateName").set_value(m_name.c_str());
      inode.append_attribute("TemplateNameSpace").set_value(m_nameSpace.c_str());
      // Return a copy of the new node append to the storage node
      return storageNode.append_copy(inode);
    }
    smtkErrorMacro(
      log,
      "Failed Instantiation of Template: "
        << m_name << " could not locate XML node generated from: " << processed);
  }
  else
  {
    smtkErrorMacro(
      log,
      "Failed Instantiation of Template: " << m_name
                                           << " could not create XML from: " << processed);
  }
  return emptyNode;
}
