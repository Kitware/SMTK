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
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/json/jsonHandleRange.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/xml/xmlConfiguration.h"
#include <algorithm>
#include <iostream>

using namespace pugi;
using namespace smtk::io;
using namespace smtk;

XmlDocV2Parser::XmlDocV2Parser(smtk::attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlDocV1Parser(myResource, logger)
{
}

XmlDocV2Parser::~XmlDocV2Parser() = default;

bool XmlDocV2Parser::canParse(pugi::xml_document& doc)
{
  // Get the attribute resource node
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
  return versionNum == 2;
}

bool XmlDocV2Parser::canParse(pugi::xml_node& node)
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
  return versionNum == 2;
}

pugi::xml_node XmlDocV2Parser::getRootNode(pugi::xml_document& doc)
{
  xml_node amnode = doc.child("SMTK_AttributeSystem");
  return amnode;
}

void XmlDocV2Parser::process(pugi::xml_document& doc)
{
  // Get the attribute resource node
  xml_node amnode = doc.child("SMTK_AttributeSystem");

  // Check that there is content
  if (amnode.empty())
  {
    smtkWarningMacro(m_logger, "Missing SMTK_AttributeSystem element");
    return;
  }

  this->process(amnode);
}

void XmlDocV2Parser::processDefinitionAtts(xml_node& defNode, smtk::attribute::DefinitionPtr& def)
{
  this->XmlDocV1Parser::processDefinitionAtts(defNode, def);

  xml_attribute xatt;
  // we just need to process RootName added in V2
  xatt = defNode.attribute("RootName");
  if (xatt)
  {
    def->setRootName(xatt.value());
  }
}

void XmlDocV2Parser::processDirectoryDef(
  pugi::xml_node& node,
  attribute::DirectoryItemDefinitionPtr idef)
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

void XmlDocV2Parser::processStringDefAtts(
  xml_node& node,
  const smtk::attribute::StringItemDefinitionPtr& idef)
{
  xml_attribute xatt;
  // we just need to process Secure XML Attribute added in V2
  this->XmlDocV1Parser::processStringDefAtts(node, idef);
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
  // FIXME: Use resource manager!
  // TODO: how do you want to use the resource manager here?
  xml_node val;
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  std::string attName;
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
      for (const auto& association : m_resource->associations())
      {
        if (auto entity = association->find(uid))
        {
          item->setValue(static_cast<int>(i), entity);
          break;
        }
      }
    }
  }
  else if (numRequiredVals == 1)
  {
    val = node.child("Val");
    if (val)
    {
      uid = smtk::common::UUID(val.text().get());
      for (const auto& association : m_resource->associations())
      {
        if (auto entity = association->find(uid))
        {
          item->setValue(0, entity);
          break;
        }
      }
    }
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV2Parser::processMeshEntityDef(
  pugi::xml_node& node,
  attribute::ComponentItemDefinitionPtr idef)
{
  xml_node child;
  xml_attribute xatt;

  idef->setAcceptsEntries(smtk::common::typeName<mesh::Resource>(), "meshset", true);

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
    smtk::view::ConfigurationPtr view;
    from_xml(child, view);
    if (!view)
    {
      smtkErrorMacro(m_logger, "Could not find View's Name/Title or Type - skipping it!");
      continue;
    }
    view->setIncludeIndex(m_includeIndex);
    m_resource->addView(view);
  }
}

void XmlDocV2Parser::processViewComponent(
  smtk::view::Configuration::Component& comp,
  xml_node& node,
  bool isTopComp)
{
  smtk::view::from_xml(node, comp, isTopComp);
}
