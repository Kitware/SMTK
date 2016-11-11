//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/io/XmlDocV3Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"
#include "smtk/attribute/DateTimeItemDefinition.h"

using namespace pugi;
using namespace smtk::io;
using namespace smtk;


//----------------------------------------------------------------------------
XmlDocV3Parser::XmlDocV3Parser(smtk::attribute::System &mySystem):
  XmlDocV2Parser(mySystem)
{
}

//----------------------------------------------------------------------------
XmlDocV3Parser::~XmlDocV3Parser()
{
}
//----------------------------------------------------------------------------
bool XmlDocV3Parser::canParse(xml_document &doc)
{
  // Get the attribute system node
  xml_node amnode = doc.child("SMTK_AttributeSystem");
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
  if (versionNum != 3)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool XmlDocV3Parser::canParse(xml_node &node)
{
  // Check the name of the node
  std::string name = node.name();
  if (name != "SMTK_AttributeSystem")
    {
    return false;
    }

  pugi::xml_attribute xatt = node.attribute("Version");
  if (!xatt)
    {
    return false;
    }

  int versionNum = xatt.as_int();
  if (versionNum != 3)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
xml_node XmlDocV3Parser::getRootNode(xml_document &doc)
{
  xml_node amnode = doc.child("SMTK_AttributeSystem");
  return amnode;
}

//----------------------------------------------------------------------------
void XmlDocV3Parser::process(xml_document &doc)
{
  // Get the attribute system node
  xml_node amnode = doc.child("SMTK_AttributeSystem");

  // Check that there is content
  if (amnode.empty())
    {
    smtkWarningMacro(m_logger, "Missing SMTK_AttributeSystem element");
    return;
    }

  this->process(amnode);
}

//----------------------------------------------------------------------------
void XmlDocV3Parser::processDateTimeDef(
  pugi::xml_node &node,
  attribute::DateTimeItemDefinitionPtr idef)
{
  xml_attribute xatt;

  xatt = node.attribute("DisplayFormat");
  if (xatt)
    {
    idef->setDisplayFormat(xatt.value());
    }

  xatt = node.attribute("ShowTimeZone");
  if (xatt)
    {
    idef->setUseTimeZone(xatt.as_bool());
    }

  xatt = node.attribute("ShowCalendarPopup");
  if (xatt)
    {
    idef->setEnableCalendarPopup(xatt.as_bool());
    }

  // Process common value definition content in base class
  XmlDocV1Parser::processDateTimeDef(node, idef);
}

//----------------------------------------------------------------------------
void XmlDocV3Parser::processDateTimeItem(
  pugi::xml_node &node, attribute::DateTimeItemPtr idef)
{
}
