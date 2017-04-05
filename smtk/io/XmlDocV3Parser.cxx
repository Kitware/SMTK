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
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/common/DateTimeZonePair.h"

using namespace pugi;
using namespace smtk::io;
using namespace smtk;


XmlDocV3Parser::XmlDocV3Parser(smtk::attribute::System &mySystem):
  XmlDocV2Parser(mySystem)
{
}

XmlDocV3Parser::~XmlDocV3Parser()
{
}

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

xml_node XmlDocV3Parser::getRootNode(xml_document &doc)
{
  xml_node amnode = doc.child("SMTK_AttributeSystem");
  return amnode;
}

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

void XmlDocV3Parser::processDateTimeDef(
  pugi::xml_node &node,
  attribute::DateTimeItemDefinitionPtr idef)
{
  // Process the common value item def stuff
  this->processItemDef(node, idef);

  xml_attribute xatt;
  xatt = node.attribute("NumberOfRequiredValues");
  std::size_t numberOfComponents = 0;
  if (xatt)
    {
    numberOfComponents = xatt.as_uint();
    idef->setNumberOfRequiredValues(numberOfComponents);
    }

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

  xml_node dnode = node.child("DefaultValue");
  if (dnode)
    {
    ::smtk::common::DateTimeZonePair dtz;
    std::string content = dnode.text().get();
    dtz.deserialize(content);
    idef->setDefaultValue(dtz);
    }
}

void XmlDocV3Parser::processDateTimeItem(
  pugi::xml_node &node, attribute::DateTimeItemPtr item)
{
  xml_attribute natt = node.attribute("NumberOfValues");
  if (!natt)
    {
    // Single value
    item->setNumberOfValues(1);
    xml_node noVal = node.child("UnsetVal");
    if (!noVal)
      {
      ::smtk::common::DateTimeZonePair dtz;
      std::string content = node.text().get();
      dtz.deserialize(content);
      item->setValue(dtz);
      }
    }
  else
    {
    // Multiple values
    std::size_t n = natt.as_uint();
    item->setNumberOfValues(n);
    xml_node valsNode = node.child("Values");
    if (valsNode)
      {
      for (xml_node val = valsNode.first_child(); val; val = val.next_sibling())
        {
        std::string nodeName = val.name();
        if (nodeName == "UnsetVal")
          {
          continue;
          }
        xml_attribute ixatt = val.attribute("Ith");
        if (!ixatt)
          {
          smtkErrorMacro(this->m_logger,
                         "XML Attribute Ith is missing for Item: " << item->name());
          continue;
          }
        unsigned int i = ixatt.as_uint();
        if (i >= n)
          {
          smtkErrorMacro(this->m_logger, "XML Attribute Ith = " << i
                         << " is out of range for Item: " << item->name());
          continue;
          }
        if (nodeName == "Val")
          {
          ::smtk::common::DateTimeZonePair dtz;
          std::string content = val.text().get();
          dtz.deserialize(content);
          item->setValue(static_cast<int>(i), dtz);
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Unsupported Value Node Type  Item: "
                         << item->name());
          }  // else
        }  // for (val)
      }  // if (valsNode)
    }  // else
}
