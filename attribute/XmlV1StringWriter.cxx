/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#include "attribute/XmlV1StringWriter.h"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "attribute/Manager.h"
#include "attribute/Definition.h"
#include <iostream>

using namespace rapidxml;
using namespace slctk::attribute; 
 
//----------------------------------------------------------------------------
XmlV1StringWriter::XmlV1StringWriter(const Manager &myManager):
m_manager(myManager)
{
}

//----------------------------------------------------------------------------
XmlV1StringWriter::~XmlV1StringWriter()
{
}
//----------------------------------------------------------------------------
std::string XmlV1StringWriter::convertToString()
{
  xml_node<>* decl = this->m_doc.allocate_node(node_declaration);
  decl->append_attribute(this->m_doc.allocate_attribute("version", "1.0"));
  decl->append_attribute(this->m_doc.allocate_attribute("encoding", "utf-8"));
  this->m_doc.append_node(decl);
  // First we need to create the Root Node
  this->m_root = this->m_doc.allocate_node(node_element, "SLCTK_AttributeManager");
  this->m_root->append_attribute(this->m_doc.allocate_attribute("version", "1.0"));
  this->m_doc.append_node(this->m_root);
  
  //Now we need to create the main elements
  this->m_definitions = this->m_doc.allocate_node(node_element, 
                                                  "Definitions");
  this->m_root->append_node(this->m_definitions);

  this->m_instances = this->m_doc.allocate_node(node_element, 
                                                  "Attributes");
  this->m_root->append_node(this->m_instances);

  this->m_sections = this->m_doc.allocate_node(node_element, 
                                                  "Sections");
  this->m_root->append_node(this->m_sections);

  this->m_modelInfo = this->m_doc.allocate_node(node_element, 
                                                  "ModelInfo");
  this->m_root->append_node(this->m_modelInfo);

  this->processDefinitions();
  this->processInstances();
  this->processSections();
  this->processModelInfo();

  std::string result;
  print(std::back_inserter(result), this->m_doc);
  return result;
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDefinitions()
{
  std::vector<slctk::AttributeDefinitionPtr> baseDefs;
  this->m_manager.findBaseDefinitions(baseDefs);
  std::size_t i, n = baseDefs.size();
  for (i = 0; i < n; i++)
    {
    this->processDefinition(baseDefs[i]);
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDefinition(slctk::AttributeDefinitionPtr def)
{
  int si = this->m_strings.size(); // next index for strings;
  this->m_strings.push_back(def->type());
  this->convertStringToXML(this->m_strings[si]);
  rapidxml::xml_node<> *node = 
    this->m_doc.allocate_node(node_element, this->m_strings[si].c_str());
  this->m_definitions->append_node(node);
  std::cerr << "Node Name = " << node->name() << "\n";
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processInstances()
{
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processSections()
{
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processModelInfo()
{
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::convertStringToXML(std::string &str)
{
  int i, n = str.size();
  for (i = 0; i < n; i++)
    {
    // See if we have any special XML characters to escape
    if (str[i] == '\'')
      {
      str.replace(i, 1, "&apos;");
      i+=5;
      n+=5;
      continue;
      }

    if (str[i] == '>')
      {
      str.replace(i, 1, "&gt;");
      i+=3;
      n+=3;
      continue;
      }
    
    if (str[i] == '<')
      {
      str.replace(i, 1, "&lt;");
      i+=3;
      n+=3;
      continue;
      }
    
    if (str[i] == '\"')
      {
      str.replace(i, 1, "&quot;");
      i+=5;
      n+=5;
      continue;
      }
    
    if (str[i] == '&')
      {
      str.replace(i, 1, "&amp;");
      i+=4;
      n+=4;
      continue;
      }
    }
}
//----------------------------------------------------------------------------
