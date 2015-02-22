//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/io/XmlDocV2Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshEntityItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include <iostream>
#include <algorithm>

using namespace pugi;
using namespace smtk::io;
using namespace smtk;


//----------------------------------------------------------------------------
XmlDocV2Parser::XmlDocV2Parser(smtk::attribute::System &mySystem):
  XmlDocV1Parser(mySystem)
{
}

//----------------------------------------------------------------------------
XmlDocV2Parser::~XmlDocV2Parser()
{
}
//----------------------------------------------------------------------------
bool XmlDocV2Parser::canParse(xml_document &doc)
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
  if (versionNum != 2)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool XmlDocV2Parser::canParse(xml_node &node)
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
  if (versionNum != 2)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
xml_node XmlDocV2Parser::getRootNode(xml_document &doc)
{
  xml_node amnode = doc.child("SMTK_AttributeSystem");
  return amnode;
}

//----------------------------------------------------------------------------
void XmlDocV2Parser::process(xml_document &doc)
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
smtk::common::UUID XmlDocV2Parser::getAttributeID(xml_node &attNode)
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

//----------------------------------------------------------------------------
void XmlDocV2Parser::processModelEntityItem(pugi::xml_node &node,
                                          attribute::ModelEntityItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  smtk::common::UUID uid;
  smtk::model::ManagerPtr mmgr = this->m_system.refModelManager();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::string attName;
  AttRefInfo info;
  if (!numRequiredVals || item->isExtensible())
    {
    // The node should have an attribute indicating how many values are
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
      {
      smtkErrorMacro(this->m_logger,
                     "XML Attribute NumberOfValues is missing for Item: "
                     << item->name());
      return;
      }
    n = xatt.as_uint();
    item->setNumberOfValues(n);
    }

  if (!n)
    {
    return;
    }
  valsNode = node.child("Values");
  if (valsNode)
    {
    for (val = valsNode.child("Val"); val; val = val.next_sibling("Val"))
      {
      xatt = val.attribute("Ith");
      if (!xatt)
        {
        smtkErrorMacro(this->m_logger,
                       "XML Attribute Ith is missing for Item: " << item->name());
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        smtkErrorMacro(this->m_logger, "XML Attribute Ith = " << i
                       << " is out of range for Item: " << item->name());
        continue;
        }
      uid = smtk::common::UUID(val.text().get());
      item->setValue(static_cast<int>(i), smtk::model::EntityRef(mmgr, uid));
      }
    }
  else if (numRequiredVals == 1)
    {
    val = node.child("Val");
    if (val)
      {
      uid = smtk::common::UUID(val.text().get());
      item->setValue(smtk::model::EntityRef(mmgr, uid));
      }
    }
  else
    {
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
}

//----------------------------------------------------------------------------
void XmlDocV2Parser::processModelInfo(xml_node &root)
{
  xml_node modelInfo = root.child("ModelInfo");
  smtk::model::ManagerPtr manager = this->m_system.refModelManager();
  if ( modelInfo && manager)
    {
    std::string name;
    smtk::model::BitFlags mask;
    xml_node gnode;
    for (gnode = modelInfo.child("GroupItem"); gnode; gnode = gnode.next_sibling("GroupItem"))
      {
      xml_attribute xatt;
      xatt = gnode.attribute("Id");
      if(!xatt)
        {
        smtkErrorMacro(this->m_logger, "Model Group is missing XML Attribute Id");
        continue;
        }
      smtk::common::UUID gid( std::string(xatt.as_string()) );
      xatt = gnode.attribute("Mask");
      if(!xatt)
        {
        smtkErrorMacro(this->m_logger, "Model Group is missing XML Attribute Mask");
        continue;
        }
      mask = xatt.as_int();
      xatt = gnode.attribute("Name");
      name = xatt ? xatt.value() : "noname-group";

      smtk::model::Group group = manager->insertGroup(gid,mask,name);
      if(group.isValid())
        {
        xml_node anode;
        for (anode = gnode.child("Attribute"); anode; anode = anode.next_sibling("Attribute"))
          {
          xatt = anode.attribute("Name");
          if(!xatt)
            {
            smtkErrorMacro(this->m_logger,
                           "Model Group associated Attribute is missing XML Attribute Name");
            continue;
            }
          name = xatt.value();
          if(smtk::attribute::AttributePtr att = this->m_system.findAttribute(name))
            {
            group.associateAttribute(att->id());
            }
          else
            {
            smtkErrorMacro(this->m_logger,
                           "Can find Model Group associated Attribute with Name: " << name);
            }
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void XmlDocV2Parser::processMeshEntityItem(pugi::xml_node &node,
  attribute::MeshEntityItemPtr item)
{
  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }

  xml_node valsNode = node.child("Values");
  if (valsNode)
    {
    for (xml_node val = valsNode.child("Val"); val; val = val.next_sibling("Val"))
      {
      item->insertValue(val.text().as_int());
      }
    }
}
