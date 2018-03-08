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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"

#include "smtk/common/DateTimeZonePair.h"
#include "smtk/common/StringUtil.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

using namespace pugi;
using namespace smtk::io;
using namespace smtk;

XmlDocV3Parser::XmlDocV3Parser(smtk::attribute::CollectionPtr myCollection)
  : XmlDocV2Parser(myCollection)
{
}

XmlDocV3Parser::~XmlDocV3Parser()
{
}

bool XmlDocV3Parser::canParse(xml_document& doc)
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
  if (versionNum != 3)
  {
    return false;
  }

  return true;
}

bool XmlDocV3Parser::canParse(xml_node& node)
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

xml_node XmlDocV3Parser::getRootNode(xml_document& doc)
{
  xml_node amnode = doc.child("SMTK_AttributeSystem");
  return amnode;
}

void XmlDocV3Parser::process(xml_document& doc)
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

void XmlDocV3Parser::processDefinition(xml_node& defNode, smtk::attribute::DefinitionPtr def)
{
  // we just need to process Tags added in V3
  this->XmlDocV2Parser::processDefinition(defNode, def);
  xml_node tagsNode = defNode.child("Tags");
  if (tagsNode)
  {
    for (xml_node tagNode = tagsNode.child("Tag"); tagNode; tagNode = tagNode.next_sibling("Tag"))
    {
      xml_attribute name_att = tagNode.attribute("Name");
      std::string values = tagNode.text().get();
      if (values.empty())
      {
        bool success = def->addTag(smtk::attribute::Tag(name_att.value()));
        if (!success)
        {
          smtkWarningMacro(m_logger, "Could not add tag \"" << name_att.value() << "\"");
        }
      }
      else
      {
        xml_attribute sep_att = tagNode.attribute("Sep");
        std::string sep = sep_att ? sep_att.value() : ",";
        std::vector<std::string> vals = smtk::common::StringUtil::split(values, sep, false, false);
        bool success = def->addTag(
          smtk::attribute::Tag(name_att.value(), std::set<std::string>(vals.begin(), vals.end())));
        if (!success)
        {
          smtkWarningMacro(m_logger, "Could not add tag \"" << name_att.value() << "\"");
        }
      }
    }
  }
}

void XmlDocV3Parser::processDateTimeDef(
  pugi::xml_node& node, attribute::DateTimeItemDefinitionPtr idef)
{
  // Process the common value item def stuff
  this->processItemDef(node, idef);

  xml_attribute xatt;
  xatt = node.attribute("NumberOfRequiredValues");
  std::size_t numberOfComponents = idef->numberOfRequiredValues();
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

void XmlDocV3Parser::processDateTimeItem(pugi::xml_node& node, attribute::DateTimeItemPtr item)
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
          smtkErrorMacro(this->m_logger, "XML Attribute Ith is missing for Item: " << item->name());
          continue;
        }
        unsigned int i = ixatt.as_uint();
        if (i >= n)
        {
          smtkErrorMacro(this->m_logger,
            "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
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
          smtkErrorMacro(this->m_logger, "Unsupported Value Node Type  Item: " << item->name());
        } // else
      }   // for (val)
    }     // if (valsNode)
  }       // else
}

void XmlDocV3Parser::processResourceItem(
  pugi::xml_node& node, smtk::attribute::ResourceItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  smtk::common::UUID uid;
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
        this->m_logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
      return;
    }
    n = xatt.as_uint();
    item->setNumberOfValues(n);
  }
  if (!n)
  {
    return;
  }

  auto rsrcMgr = this->m_collection->manager();
  if (!rsrcMgr && item->numberOfValues() > 0)
  {
    static bool once = true;
    if (once)
    {
      smtkErrorMacro(
        this->m_logger, "The resource manager for the attribute collection being read is "
                        "not set but the ResourceItem \""
          << item->name() << "\" "
                             "of \""
          << item->attribute()->name() << "\" has entries"
                                          "that need to find resources using a resource manager.");
      once = false;
    }
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
        smtkErrorMacro(this->m_logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_uint();
      if (i >= n)
      {
        smtkErrorMacro(this->m_logger,
          "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
        continue;
      }
      item->setValue(static_cast<int>(i), this->processResourceItemValue(val, rsrcMgr));
    }
  }
  else if (numRequiredVals == 1)
  {
    val = node.child("Val");
    if (val)
    {
      item->setValue(0, this->processResourceItemValue(val, rsrcMgr));
    }
  }
  else
  {
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV3Parser::processResourceDef(
  pugi::xml_node& node, smtk::attribute::ResourceItemDefinitionPtr idef)
{
  xml_node accepts, labels, child;
  xml_attribute watt, xatt;
  int i;
  this->processItemDef(node, idef);

  accepts = node.child("Accepts");
  if (accepts)
  {
    std::string resourceName("Resource");
    for (child = accepts.first_child(); child; child = child.next_sibling())
    {
      if (child.name() == resourceName)
      {
        idef->setAcceptsEntries(
          child.attribute("Name").value(), child.attribute("Filter").value(), true);
      }
    }
  }

  watt = node.attribute("Writable");
  if (watt)
  {
    idef->setIsWritable(watt.as_bool());
  }

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

  // Lets see if there are labels
  if (node.child("Labels"))
  {
    smtkErrorMacro(this->m_logger, "Labels has been changed to ResourceLabels : " << idef->name());
  }
  labels = node.child("ResourceLabels");
  if (labels)
  {
    // Are we using a common label?
    xatt = labels.attribute("CommonLabel");
    if (xatt)
    {
      idef->setCommonValueLabel(xatt.value());
    }
    else
    {
      for (child = labels.first_child(), i = 0; child; child = child.next_sibling(), i++)
      {
        idef->setValueLabel(i, child.value());
      }
    }
  }
}

smtk::resource::ResourcePtr XmlDocV3Parser::processResourceItemValue(
  const pugi::xml_node& val, smtk::resource::ManagerPtr rsrcMgr)
{
  auto rsrcNode = val.child("Resource");
  if (!rsrcNode)
  {
    smtkErrorMacro(this->m_logger, "ResourceItem Val node does not have a Resource as required.");
    return smtk::resource::ResourcePtr();
  }
  auto ruid = smtk::common::UUID(rsrcNode.text().get());
  auto rsrc = rsrcMgr->get(ruid);
  if (!rsrc)
  {
    smtkWarningMacro(this->m_logger, "Resource " << ruid << " is not loaded. Ignoring.");
    return smtk::resource::ResourcePtr();
  }
  return rsrc;
}

void XmlDocV3Parser::processComponentItem(
  pugi::xml_node& node, smtk::attribute::ComponentItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  smtk::common::UUID uid;
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
        this->m_logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
      return;
    }
    n = xatt.as_uint();
    item->setNumberOfValues(n);
  }
  if (!n)
  {
    return;
  }

  auto rsrcMgr = this->m_collection->manager();
  if (!rsrcMgr && item->numberOfValues() > 0)
  {
    static bool once = true;
    if (once)
    {
      smtkErrorMacro(
        this->m_logger, "The resource manager for the attribute collection being read is "
                        "not set but the ComponentItem \""
          << item->name() << "\" "
                             "of \""
          << item->attribute()->name() << "\" has entries"
                                          "that need to find resources using a resource manager.");
      once = false;
    }
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
        smtkErrorMacro(this->m_logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_uint();
      if (i >= n)
      {
        smtkErrorMacro(this->m_logger,
          "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
        continue;
      }
      item->setValue(static_cast<int>(i), this->processComponentItemValue(val, rsrcMgr));
    }
  }
  else if (numRequiredVals == 1)
  {
    val = node.child("Val");
    if (val)
    {
      item->setValue(0, this->processComponentItemValue(val, rsrcMgr));
    }
  }
  else
  {
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV3Parser::processComponentDef(
  pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef)
{
  xml_node accepts, labels, child;
  xml_attribute watt, xatt;
  int i;
  this->processItemDef(node, idef);

  accepts = node.child("Accepts");
  if (accepts)
  {
    std::string resourceName("Resource");
    for (child = accepts.first_child(); child; child = child.next_sibling())
    {
      if (child.name() == resourceName)
      {
        idef->setAcceptsEntries(
          child.attribute("Name").value(), child.attribute("Filter").value(), true);
      }
    }
  }

  watt = node.attribute("Writable");
  if (watt)
  {
    idef->setIsWritable(watt.as_bool());
  }

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

  // Lets see if there are labels
  if (node.child("Labels"))
  {
    smtkErrorMacro(this->m_logger, "Labels has been changed to ComponentLabels : " << idef->name());
  }
  labels = node.child("ComponentLabels");
  if (labels)
  {
    // Are we using a common label?
    xatt = labels.attribute("CommonLabel");
    if (xatt)
    {
      idef->setCommonValueLabel(xatt.value());
    }
    else
    {
      for (child = labels.first_child(), i = 0; child; child = child.next_sibling(), i++)
      {
        idef->setValueLabel(i, child.value());
      }
    }
  }
}

smtk::resource::ComponentPtr XmlDocV3Parser::processComponentItemValue(
  const pugi::xml_node& val, smtk::resource::ManagerPtr rsrcMgr)
{
  auto rsrcNode = val.child("Resource");
  auto compNode = val.child("Component");
  if (!rsrcNode || !compNode)
  {
    smtkErrorMacro(this->m_logger,
      "ComponentItem Val node does not have both a Resource and Component as required.");
    return smtk::resource::ComponentPtr();
  }
  auto ruid = smtk::common::UUID(rsrcNode.text().get());
  auto rsrc = rsrcMgr->get(ruid);
  if (!rsrc)
  {
    smtkWarningMacro(this->m_logger, "Resource " << ruid << " is not loaded. Ignoring.");
    return smtk::resource::ComponentPtr();
  }
  auto cuid = smtk::common::UUID(compNode.text().get());
  return rsrc->find(cuid);
}
