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
#include "cJSON.h"
#include "pugixml/src/pugixml.cpp"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/json/Interface.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/view/View.h"
#include <algorithm>
#include <iostream>

using namespace pugi;
using namespace smtk::io;
using namespace smtk;

XmlDocV2Parser::XmlDocV2Parser(smtk::attribute::CollectionPtr myCollection)
  : XmlDocV1Parser(myCollection)
{
}

XmlDocV2Parser::~XmlDocV2Parser()
{
}

bool XmlDocV2Parser::canParse(xml_document& doc)
{
  // Get the attribute collection node
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

bool XmlDocV2Parser::canParse(xml_node& node)
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

xml_node XmlDocV2Parser::getRootNode(xml_document& doc)
{
  xml_node amnode = doc.child("SMTK_AttributeSystem");
  return amnode;
}

void XmlDocV2Parser::process(xml_document& doc)
{
  // Get the attribute collection node
  xml_node amnode = doc.child("SMTK_AttributeSystem");

  // Check that there is content
  if (amnode.empty())
  {
    smtkWarningMacro(m_logger, "Missing SMTK_AttributeSystem element");
    return;
  }

  this->process(amnode);
}

void XmlDocV2Parser::processDefinition(xml_node& defNode, smtk::attribute::DefinitionPtr def)
{
  xml_attribute xatt;
  // we just need to process Secure XML Attribute added in V2
  this->XmlDocV1Parser::processDefinition(defNode, def);
  xatt = defNode.attribute("RootName");
  if (xatt)
  {
    def->setRootName(xatt.value());
  }
}

void XmlDocV2Parser::processDirectoryDef(
  pugi::xml_node& node, attribute::DirectoryItemDefinitionPtr idef)
{
  xml_node defaultNode;
  xml_attribute xatt;
  this->XmlDocV1Parser::processDirectoryDef(node, idef);

  xatt = node.attribute("Extensible");
  if (xatt)
  {
    idef->setIsExtensible(xatt.as_bool());
    xatt = node.attribute("MaxNumberOfValues");
    if (xatt)
    {
      idef->setMaxNumberOfValues(xatt.as_uint());
    }
  }
  // Check for default value
  defaultNode = node.child("DefaultValue");
  if (defaultNode)
  {
    idef->setDefaultValue(defaultNode.text().get());
  }
}

void XmlDocV2Parser::processFileDef(pugi::xml_node& node, attribute::FileItemDefinitionPtr idef)
{
  xml_attribute xatt;
  this->XmlDocV1Parser::processFileDef(node, idef);

  xatt = node.attribute("Extensible");
  if (xatt)
  {
    idef->setIsExtensible(xatt.as_bool());
    xatt = node.attribute("MaxNumberOfValues");
    if (xatt)
    {
      idef->setMaxNumberOfValues(xatt.as_uint());
    }
  }
  // Check for default value
  xml_node defaultNode = node.child("DefaultValue");
  if (defaultNode)
  {
    idef->setDefaultValue(defaultNode.text().get());
  }
}

void XmlDocV2Parser::processStringDef(xml_node& node, smtk::attribute::StringItemDefinitionPtr idef)
{
  xml_attribute xatt;
  // we just need to process Secure XML Attribute added in V2
  this->XmlDocV1Parser::processStringDef(node, idef);
  xatt = node.attribute("Secure");
  if (xatt)
  {
    idef->setIsSecure(xatt.as_bool());
  }
}

smtk::common::UUID XmlDocV2Parser::getAttributeID(xml_node& attNode)
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

void XmlDocV2Parser::processFileItem(pugi::xml_node& node, attribute::FileItemPtr item)
{
  std::size_t i = 0, n = item->numberOfValues();
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  xml_attribute xatt;
  xml_node valsNode;
  xml_node val;
  if (item->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
    {
      smtkErrorMacro(
        m_logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
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
  xml_node singleValNode = node.child("Val");
  if (valsNode)
  {
    for (val = valsNode.child("Val"); val; val = val.next_sibling("Val"))
    {
      xatt = val.attribute("Ith");
      if (!xatt)
      {
        smtkErrorMacro(m_logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_uint();
      if (i >= n)
      {
        smtkErrorMacro(
          m_logger, "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
        continue;
      }
      item->setValue(static_cast<int>(i), val.text().get());
    }
  }
  else if (singleValNode)
  {
    // Workaround for erroneous V2 files
    item->setValue(singleValNode.text().get());
  }
  else if (numRequiredVals == 1)
  {
    item->setValue(node.text().get());
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node Values is missing for Item: " << item->name());
  }

  valsNode = node.child("RecentValues");

  if (valsNode)
  {
    for (val = valsNode.child("Val"); val; val = val.next_sibling("Val"))
    {
      item->addRecentValue(val.text().get());
    }
  }
}

void XmlDocV2Parser::processDirectoryItem(pugi::xml_node& node, attribute::DirectoryItemPtr item)
{
  std::size_t i = 0, n = item->numberOfValues();
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  xml_attribute xatt;
  xml_node valsNode;
  xml_node val;
  if (item->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
    {
      smtkErrorMacro(
        m_logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
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
        smtkErrorMacro(m_logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_uint();
      if (i >= n)
      {
        smtkErrorMacro(
          m_logger, "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
        continue;
      }
      item->setValue(static_cast<int>(i), val.text().get());
    }
  }
  else if (numRequiredVals == 1)
  {
    item->setValue(node.text().get());
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV2Parser::processModelEntityItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  smtk::common::UUID uid;
  smtk::model::ManagerPtr mmgr = m_collection->refModelManager(); // FIXME: Use resource manager!
  xml_node val;
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  std::string attName;
  AttRefInfo info;
  if (!numRequiredVals || item->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
    {
      smtkErrorMacro(
        m_logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
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
        smtkErrorMacro(m_logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_uint();
      if (i >= n)
      {
        smtkErrorMacro(
          m_logger, "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
        continue;
      }
      uid = smtk::common::UUID(val.text().get());
      item->setObjectValue(static_cast<int>(i), mmgr->findEntity(uid));
    }
  }
  else if (numRequiredVals == 1)
  {
    val = node.child("Val");
    if (val)
    {
      uid = smtk::common::UUID(val.text().get());
      item->setObjectValue(mmgr->findEntity(uid));
    }
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV2Parser::processModelInfo(xml_node&)
{
  /** This seems to be outdated with ModelEntityItem already being processed
   **/
}

void XmlDocV2Parser::processMeshSelectionItem(
  pugi::xml_node& node, attribute::MeshSelectionItemPtr item)
{
  xml_node extraNode = node.child("CtrlKey");
  item->setCtrlKeyDown(extraNode && extraNode.text().as_int() ? true : false);
  extraNode = node.child("MeshModifyMode");
  if (extraNode)
    item->setModifyMode(attribute::MeshSelectionItem::string2ModifyMode(extraNode.text().get()));
  xml_attribute xatt;
  xatt = node.attribute("NumberOfValues");
  if (!xatt || xatt.as_uint() == 0)
    return;

  xml_node selValsNode = node.child("SelectionValues");
  if (selValsNode)
  {
    for (xml_node valsNode = selValsNode.child("Values"); valsNode;
         valsNode = valsNode.next_sibling("Values"))
    {
      xatt = valsNode.attribute("EntityUUID");
      if (xatt)
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

void XmlDocV2Parser::processMeshEntityItem(pugi::xml_node& node, attribute::MeshItemPtr item)
{
  xml_attribute xatt;
  std::size_t n = item->numberOfValues();
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  if (!numRequiredVals || item->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
    {
      smtkErrorMacro(
        m_logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
      return;
    }
    n = xatt.as_uint();
    // QUESTION: Should we set numberOfRequired value here?
  }

  if (!n)
  {
    return;
  }

  smtk::common::UUID cid;
  smtk::model::ManagerPtr modelmgr = m_collection->refModelManager();
  xml_node valsNode, val;

  std::size_t i = 0;
  valsNode = node.child("Values");
  if (valsNode)
  {
    for (val = valsNode.child("Val"); val; val = val.next_sibling("Val"), ++i)
    {
      xatt = val.attribute("collectionid");
      if (!xatt)
      {
        smtkErrorMacro(
          m_logger, "XML Attribute collectionid is missing for Item: " << item->name());
        continue;
      }
      if (i >= n)
      {
        smtkErrorMacro(
          m_logger, "The number of values: " << i << " is out of range for Item: " << item->name());
        break;
      }
      cid = smtk::common::UUID(xatt.value());

      //convert back to a handle
      cJSON* jshandle = cJSON_Parse(val.text().get());
      smtk::mesh::HandleRange hrange = smtk::mesh::from_json(jshandle);
      cJSON_Delete(jshandle);
      smtk::mesh::CollectionPtr c = modelmgr->meshes()->collection(cid);
      if (!c)
      {
        std::cerr << "Expecting a valid collection for mesh item: " << item->name() << std::endl;
        continue;
      }
      smtk::mesh::InterfacePtr interface = c->interface();

      if (!interface)
      {
        std::cerr << "Expecting a valid mesh interface for mesh item: " << item->name()
                  << std::endl;
        continue;
      }

      item->appendValue(smtk::mesh::MeshSet(c, interface->getRoot(), hrange));
    }
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV2Parser::processMeshSelectionDef(
  pugi::xml_node& node, attribute::MeshSelectionItemDefinitionPtr idef)
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
    smtkErrorMacro(m_logger,
                   "Missing XML Attribute ModelEntityRef for Item Definition : "
                   << idef->name());
*/
  }

  xml_node mmask = node.child("MembershipMask");
  if (mmask)
  {
    idef->setMembershipMask(this->decodeModelEntityMask(mmask.text().as_string()));
  }
}

void XmlDocV2Parser::processMeshEntityDef(
  pugi::xml_node& node, attribute::MeshItemDefinitionPtr idef)
{
  xml_node child;
  xml_attribute xatt;

  this->processItemDef(node, idef);

  xatt = node.attribute("NumberOfRequiredValues");
  if (xatt)
  {
    idef->setNumberOfRequiredValues(xatt.as_int());
  }

  xatt = node.attribute("Extensible");
  if (xatt)
  {
    idef->setIsExtensible(xatt.as_bool());
    xatt = node.attribute("MaxNumberOfValues");
    if (xatt)
    {
      idef->setMaxNumberOfValues(xatt.as_uint());
    }
  }
}

void XmlDocV2Parser::processViews(xml_node& root)
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
    std::string name, vtype, icon;
    smtk::view::ViewPtr view;
    xatt = child.attribute("Name");
    if (xatt)
    {
      name = xatt.value();
    }
    else
    {
      xatt = child.attribute("Title");
      if (xatt)
      {
        name = xatt.value();
      }
      else
      {
        smtkErrorMacro(m_logger, "Could not find View's Name or Title - skipping it!");
        continue;
      }
    }

    xatt = child.attribute("Type");
    if (xatt)
    {
      vtype = xatt.value();
    }
    else
    {
      smtkErrorMacro(m_logger, "Could not find View " << name << "'s Type - skipping it!");
      continue;
    }

    view = smtk::view::View::New(vtype, name);
    xatt = child.attribute("Icon");
    if (xatt)
    {
      icon = xatt.value();
      view->setIconName(icon);
    }
    this->processViewComponent(view->details(), child, true);
    m_collection->addView(view);
  }
}

void XmlDocV2Parser::processViewComponent(
  smtk::view::View::Component& comp, xml_node& node, bool isTopComp)
{
  // Add the attributes of the node to the component
  xml_attribute xatt;
  std::string name;

  for (xatt = node.first_attribute(); xatt; xatt = xatt.next_attribute())
  {
    // If this is the top View comp then skip Title, Name and Type Attributes
    name = xatt.name();
    if (isTopComp &&
      ((name == "Name") || (name == "Title") || (name == "Type") || (name == "Icon")))
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
