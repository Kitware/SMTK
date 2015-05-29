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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/common/View.h"
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
void XmlDocV2Parser::processFileItem(pugi::xml_node &node,
                                        attribute::FileItemPtr item)
{
  // still process FileItem as V1Parser, but add the recentValues after
  this->XmlDocV1Parser::processFileItem(node, item);

  xml_node valsNode = node.child("RecentValues");

  if (valsNode)
    {
    xml_node val;
    for (val = valsNode.child("Val"); val; val = val.next_sibling("Val"))
      {
      item->addRecentValue(val.text().get());
      }
    }
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
void XmlDocV2Parser::processModelInfo(xml_node &)
{
  /** This seems to be outdated with ModelEntityItem already being processed
   **/
}

//----------------------------------------------------------------------------
void XmlDocV2Parser::processMeshSelectionItem(pugi::xml_node &node,
  attribute::MeshSelectionItemPtr item)
{
  xml_node extraNode = node.child("CtrlKey");
  item->setCtrlKeyDown(extraNode && extraNode.text().as_int() ? true : false);
  extraNode = node.child("MeshModifyMode");
  if(extraNode)
    item->setModifyMode(attribute::MeshSelectionItem::string2ModifyMode(
                            extraNode.text().get()));
  xml_attribute xatt;
  xatt = node.attribute("NumberOfValues");
  if (!xatt || xatt.as_uint() == 0)
    return;

  xml_node selValsNode = node.child("SelectionValues");
  if(selValsNode)
    {
    for (xml_node valsNode = selValsNode.child("Values"); valsNode; valsNode = valsNode.next_sibling("Values"))
      {
      xatt = valsNode.attribute("EntityUUID");
      if(xatt)
        {
        std::set<int> vals;
        for (xml_node val = valsNode.child("Val"); val; val = val.next_sibling("Val"))
          {
          vals.insert(val.text().as_int());
          }
        item->setValues(smtk::common::UUID(xatt.value()), vals);
        }
      }
    }
}

//----------------------------------------------------------------------------
void XmlDocV2Parser::processMeshSelectionDef(pugi::xml_node &node,
                                         attribute::MeshSelectionItemDefinitionPtr idef)
{
  this->processItemDef(node, idef);

  xml_attribute xatt;
  xatt = node.attribute("ModelEntityRef");
  if (xatt)
    {
    idef->setRefModelEntityName(xatt.value());
    }
  else
    {
/*  // this should be optional
    smtkErrorMacro(this->m_logger,
                   "Missing XML Attribute ModelEntityRef for Item Definition : "
                   << idef->name());
*/
    }

  xml_node mmask = node.child("MembershipMask");
  if (mmask)
    {
    idef->setMembershipMask(
      this->decodeModelEntityMask(
        mmask.text().as_string()));
    }

}

//----------------------------------------------------------------------------
void XmlDocV2Parser::processViews(xml_node &root)
{
  xml_node views = root.child("Views");
  if (!views)
    {
    return;
    }

  xml_node child;
  for (child = views.first_child(); child; child = child.next_sibling())
    {
    xml_attribute xatt;
    std::string title, vtype, icon;
    smtk::common::ViewPtr view;
    xatt = child.attribute("Title");
    if (xatt)
      {
      title = xatt.value();
      }
    else
      {
      smtkErrorMacro(this->m_logger,
                   "Could not find View's Title - skipping it!");
      continue;
      }
    
    xatt = child.attribute("Type");
    if (xatt)
      {
      vtype = xatt.value();
      }
    else
      {
      smtkErrorMacro(this->m_logger,
                     "Could not find View " << title << "'s Type - skipping it!");
      continue;
      }

    view = smtk::common::View::New(vtype, title);
    xatt = child.attribute("Icon");
    if (xatt)
      {
      icon = xatt.value();
      view->setIconName(icon);
      }
    this->processViewComponent(view->details(), child, true);
    this->m_system.addView(view);
    }
}

//----------------------------------------------------------------------------
void XmlDocV2Parser::processViewComponent(smtk::common::View::Component &comp,
                                          xml_node &node, bool isTopComp)
{
  // Add the attributes of the node to the component
  xml_attribute xatt;
  std::string name;
  
  for (xatt = node.first_attribute(); xatt; xatt = xatt.next_attribute())
    {
    // If this is the top View comp then skip Title and Type Attributes
    name = xatt.name();
    if (isTopComp && ((name == "Title") || (name == "Type") || (name == "Icon")))
      {
      continue;
      }
    comp.setAttribute(name, xatt.value());
    }
  // if the node has text then save it in the component's contents
  // else process the node's children
  if (!node.text().empty())
    {
    comp.setContents(node.text().get());
    }
  else
    {
    xml_node child;
    for (child = node.first_child(); child; child = child.next_sibling())
      {
      this->processViewComponent(comp.addChild(child.name()), child, false);
      }
    }
}
