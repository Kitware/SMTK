//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlDocV1Parser.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

#include "smtk/io/ItemDefinitionsHelper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Categories.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/CustomItem.h"
#include "smtk/attribute/CustomItemDefinition.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/common/DateTimeZonePair.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/Properties.h"

#include "smtk/common/StringUtil.h"
#include "smtk/view/Configuration.h"

#include <algorithm>
#include <iostream>

using namespace pugi;
using namespace smtk::attribute;
using namespace smtk::io;
using namespace smtk::common;
using namespace smtk;

// Some helper functions and classes
namespace
{

int getValueFromXMLElement(xml_node& node, int /*unused*/)
{
  return node.text().as_int();
}

double getValueFromXMLElement(xml_node& node, double /*unused*/)
{
  return node.text().as_double();
}

const char* getValueFromXMLElement(xml_node& node, std::string /*unused*/)
{
  return node.text().get();
}

std::vector<int>
getValueFromXMLElement(xml_node& node, const std::string& sep, std::vector<int> /*unused*/)
{
  std::vector<int> result;
  std::vector<std::string> vals;
  std::stringstream convert;
  int val;
  vals = smtk::common::StringUtil::split(node.text().get(), sep, false, true);
  std::vector<std::string>::iterator it;
  for (it = vals.begin(); it != vals.end(); ++it)
  {
    convert.str(*it);
    convert >> val;
    result.push_back(val);
    convert.clear();
  }
  return result;
}

std::vector<double>
getValueFromXMLElement(xml_node& node, const std::string& sep, std::vector<double> /*unused*/)
{
  std::vector<double> result;
  std::vector<std::string> vals;
  std::stringstream convert;
  double val;
  vals = smtk::common::StringUtil::split(node.text().get(), sep, false, true);
  std::vector<std::string>::iterator it;
  for (it = vals.begin(); it != vals.end(); ++it)
  {
    convert.str(*it);
    convert >> val;
    result.push_back(val);
    convert.clear();
  }
  return result;
}

std::vector<std::string>
getValueFromXMLElement(xml_node& node, const std::string& sep, std::vector<std::string> /*unused*/)
{
  std::vector<std::string> vals;
  vals = smtk::common::StringUtil::split(node.text().get(), sep, false, false);
  return vals;
}

template<typename ItemDefType, typename BasicType>
void processDerivedValueDef(pugi::xml_node& node, ItemDefType idef, Logger& logger)
{
  xml_node dnode, child, rnode;
  xml_attribute xatt;
  attribute::Categories::Set::CombinationMode catMode;
  // Is the item discrete?
  dnode = node.child("DiscreteInfo");
  if (dnode)
  {
    BasicType val;
    int i;
    xml_node vnode;
    std::string cname;
    for (child = dnode.first_child(), i = 0; child; child = child.next_sibling(), i++)
    {
      cname = child.name();
      if (cname == "Structure")
      {
        vnode = child.child("Value");
      }
      else if (cname == "Value")
      {
        vnode = child;
      }
      else
      {
        continue; // XML Element I don't care about
      }
      if (!vnode)
      {
        smtkErrorMacro(
          logger,
          "Missing XML Node \"Value\" in DiscreteInfo section of Item Definition : "
            << idef->name());
        continue;
      }

      xatt = vnode.attribute("Enum");
      val = getValueFromXMLElement(vnode, BasicType());
      if (xatt)
      {
        idef->addDiscreteValue(val, xatt.value());
      }
      else
      {
        idef->addDiscreteValue(val);
      }
      // First grab the associated enum
      std::string v = idef->discreteEnum(i);
      // Check to see if the enum has an explicit advance level
      xatt = vnode.attribute("AdvanceLevel");
      if (xatt)
      {
        idef->setEnumAdvanceLevel(v, xatt.as_uint());
      }
      if (cname != "Structure")
      {
        continue;
      }
      // Ok lets read in the items associated with this value
      xml_node inode, items = child.child("Items");
      if (items)
      {
        for (inode = items.child("Item"); inode; inode = inode.next_sibling("Item"))
        {
          std::string iname = inode.text().get();
          idef->addConditionalItem(v, iname);
        }
      }
      // Does the enum have explicit categories
      // This is the old format for categories
      xml_node catNodes = child.child("Categories");
      // This is the current format
      xml_node catInfoNode = child.child("CategoryInfo");
      if (catInfoNode)
      {
        Categories::Set cats;
        // Lets get the overall combination mode
        xatt = catInfoNode.attribute("Combination");
        if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
        {
          cats.setCombinationMode(catMode);
        }
        // Get the Include set (if one exists)
        xml_node catGroup;
        catGroup = catInfoNode.child("Include");
        if (catGroup)
        {
          // Lets get the include combination mode
          xatt = catGroup.attribute("Combination");
          if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
          {
            cats.setInclusionMode(catMode);
          }
          for (inode = catGroup.first_child(); inode; inode = inode.next_sibling())
          {
            cats.insertInclusion(inode.text().get());
          }
        }
        catGroup = catInfoNode.child("Exclude");
        if (catGroup)
        {
          // Lets get the include combination mode
          xatt = catGroup.attribute("Combination");
          if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
          {
            cats.setExclusionMode(catMode);
          }
          for (inode = catGroup.first_child(); inode; inode = inode.next_sibling())
          {
            cats.insertExclusion(inode.text().get());
          }
        }
        idef->setEnumCategories(v, cats);
      }
      else if (catNodes)
      {
        Categories::Set cats;
        xatt = catNodes.attribute("CategoryCheckMode");
        if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
        {
          cats.setInclusionMode(catMode);
        }
        for (inode = catNodes.child("Cat"); inode; inode = inode.next_sibling("Cat"))
        {
          std::string iname = inode.text().get();
          cats.insertInclusion(iname);
        }
        idef->setEnumCategories(v, cats);
      }
    }
    xatt = dnode.attribute("DefaultIndex");
    if (xatt)
    {
      idef->setDefaultDiscreteIndex(xatt.as_int());
    }
    return;
  }
  // Does this def have a default value
  dnode = node.child("DefaultValue");
  if (dnode)
  {
    xatt = dnode.attribute("Sep");
    std::string sep = xatt ? xatt.value() : ",";
    std::vector<BasicType> defs = getValueFromXMLElement(dnode, sep, std::vector<BasicType>());
    if (defs.size() == 1 || defs.size() == idef->numberOfRequiredValues())
    {
      idef->setDefaultValue(defs);
    }
    else
    {
      smtkErrorMacro(
        logger,
        "XML DefaultValue has incorrect size: " << defs.size() << " for item " << idef->type());
    }
  }
  // Does this node have a range?
  rnode = node.child("RangeInfo");
  if (rnode)
  {
    bool inclusive;
    child = rnode.child("Min");
    if (child)
    {
      xatt = child.attribute("Inclusive");
      if (xatt)
      {
        inclusive = xatt.as_bool();
      }
      else
      {
        inclusive = false;
      }
      idef->setMinRange(getValueFromXMLElement(child, BasicType()), inclusive);
    }

    child = rnode.child("Max");
    if (child)
    {
      xatt = child.attribute("Inclusive");
      if (xatt)
      {
        inclusive = xatt.as_bool();
      }
      else
      {
        inclusive = false;
      }
      idef->setMaxRange(getValueFromXMLElement(child, BasicType()), inclusive);
    }
  }
}

template<typename ItemType, typename BasicType>
void processDerivedValue(
  pugi::xml_node& node,
  ItemType item,
  attribute::ResourcePtr resource,
  std::vector<ItemExpressionInfo>& itemExpressionInfo,
  Logger& logger)
{
  if (item->isDiscrete())
  {
    return; // nothing left to do
  }

  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val, noVal;
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  std::string nodeName, expName;
  attribute::AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;

  // Is this an expression?
  xatt = node.attribute("Expression");
  if (allowsExpressions && xatt)
  {
    expName = node.text().get();
    expAtt = resource->findAttribute(expName);
    if (!expAtt)
    {
      info.item = item;
      info.pos = 0;
      info.expName = expName;
      itemExpressionInfo.push_back(info);
    }
    else
    {
      item->setExpression(expAtt);
    }
    return;
  }

  if (item->isExtensible())
  {
    // The node should have an attribute indicating how many values are
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
    {
      smtkErrorMacro(logger, "XML Attribute NumberOfValues is missing for Item: " << item->name());
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
    for (val = valsNode.first_child(); val; val = val.next_sibling())
    {
      nodeName = val.name();
      if (nodeName == "UnsetVal")
      {
        continue;
      }
      xatt = val.attribute("Ith");
      if (!xatt)
      {
        smtkErrorMacro(logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_uint();
      if (i >= n)
      {
        smtkErrorMacro(
          logger, "XML Attribute Ith = " << i << " is out of range for Item: " << item->name());
        continue;
      }
      if (nodeName == "Val")
      {
        item->setValue(static_cast<int>(i), getValueFromXMLElement(val, BasicType()));
      }
      else if (allowsExpressions && (nodeName == "Expression"))
      {
        smtkErrorMacro(
          logger,
          "Encountered old expression per value syntax for Item: "
            << item->name() << " of Attribute: " << item->attribute()->name()
            << " - this is no longer supported and will be ignored!");
      }
      else
      {
        smtkErrorMacro(logger, "Unsupported Value Node Type  Item: " << item->name());
      }
    }
  }
  else if ((numRequiredVals == 1) && !item->isExtensible())
  {
    // Lets see if there is an unset val element
    noVal = node.child("UnsetVal");
    if (!(noVal || node.text().empty()))
    {
      item->setValue(getValueFromXMLElement(node, BasicType()));
    }
  }
  else
  {
    smtkErrorMacro(logger, "XML Node Values is missing for Item: " << item->name());
  }
}
}; // namespace

XmlDocV1Parser::XmlDocV1Parser(ResourcePtr myResource, smtk::io::Logger& logger)
  : m_resource(myResource)
  , m_logger(logger)
{
}

XmlDocV1Parser::~XmlDocV1Parser() = default;

xml_node XmlDocV1Parser::getRootNode(xml_document& doc)
{
  xml_node amnode = doc.child("SMTK_AttributeManager");
  return amnode;
}

void XmlDocV1Parser::getCategories(
  xml_node& rootNode,
  std::set<std::string>& cats,
  std::string& defCat)
{
  xml_node cnode, node = rootNode.child("Categories");

  if (node)
  {
    // Get the default category if one is specified
    defCat = node.attribute("Default").value();
    if (!defCat.empty())
    {
      cats.insert(defCat);
    }
    for (cnode = node.first_child(); cnode; cnode = cnode.next_sibling())
    {
      if (cnode.text().empty())
      {
        continue;
      }
      cats.insert(cnode.text().get());
    }
  }
}

bool XmlDocV1Parser::canParse(pugi::xml_document& doc)
{
  // Get the attribute resource node
  xml_node amnode = doc.child("SMTK_AttributeManager");
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
  return versionNum == 1;
}

bool XmlDocV1Parser::canParse(pugi::xml_node& node)
{
  // Check the name of the node
  std::string name = node.name();
  if (name != "SMTK_AttributeManager")
  {
    return false;
  }

  pugi::xml_attribute xatt = node.attribute("Version");
  if (!xatt)
  {
    return false;
  }

  int versionNum = xatt.as_int();
  return versionNum == 1;
}

void XmlDocV1Parser::process(pugi::xml_document& doc)
{
  // Get the attribute resource node
  xml_node amnode = doc.child("SMTK_AttributeManager");

  // Check that there is content
  if (amnode.empty())
  {
    smtkWarningMacro(m_logger, "Missing SMTK_AttributeManager element");
    return;
  }

  this->process(amnode);
}

void XmlDocV1Parser::process(pugi::xml_node& amnode)
{
  std::map<std::string, std::map<std::string, std::string>>
    globalItemBlocks; // There are no global item blocks being passed in
  this->process(amnode, globalItemBlocks);
}

void XmlDocV1Parser::process(
  pugi::xml_node& amnode,
  std::map<std::string, std::map<std::string, std::string>>& globalItemBlocks)
{
  // Lets get the UUID of the resource if there is one
  auto idAtt = amnode.attribute("ID");
  if (idAtt)
  {
    std::string idName = idAtt.value();
    smtk::common::UUID uuid(idName);
    m_resource->setId(uuid);
  }

  // Clear the vectors for dealing with attribute references
  m_itemExpressionInfo.clear();
  m_attRefInfo.clear();
  xml_node node, cnode;

  // Get the category information, starting with current set
  std::set<std::string> newCats, seccategories = m_resource->categories();
  std::string defCat, s;
  this->getCategories(amnode, newCats, defCat);
  if (!defCat.empty())
  {
    m_defaultCategory = defCat;
  }
  // Add the new Categories
  seccategories.insert(newCats.begin(), newCats.end());

  // Process Analysis Info
  std::set<std::string> categories;
  node = amnode.child("Analyses");
  if (node)
  {
    attribute::Analyses& analyses = m_resource->analyses();
    xml_node anode;
    auto xatt = node.attribute("Exclusive");
    if (xatt && xatt.as_bool())
    {
      analyses.setTopLevelExclusive(true);
    }
    for (anode = node.first_child(); anode; anode = anode.next_sibling())
    {
      s = anode.attribute("Type").value();
      categories.clear();
      for (cnode = anode.first_child(); cnode; cnode = cnode.next_sibling())
      {
        if (cnode.text().empty())
        {
          continue;
        }
        categories.insert(cnode.text().get());
      }
      auto* analysis = analyses.create(s);
      if (analysis == nullptr)
      {
        smtkErrorMacro(m_logger, "Failed to create Analysis: " << s);
        continue;
      }
      analysis->setLocalCategories(categories);
      // Does this analysis have a base type?
      xatt = anode.attribute("BaseType");
      if (xatt)
      {
        const auto* bt = xatt.value();
        auto* parent = analyses.find(bt);
        if (parent == nullptr)
        {
          smtkErrorMacro(m_logger, "Failed to set Analysis: " << s << " parent to " << bt);
        }
        analysis->setParent(parent);
      }
      xatt = anode.attribute("Exclusive");
      if (xatt && xatt.as_bool())
      {
        analysis->setExclusive(true);
      }
      xatt = anode.attribute("Required");
      if (xatt && xatt.as_bool())
      {
        analysis->setRequired(true);
      }
      // Does the analysis have a label?
      xatt = anode.attribute("Label");
      if (xatt)
      {
        analysis->setLabel(xatt.value());
      }
    }
  }

  // Process AdvanceLevel Info
  node = amnode.child("AdvanceLevels");
  if (node)
  {
    xml_node anode;
    for (anode = node.first_child(); anode; anode = anode.next_sibling())
    {
      s = anode.attribute("Label").value();
      int val = anode.text().as_int();
      if (s.empty())
      {
        std::stringstream tmp;
        tmp << "Level " << val;
        s = tmp.str();
      }
      m_resource->addAdvanceLevel(val, s);

      xml_attribute xatt = anode.attribute("Color");
      if (xatt)
      {
        double color[4];
        s = xatt.value();
        if (!s.empty() && XmlDocV1Parser::decodeColorInfo(s, color) == 0)
        {
          m_resource->setAdvanceLevelColor(val, color);
        }
      }
    }
  }

  this->processItemDefinitionBlocks(amnode, globalItemBlocks);
  this->processAssociationRules(amnode);
  this->processAttributeInformation(amnode);
  this->processViews(amnode);

  // Let's finalize the definition information so local categories,
  // local advance levels, etc. get properly propagated.
  m_resource->finalizeDefinitions();

  // Lets see if there are active category information
  node = amnode.child("ActiveCategories");
  if (node)
  {
    bool enabled = false;
    std::set<std::string> cats;
    xml_attribute xatt = node.attribute("Enabled");
    if (xatt)
    {
      enabled = xatt.as_bool();
    }

    xml_node cnode;
    for (cnode = node.first_child(); cnode; cnode = cnode.next_sibling())
    {
      cats.insert(cnode.text().get());
    }
    m_resource->setActiveCategories(cats);
    m_resource->setActiveCategoriesEnabled(enabled);
  }

  // Now we need to check to see if there are any categories in the resource
  // that were not explicitly listed in the categories section
  std::set<std::string>::const_iterator it;
  const std::set<std::string>& cats = m_resource->categories();
  for (it = cats.begin(); it != cats.end(); it++)
  {
    if (seccategories.find(*it) == seccategories.end())
    {
      smtkErrorMacro(
        m_logger, "Category: " << *it << " was not listed in Resource's Category Section");
    }
  }

  this->processHints(amnode);
}

void XmlDocV1Parser::processItemDefinitionBlocks(
  xml_node& root,
  std::map<std::string, std::map<std::string, std::string>>& globalItemBlocks)
{
  std::size_t numGlobalBlocks = 0;
  xml_node child, node = root.child("ItemBlocks");
  if (!node)
  {
    // if we don't have any global items blocks we can just return
    // else we need to create an ItemBlocks node to store the global blocks
    if (globalItemBlocks.empty())
    {
      return;
    }
    node = root.append_child("ItemBlocks");
  }

  xml_attribute xnsatt = node.attribute("Namespace");
  std::string itemBlocksNamespace = (xnsatt) ? xnsatt.value() : "";
  // Lets insert the global item blocks into the doc's item blocks node
  for (const auto& namespaceMap : globalItemBlocks)
  {
    for (const auto& itemBlockDef : namespaceMap.second)
    {
      xml_document tempDoc;
      if (tempDoc.load_buffer(itemBlockDef.second.c_str(), itemBlockDef.second.size()))
      {
        xml_node globalDef = tempDoc.first_child(); // There should only be one node
        if (globalDef)
        {
          node.prepend_copy(globalDef);
          numGlobalBlocks++;
        }
        else
        {
          smtkErrorMacro(
            m_logger,
            "Could not insert global Item Block: "
              << namespaceMap.first << "::" << itemBlockDef.first << " into local item block map");
        }
      }
      else
      {
        smtkErrorMacro(
          m_logger,
          "Could not de-serialize global Item Block: "
            << namespaceMap.first << "::" << itemBlockDef.first << " into local item block map");
      }
    }
  }
  for (child = node.first_child(); child; child = child.next_sibling())
  {
    xml_attribute xatt;
    std::string blockName;
    std::string blockNamespace;
    bool hasExplicitNamespace;
    xatt = child.attribute("Name");
    if (xatt)
    {
      blockName = xatt.value();
    }
    else
    {
      smtkErrorMacro(m_logger, "Item Block is Missing Name attribute");
      continue;
    }
    // See if the item block has an explicit namespace set.  If it does not
    // it will inherit the namespace specified at the ItemBlocks node.  If that
    // node does not have one set, then we assume it is the global namespace represented by ""

    xatt = child.attribute("Namespace");
    if (xatt)
    {
      blockNamespace = xatt.value();
      hasExplicitNamespace = true;
    }
    else
    {
      blockNamespace = itemBlocksNamespace;
      hasExplicitNamespace = false;
    }
    xml_node items = child.first_child();
    if (items)
    {
      m_itemDefintionBlocks[blockNamespace][blockName] = items;
      // Are we no longer processing item blocks that were passed in?
      if (!numGlobalBlocks)
      {
        // Is the block marked for export?
        xatt = child.attribute("Export");
        if (xatt && xatt.as_bool())
        {
          // In order to retain its proper namespace association
          // the block must have an explicit namespace attribute
          // associated with it
          if (!hasExplicitNamespace)
          {
            // Add the namespace it inherited from the itemblocks node
            child.append_attribute("Namespace").set_value(itemBlocksNamespace.c_str());
          }
          // serialize the node and add it to the global map
          std::stringstream temp;
          child.print(temp);

          globalItemBlocks[blockNamespace][blockName] = temp.str();
        }
      }
    }
    else
    {
      smtkErrorMacro(m_logger, "Item Block: " << blockName << " is missing ItemDefinition node");
      continue;
    }
    // If this is an item block that we got from the global dictionary then decrement the count
    if (numGlobalBlocks)
    {
      numGlobalBlocks--;
    }
  }
}

void XmlDocV1Parser::processDefinitionInformation(xml_node& root)
{
  xml_node child, node = root.child("Definitions");
  if (node)
  {
    for (child = node.first_child(); child; child = child.next_sibling())
    {
      this->createDefinition(child);
    }
  }
}
void XmlDocV1Parser::processAttributeInformation(xml_node& root)
{
  // Process definitions first
  this->processDefinitionInformation(root);
  xml_node child, node = root.child("Attributes");
  std::size_t i;
  if (!node)
  {
    return;
  }

  for (child = node.first_child(); child; child = child.next_sibling())
  {
    this->processAttribute(child);
  }

  // At this point we have all the attributes read in so lets
  // fix up all of the attribute  references
  attribute::AttributePtr att;
  for (i = 0; i < m_itemExpressionInfo.size(); i++)
  {
    att = m_resource->findAttribute(m_itemExpressionInfo[i].expName);
    if (att)
    {
      m_itemExpressionInfo[i].item->setExpression(att);
    }
    else
    {
      smtkErrorMacro(
        m_logger,
        "Expression Attribute: " << m_itemExpressionInfo[i].expName
                                 << " is missing and required by Item : "
                                 << m_itemExpressionInfo[i].item->name());
    }
  }

  for (i = 0; i < m_attRefInfo.size(); i++)
  {
    att = m_resource->findAttribute(m_attRefInfo[i].attName);
    if (att)
    {
      m_attRefInfo[i].item->setValue(m_attRefInfo[i].pos, att);
    }
    else
    {
      smtkErrorMacro(
        m_logger,
        "Referenced Attribute: " << m_attRefInfo[i].attName << " is missing and required by Item: "
                                 << m_attRefInfo[i].item->name());
    }
  }
}

void XmlDocV1Parser::createDefinition(xml_node& defNode)
{
  attribute::DefinitionPtr def, baseDef;
  xml_attribute xatt;
  std::string type, baseType;
  xatt = defNode.attribute("Type");
  if (xatt)
  {
    type = xatt.value();
  }
  else
  {
    smtkErrorMacro(m_logger, "Definition missing Type XML Attribute");
    return;
  }
  baseType = defNode.attribute("BaseType").value();
  if (!baseType.empty())
  {
    baseDef = m_resource->findDefinition(baseType);
    if (!baseDef)
    {
      smtkErrorMacro(
        m_logger,
        "Could not find Base Definition: " << baseType << " needed to create Definition: " << type);
      return;
    }
    def = m_resource->createDefinition(type, baseDef);
  }
  else
  {
    def = m_resource->createDefinition(type);
  }
  if (!def)
  {
    if (m_reportAsError)
    {
      smtkErrorMacro(m_logger, "Definition: " << type << " already exists in the Resource");
    }
    else
    {
      smtkWarningMacro(m_logger, "Definition: " << type << " already exists in the Resource");
    }
    return;
  }
  this->processDefinition(defNode, def);
}

void XmlDocV1Parser::processDefinition(xml_node& defNode, DefinitionPtr def)
{
  xml_attribute xatt;
  xml_node node;
  // First set the include file index
  def->setIncludeIndex(m_includeIndex);
  xatt = defNode.attribute("Label");
  if (xatt)
  {
    def->setLabel(xatt.value());
  }
  xatt = defNode.attribute("Version");
  if (xatt)
  {
    def->setVersion(xatt.as_int());
  }

  xatt = defNode.attribute("Abstract");
  if (xatt)
  {
    def->setIsAbstract(xatt.as_bool());
  }

  xatt = defNode.attribute("AdvanceLevel");
  if (xatt)
  {
    def->setLocalAdvanceLevel(xatt.as_uint());
  }
  else
  {
    xatt = defNode.attribute("AdvanceReadLevel");
    if (xatt)
    {
      def->setLocalAdvanceLevel(0, xatt.as_uint());
    }
    xatt = defNode.attribute("AdvanceWriteLevel");
    if (xatt)
    {
      def->setLocalAdvanceLevel(1, xatt.as_uint());
    }
  }

  xatt = defNode.attribute("Unique");
  if (xatt)
  {
    def->setIsUnique(xatt.as_bool());
  }

  xatt = defNode.attribute("Nodal");
  if (xatt)
  {
    def->setIsNodal(xatt.as_bool());
  }

  // Read oldest association mask format first.  Note that the association is set as extensible.
  // It will be overwritten if a new-style AssociationsDef is also provided.
  xatt = defNode.attribute("Associations");
  if (xatt)
  {
    model::BitFlags mask = this->decodeModelEntityMask(xatt.value());
    def->setLocalAssociationMask(mask);
    def->localAssociationRule()->setIsExtensible(true);
  }

  double color[4];

  node = defNode.child("NotApplicableColor");
  if (node && this->getColor(node, color, "NotApplicableColor"))
  {
    def->setNotApplicableColor(color);
  }

  node = defNode.child("DefaultColor");
  if (node && this->getColor(node, color, "DefaultColor"))
  {
    def->setDefaultColor(color);
  }

  node = defNode.child("BriefDescription");
  if (node)
  {
    def->setBriefDescription(node.text().get());
  }

  node = defNode.child("DetailedDescription");
  if (node)
  {
    def->setDetailedDescription(node.text().get());
  }

  // See if we allow any associations
  node = defNode.child("AssociationsDef");
  if (node)
  {
    this->processAssociationDef(node, def);
  }

  // Now lets process its items
  xml_node itemsNode = defNode.child("ItemDefinitions");
  std::set<std::string> currentActiveBlocks;
  ItemDefinitionsHelper helper;

  helper.processItemDefinitions<DefinitionPtr>(
    this, itemsNode, def, currentActiveBlocks, def->type(), "Definition");
}

void XmlDocV1Parser::processAssociationDef(xml_node& node, DefinitionPtr def)
{
  // In V1 and V2 files the association information was based on
  // model entities only
  std::string assocName = node.attribute("Name").value();
  if (assocName.empty())
  {
    assocName = def->type() + "Associations";
  }
  ReferenceItemDefinitionPtr assocDef =
    smtk::dynamic_pointer_cast<ReferenceItemDefinition>(ReferenceItemDefinition::New(assocName));
  this->processModelEntityDef(node, assocDef);
  def->setLocalAssociationRule(assocDef);
}

void XmlDocV1Parser::processCategories(
  xml_node& node,
  Categories::Set& catSet,
  Categories::CombinationMode& inheritanceMode)
{
  attribute::Categories::Set::CombinationMode catMode;
  xml_node child;

  // The default inheritance mode is Or
  inheritanceMode = Categories::CombinationMode::Or;

  // Check for old style
  xml_attribute xatt = node.attribute("OkToInheritCategories");
  if (xatt && !xatt.as_bool())
  {
    inheritanceMode = Categories::CombinationMode::LocalOnly;
  }
  xatt = node.attribute("CategoryCheckMode");
  if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
  {
    catSet.setInclusionMode(catMode);
  }

  // This is old style
  xml_node catNodes = node.child("Categories");
  // This is the new format
  xml_node catInfoNode = node.child("CategoryInfo");
  if (catInfoNode)
  {
    // Are we inheriting categories?
    xatt = catInfoNode.attribute("Inherit");
    if (xatt && !xatt.as_bool())
    {
      inheritanceMode = Categories::CombinationMode::LocalOnly;
    }

    // Lets get the overall combination mode
    xatt = catInfoNode.attribute("Combination");
    if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
    {
      catSet.setCombinationMode(catMode);
    }
    // Get the Include set (if one exists)
    xml_node catGroup;
    catGroup = catInfoNode.child("Include");
    if (catGroup)
    {
      // Lets get the include combination mode
      xatt = catGroup.attribute("Combination");
      if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
      {
        catSet.setInclusionMode(catMode);
      }
      for (child = catGroup.first_child(); child; child = child.next_sibling())
      {
        catSet.insertInclusion(child.text().get());
      }
    }
    catGroup = catInfoNode.child("Exclude");
    if (catGroup)
    {
      // Lets get the include combination mode
      xatt = catGroup.attribute("Combination");
      if (XmlDocV1Parser::getCategoryComboMode(xatt, catMode))
      {
        catSet.setExclusionMode(catMode);
      }
      for (child = catGroup.first_child(); child; child = child.next_sibling())
      {
        catSet.insertExclusion(child.text().get());
      }
    }
  }
  else if (catNodes) // Deprecated Format
  {
    for (child = catNodes.first_child(); child; child = child.next_sibling())
    {
      catSet.insertInclusion(child.text().get());
    }
  }
}
void XmlDocV1Parser::processItemDef(xml_node& node, ItemDefinitionPtr idef)
{
  xml_attribute xatt;
  xml_node child;
  xatt = node.attribute("Label");
  if (xatt)
  {
    idef->setLabel(xatt.value());
  }
  xatt = node.attribute("Version");
  if (xatt)
  {
    idef->setVersion(xatt.as_int());
  }
  xatt = node.attribute("Optional");
  if (xatt)
  {
    idef->setIsOptional(xatt.as_bool());
    idef->setIsEnabledByDefault(node.attribute("IsEnabledByDefault").as_bool());
  }

  // Process Category Information
  Categories::CombinationMode inheritanceMode;
  this->processCategories(node, idef->localCategories(), inheritanceMode);
  idef->setCategoryInheritanceMode(inheritanceMode);
  // If the definition's local categories are empty and is not a group definition
  // and there is a default category defined - then use it
  if (
    idef->localCategories().empty() &&
    !(m_defaultCategory.empty() ||
      smtk::dynamic_pointer_cast<attribute::GroupItemDefinition>(idef)))
  {
    idef->localCategories().insertInclusion(m_defaultCategory);
  }

  // If using AdvanceLevel then we are setting
  // both read and write
  xatt = node.attribute("AdvanceLevel");
  if (xatt)
  {
    idef->setLocalAdvanceLevel(0, xatt.as_uint());
    idef->setLocalAdvanceLevel(1, xatt.as_uint());
  }
  else
  {
    xatt = node.attribute("AdvanceReadLevel");
    if (xatt)
    {
      idef->setLocalAdvanceLevel(0, xatt.as_uint());
    }
    xatt = node.attribute("AdvanceWriteLevel");
    if (xatt)
    {
      idef->setLocalAdvanceLevel(1, xatt.as_uint());
    }
  }

  child = node.child("BriefDescription");
  if (child)
  {
    idef->setBriefDescription(child.text().get());
  }

  child = node.child("DetailedDescription");
  if (child)
  {
    idef->setDetailedDescription(child.text().get());
  }
}

void XmlDocV1Parser::processDoubleDef(pugi::xml_node& node, attribute::DoubleItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  processDerivedValueDef<attribute::DoubleItemDefinitionPtr, double>(node, idef, m_logger);
}

void XmlDocV1Parser::processIntDef(pugi::xml_node& node, attribute::IntItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  processDerivedValueDef<attribute::IntItemDefinitionPtr, int>(node, idef, m_logger);
}

void XmlDocV1Parser::processStringDef(pugi::xml_node& node, attribute::StringItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  if (xml_attribute xatt = node.attribute("MultipleLines"))
  {
    (void)xatt;
    idef->setIsMultiline(true);
  }
  processDerivedValueDef<attribute::StringItemDefinitionPtr, std::string>(node, idef, m_logger);
}

void XmlDocV1Parser::processModelEntityDef(
  pugi::xml_node& node,
  attribute::ReferenceItemDefinitionPtr idef)
{
  xml_node labels, mmask, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);
  mmask = node.child("MembershipMask");
  if (mmask)
  {
    idef->setAcceptsEntries("smtk::model::Resource", mmask.text().as_string(), true);
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
    smtkErrorMacro(m_logger, "Labels has been changed to ComponentLabels : " << idef->name());
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

void XmlDocV1Parser::processMeshEntityDef(
  pugi::xml_node& node,
  attribute::ComponentItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(
    m_logger,
    "The Mesh Entity defs should only be availabe starting Attribute Version 2 Format"
      << idef->name());
}

void XmlDocV1Parser::processDateTimeDef(
  pugi::xml_node& node,
  attribute::DateTimeItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(
    m_logger,
    "DateTime item defs only supported starting Attribute Version 3 Format" << idef->name());
}

void XmlDocV1Parser::processReferenceDef(
  pugi::xml_node& node,
  ReferenceItemDefinitionPtr idef,
  const std::string& lbl)
{
  (void)node;
  (void)lbl;
  smtkWarningMacro(
    m_logger,
    "Resource item defs only supported starting Attribute Version 3 Format" << idef->name());
}

void XmlDocV1Parser::processResourceDef(
  pugi::xml_node& node,
  attribute::ResourceItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(
    m_logger,
    "Resource item defs only supported starting Attribute Version 3 Format" << idef->name());
}

void XmlDocV1Parser::processComponentDef(
  pugi::xml_node& node,
  attribute::ComponentItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(
    m_logger,
    "Component item defs only supported starting Attribute Version 3 Format" << idef->name());
}

void XmlDocV1Parser::processValueDef(pugi::xml_node& node, attribute::ValueItemDefinitionPtr idef)
{
  xml_node labels, child;
  xml_attribute xatt;
  std::size_t i;
  this->processItemDef(node, idef);

  xatt = node.attribute("NumberOfRequiredValues");
  std::size_t numberOfComponents = idef->numberOfRequiredValues();
  if (xatt)
  {
    numberOfComponents = xatt.as_uint();
    idef->setNumberOfRequiredValues(numberOfComponents);
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
    smtkErrorMacro(m_logger, "Labels has been changed to ComponentLabels : " << idef->name());
  }
  labels = node.child("ComponentLabels");
  if (labels)
  {
    if ((numberOfComponents == 1) && !idef->isExtensible())
    {
      smtkErrorMacro(
        m_logger,
        "Should not use ComponentLabels when NumberOfRequiredValues=1 : " << idef->name());
    }

    // Are we using a common label?
    xatt = labels.attribute("CommonLabel");
    if (xatt)
    {
      idef->setCommonValueLabel(xatt.value());
      if (labels.first_child())
      {
        smtkErrorMacro(
          m_logger, "Cannot combine CommonLabel with Label child nodes : " << idef->name());
      }
    }
    else
    {
      for (child = labels.first_child(), i = 0; child; child = child.next_sibling(), i++)
      {
        if (i < numberOfComponents)
        {
          idef->setValueLabel(i, child.text().get());
        }
      }
      if (i != numberOfComponents)
      {
        smtkErrorMacro(m_logger, "Wrong number of component values for : " << idef->name());
      }
    }
  }
  child = node.child("ExpressionType");
  if (child)
  {
    std::string etype = child.text().get();
    idef->setExpressionType(etype);
  }
  xatt = node.attribute("Units");
  if (xatt)
  {
    idef->setUnits(xatt.value());
  }

  // Now lets process its children items
  xml_node cinode, citemsNode = node.child("ChildrenDefinitions");

  ItemDefinitionsHelper helper;
  std::set<std::string> currentActiveBlocks;

  helper.processItemDefinitions<ValueItemDefinitionPtr>(
    this, citemsNode, idef, currentActiveBlocks, idef->name(), "ValueItemDefinition");
}

void XmlDocV1Parser::processRefDef(pugi::xml_node& node, attribute::ComponentItemDefinitionPtr idef)
{
  xml_node labels, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);

  // Has the attribute definition been set?
  child = node.child("AttDef");
  if (child)
  {
    std::string etype = child.text().get();
    std::string a = Resource::createAttributeQuery(etype);
    idef->setAcceptsEntries(smtk::common::typeName<attribute::Resource>(), a, true);
  }

  xatt = node.attribute("NumberOfRequiredValues");
  if (xatt)
  {
    idef->setNumberOfRequiredValues(xatt.as_int());
  }

  // Lets see if there are labels
  if (node.child("Labels"))
  {
    smtkErrorMacro(m_logger, "Labels has been changed to ComponentLabels : " << idef->name());
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

void XmlDocV1Parser::processDirectoryDef(
  pugi::xml_node& node,
  attribute::DirectoryItemDefinitionPtr idef)
{
  xml_node labels, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);

  xatt = node.attribute("NumberOfRequiredValues");
  if (xatt)
  {
    idef->setNumberOfRequiredValues(xatt.as_int());
  }

  xatt = node.attribute("ShouldExist");
  if (xatt)
  {
    idef->setShouldExist(xatt.as_bool());
  }

  xatt = node.attribute("ShouldBeRelative");
  if (xatt)
  {
    idef->setShouldBeRelative(xatt.as_bool());
  }

  // Lets see if there are labels
  labels = node.child("Labels");
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

void XmlDocV1Parser::processFileDef(pugi::xml_node& node, attribute::FileItemDefinitionPtr idef)
{
  xml_node labels, defaultNode, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);

  xatt = node.attribute("NumberOfRequiredValues");
  if (xatt)
  {
    idef->setNumberOfRequiredValues(xatt.as_int());
  }

  xatt = node.attribute("ShouldExist");
  if (xatt)
  {
    idef->setShouldExist(xatt.as_bool());
  }

  xatt = node.attribute("ShouldBeRelative");
  if (xatt)
  {
    idef->setShouldBeRelative(xatt.as_bool());
  }

  // Lets see if there are labels
  if (node.child("Labels"))
  {
    smtkErrorMacro(m_logger, "Labels has been changed to ComponentLabels : " << idef->name());
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

  // Check for filters
  xatt = node.attribute("FileFilters");
  if (xatt)
  {
    idef->setFileFilters(xatt.as_string());
  }

  // Check for default value
  defaultNode = node.child("DefaultValue");
  if (defaultNode)
  {
    idef->setDefaultValue(defaultNode.text().get());
  }
}

void XmlDocV1Parser::processGroupDef(pugi::xml_node& node, attribute::GroupItemDefinitionPtr def)
{
  xml_node labels, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, def);

  xatt = node.attribute("NumberOfRequiredGroups");
  if (xatt)
  {
    def->setNumberOfRequiredGroups(xatt.as_uint());
  }

  xatt = node.attribute("Extensible");
  if (xatt)
  {
    def->setIsExtensible(xatt.as_bool());
    xatt = node.attribute("MaxNumberOfGroups");
    if (xatt)
    {
      def->setMaxNumberOfGroups(xatt.as_uint());
    }
  }

  xatt = node.attribute("IsConditional");
  if (xatt)
  {
    bool isConditional = xatt.as_bool();
    def->setIsConditional(isConditional);
    if (isConditional)
    {
      xatt = node.attribute("MinNumberOfChoices");
      if (xatt)
      {
        def->setMinNumberOfChoices(xatt.as_uint());
      }
      xatt = node.attribute("MaxNumberOfChoices");
      if (xatt)
      {
        def->setMaxNumberOfChoices(xatt.as_uint());
      }
    }
  }
  // Lets see if there are labels
  if (node.child("Labels"))
  {
    smtkErrorMacro(m_logger, "Labels has been changed to ComponentLabels : " << def->name());
  }
  labels = node.child("ComponentLabels");
  if (labels)
  {
    // Are we using a common label?
    xatt = labels.attribute("CommonLabel");
    if (xatt)
    {
      def->setCommonSubGroupLabel(xatt.value());
    }
    else
    {
      for (child = labels.first_child(), i = 0; child; child = child.next_sibling(), i++)
      {
        def->setSubGroupLabel(i, child.text().get());
      }
    }
  }
  xml_node itemsNode = node.child("ItemDefinitions");

  ItemDefinitionsHelper helper;
  std::set<std::string> currentActiveBlocks;

  helper.processItemDefinitions<GroupItemDefinitionPtr>(
    this, itemsNode, def, currentActiveBlocks, def->name(), "GroupItemDefinition");
}

smtk::common::UUID XmlDocV1Parser::getAttributeID(xml_node& attNode)
{
  // In Version 1 Format we didn't use UIDs so we need to give
  // attributes new IDs
  xml_attribute xatt;
  std::string name;

  xatt = attNode.attribute("Name");
  if (xatt)
  {
    name = xatt.value();
  }
  smtkWarningMacro(
    m_logger, "Attribute: " << name << " is being assigned a new ID (Version 1 Format)!");

  return smtk::common::UUID::null();
}

void XmlDocV1Parser::processAttribute(xml_node& attNode)
{
  xml_node itemsNode, assocsNode, iNode, node;
  std::string name, type;
  xml_attribute xatt;
  attribute::AttributePtr att;
  attribute::DefinitionPtr def;
  smtk::common::UUID id;
  int i, n;

  xatt = attNode.attribute("Name");
  if (!xatt)
  {
    smtkErrorMacro(m_logger, "Invalid Attribute! - Missing XML Attribute Name");
    return;
  }
  name = xatt.value();
  xatt = attNode.attribute("Type");
  if (!xatt)
  {
    smtkErrorMacro(m_logger, "Invalid Attribute: " << name << "  - Missing XML Attribute Type");
    return;
  }
  type = xatt.value();

  id = this->getAttributeID(attNode);

  def = m_resource->findDefinition(type);
  if (!def)
  {
    smtkErrorMacro(
      m_logger,
      "Attribute: " << name << " of Type: " << type << "  - can not find attribute definition");
    return;
  }

  // Is the definition abstract?
  if (def->isAbstract())
  {
    smtkErrorMacro(
      m_logger,
      "Attribute: " << name << " of Type: " << type << "  - is based on an abstract definition");
    return;
  }

  // Do we have a valid uuid?
  if (id.isNull())
  {
    att = m_resource->createAttribute(name, def);
  }
  else
  {
    att = m_resource->createAttribute(name, def, id);
  }

  if (!att)
  {
    smtkErrorMacro(
      m_logger,
      "Attribute: " << name << " of Type: " << type
                    << "  - could not be created - is the name in use");
    return;
  }

  // Set the attribute include index
  att->setIncludeIndex(m_includeIndex);
  xatt = attNode.attribute("OnInteriorNodes");
  if (xatt)
  {
    att->setAppliesToInteriorNodes(xatt.as_bool());
  }

  xatt = attNode.attribute("OnBoundaryNodes");
  if (xatt)
  {
    att->setAppliesToBoundaryNodes(xatt.as_bool());
  }

  double color[4];

  node = attNode.child("Color");
  if (node && this->getColor(node, color, "Color"))
  {
    att->setColor(color);
  }

  // If using AdvanceLevel then we are setting
  // both read and write
  xatt = attNode.attribute("AdvanceLevel");
  if (xatt)
  {
    att->setLocalAdvanceLevel(0, xatt.as_uint());
    att->setLocalAdvanceLevel(1, xatt.as_uint());
  }
  else
  {
    xatt = attNode.attribute("AdvanceReadLevel");
    if (xatt)
    {
      att->setLocalAdvanceLevel(0, xatt.as_uint());
    }
    xatt = attNode.attribute("AdvanceWriteLevel");
    if (xatt)
    {
      att->setLocalAdvanceLevel(1, xatt.as_uint());
    }
  }
  itemsNode = attNode.child("Items");
  if (itemsNode)
  {
    // Process all of the items in the attribute w/r to the XML
    // NOTE That the writer processes the items in order - lets assume
    // that for speed and if that fails we can try to search for the correct
    // xml node
    n = static_cast<int>(att->numberOfItems());
    for (i = 0, iNode = itemsNode.first_child(); (i < n) && iNode;
         i++, iNode = iNode.next_sibling())
    {
      // See if the name of the item matches the name of node
      xatt = iNode.attribute("Name");
      if (!xatt)
      {
        smtkErrorMacro(
          m_logger, "Bad Item for Attribute : " << name << "- missing XML Attribute Name");
        node = itemsNode.find_child_by_attribute("Name", att->item(i)->name().c_str());
      }
      else
      {
        // Is the ith xml node the same as the ith item of the attribute?
        if (att->item(i)->name() == xatt.value())
        {
          node = iNode;
        }
        else
        {
          node = itemsNode.find_child_by_attribute("Name", att->item(i)->name().c_str());
        }
      }
      if (!node)
      {
        smtkErrorMacro(
          m_logger,
          "Can not locate XML Item node :" << att->item(i)->name() << " for Attribute : " << name);
        continue;
      }
      this->processItem(node, att->item(i));
    }
    if (iNode || (i != n))
    {
      smtkErrorMacro(m_logger, "Number of Items does not match XML for Attribute : " << name);
    }
  }

  assocsNode = attNode.child("Associations");
  if (assocsNode)
  {
    ReferenceItem::Ptr assocItem = att->associatedObjects();
    this->processItem(assocsNode, assocItem);
    // Now the ReferenceItem is deserialized but we need
    // to let the referenced objects know about the associations.
    for (auto it = assocItem->begin(); it != assocItem->end(); ++it)
    {
      if (!it.isSet())
      {
        continue;
      }
      auto mcomp = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
      auto mmgr = mcomp ? mcomp->modelResource() : smtk::model::ResourcePtr();
      if (mcomp && mmgr)
      {
        mmgr->associateAttribute(nullptr, att->id(), mcomp->id());
      }
    }
  }
}

void XmlDocV1Parser::processItem(xml_node& node, ItemPtr item)
{
  xml_attribute xatt;
  // See if there is enabled info - note that since we have added
  // forceRequired, we can't just look at the isOptional information.
  xatt = node.attribute("Enabled");
  if (xatt)
  {
    item->setIsEnabled(xatt.as_bool());
  }

  // If using AdvanceLevel then we are setting
  // both read and write
  xatt = node.attribute("AdvanceLevel");
  if (xatt)
  {
    item->setLocalAdvanceLevel(0, xatt.as_uint());
    item->setLocalAdvanceLevel(1, xatt.as_uint());
  }
  else
  {
    xatt = node.attribute("AdvanceReadLevel");
    if (xatt)
    {
      item->setLocalAdvanceLevel(0, xatt.as_uint());
    }
    xatt = node.attribute("AdvanceWriteLevel");
    if (xatt)
    {
      item->setLocalAdvanceLevel(1, xatt.as_uint());
    }
  }

  switch (item->type())
  {
    case Item::AttributeRefType:
      this->processRefItem(node, smtk::dynamic_pointer_cast<ComponentItem>(item));
      break;
    case Item::DoubleType:
      this->processDoubleItem(node, smtk::dynamic_pointer_cast<DoubleItem>(item));
      break;
    case Item::DirectoryType:
      this->processDirectoryItem(node, smtk::dynamic_pointer_cast<DirectoryItem>(item));
      break;
    case Item::FileType:
      this->processFileItem(node, smtk::dynamic_pointer_cast<FileItem>(item));
      break;
    case Item::GroupType:
      this->processGroupItem(node, smtk::dynamic_pointer_cast<GroupItem>(item));
      break;
    case Item::IntType:
      this->processIntItem(node, smtk::dynamic_pointer_cast<IntItem>(item));
      break;
    case Item::StringType:
      this->processStringItem(node, smtk::dynamic_pointer_cast<StringItem>(item));
      break;
    case Item::ModelEntityType:
      this->processModelEntityItem(node, smtk::dynamic_pointer_cast<ComponentItem>(item));
      break;
    case Item::MeshEntityType:
      this->processMeshEntityItem(node, smtk::dynamic_pointer_cast<ComponentItem>(item));
      break;
    case Item::DateTimeType:
      this->processDateTimeItem(node, smtk::dynamic_pointer_cast<DateTimeItem>(item));
      break;
    case Item::ReferenceType:
      this->processReferenceItem(node, smtk::dynamic_pointer_cast<ReferenceItem>(item));
      break;
    case Item::ResourceType:
      this->processResourceItem(node, smtk::dynamic_pointer_cast<ResourceItem>(item));
      break;
    case Item::ComponentType:
      this->processComponentItem(node, smtk::dynamic_pointer_cast<ComponentItem>(item));
      break;
    case Item::VoidType:
      // Nothing to do!
      break;
    default:
    {
      if (
        smtk::attribute::CustomItemBase* citem =
          dynamic_cast<smtk::attribute::CustomItemBase*>(item.get()))
      {
        (*citem) << node;
      }
      else
      {
        smtkErrorMacro(m_logger, "Unsupported Item Type: " << Item::type2String(item->type()));
      }
    }
  }
}

void XmlDocV1Parser::processValueItem(pugi::xml_node& node, attribute::ValueItemPtr item)
{
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  std::size_t i = 0, n = item->numberOfValues();
  xml_attribute xatt;
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

  if (!item->isDiscrete())
  {
    return; // there is nothing to be done
  }

  // OK Time to process the children items of this Discrete Item
  xml_node val, values, childNode, childrenNodes, inode;
  childrenNodes = node.child("ChildrenItems");
  if (childrenNodes)
  {
    // Process all of the children items in the item w/r to the XML
    // NOTE That the writer processes the items in order - lets assume
    // that for speed and if that fails we can try to search for the correct
    // xml node
    std::map<std::string, ItemPtr>::const_iterator iter;
    const std::map<std::string, ItemPtr>& childrenItems = item->childrenItems();
    for (childNode = childrenNodes.first_child(), iter = childrenItems.begin();
         (iter != childrenItems.end()) && childNode;
         iter++, childNode = childNode.next_sibling())
    {
      // See if the name of the item matches the name of node
      xatt = childNode.attribute("Name");
      if (!xatt)
      {
        smtkErrorMacro(
          m_logger, "Bad Child Item for Item : " << item->name() << "- missing XML Attribute Name");
        inode = childrenNodes.find_child_by_attribute("Name", iter->second->name().c_str());
      }
      else
      {
        // Is the ith xml node the same as the ith item of the attribute?
        if (iter->second->name() == xatt.value())
        {
          inode = childNode;
        }
        else
        {
          inode = childrenNodes.find_child_by_attribute("Name", iter->second->name().c_str());
        }
      }
      if (!inode)
      {
        smtkErrorMacro(
          m_logger,
          "Can not locate XML Child Item node :" << iter->second->name()
                                                 << " for Item : " << item->name());
        continue;
      }
      this->processItem(inode, iter->second);
    }
    if (childNode || (iter != childrenItems.end()))
    {
      smtkErrorMacro(
        m_logger, "Number of Children Items does not match XML for Item : " << item->name());
    }
  }
  int index = 0;
  if (!n)
  {
    return;
  }
  // There are 2 possible formats - a general one that must be used when n > 1
  // and a special compact one that could be used for n == 1
  // Lets check the general one first - note we only need to process the values
  // that have been set
  values = node.child("DiscreteValues");
  if (values)
  {
    for (val = values.child("Index"); val; val = val.next_sibling("Index"))
    {
      xatt = val.attribute("Ith");
      if (!xatt)
      {
        smtkErrorMacro(m_logger, "XML Attribute Ith is missing for Item: " << item->name());
        continue;
      }
      i = xatt.as_int();
      if (i >= n)
      {
        smtkErrorMacro(
          m_logger,
          "XML Attribute Ith = " << i << " and is out of range for Item: " << item->name());
        continue;
      }
      index = val.text().as_int();
      if (!item->setDiscreteIndex(static_cast<int>(i), index))
      {
        smtkErrorMacro(
          m_logger,
          "Discrete Index " << index << " for  ith value : " << i
                            << " is not valid for Item: " << item->name());
      }
    }
    return;
  }
  if (numRequiredVals == 1) // Special Common Case
  {
    if (!node.text().empty())
    {
      if (!item->setDiscreteIndex(node.text().as_int()))
      {
        smtkErrorMacro(
          m_logger,
          "Discrete Index " << index << " for  ith value : " << i
                            << " is not valid for Item: " << item->name());
      }
    }
    return;
  }
  smtkErrorMacro(m_logger, "Missing Discrete Values for Item: " << item->name());
}

void XmlDocV1Parser::processRefItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  std::string attName;
  attribute::AttributePtr att;
  AttRefInfo info;
  if (!numRequiredVals)
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
      attName = val.text().get();
      att = m_resource->findAttribute(attName);
      if (!att)
      {
        info.item = item;
        info.pos = static_cast<int>(i);
        info.attName = attName;
        m_attRefInfo.push_back(info);
      }
      else
      {
        item->setValue(static_cast<int>(i), att);
      }
    }
  }
  else if (numRequiredVals == 1)
  {
    val = node.child("Val");
    if (val)
    {
      attName = val.text().get();
      att = m_resource->findAttribute(attName);
      if (!att)
      {
        info.item = item;
        info.pos = 0;
        info.attName = attName;
        m_attRefInfo.push_back(info);
      }
      else
      {
        item->setValue(att);
      }
    }
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node Values is missing for Item: " << item->name());
  }
}

void XmlDocV1Parser::processDirectoryItem(pugi::xml_node& node, attribute::DirectoryItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  if (!numRequiredVals)
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

void XmlDocV1Parser::processDoubleItem(pugi::xml_node& node, attribute::DoubleItemPtr item)
{
  this->processValueItem(node, dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::DoubleItemPtr, double>(
    node, item, m_resource, m_itemExpressionInfo, m_logger);
}

void XmlDocV1Parser::processIntItem(pugi::xml_node& node, attribute::IntItemPtr item)
{
  this->processValueItem(node, dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::IntItemPtr, int>(
    node, item, m_resource, m_itemExpressionInfo, m_logger);
}

void XmlDocV1Parser::processStringItem(pugi::xml_node& node, attribute::StringItemPtr item)
{
  this->processValueItem(node, dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::StringItemPtr, std::string>(
    node, item, m_resource, m_itemExpressionInfo, m_logger);
}

void XmlDocV1Parser::processModelEntityItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  (void)node;
  smtkWarningMacro(
    m_logger,
    "All Model Entity Items will be ignored for Attribute Version 1 Format" << item->name());
  return;
}

void XmlDocV1Parser::processMeshEntityItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  (void)node;
  smtkWarningMacro(
    m_logger,
    "All Mesh Entity Items will be ignored for Attribute Version 1, 2, & 3 Format" << item->name());
}

void XmlDocV1Parser::processDateTimeItem(pugi::xml_node& node, attribute::DateTimeItemPtr item)
{
  (void)node;
  smtkWarningMacro(
    m_logger, "DateTime items only supported starting Attribute Version 3 Format" << item->name());
}

void XmlDocV1Parser::processReferenceItem(pugi::xml_node& node, ReferenceItemPtr item)
{
  (void)node;
  smtkWarningMacro(
    m_logger, "Reference items only supported starting Attribute Version 3 Format" << item->name());
}

void XmlDocV1Parser::processResourceItem(pugi::xml_node& node, attribute::ResourceItemPtr item)
{
  (void)node;
  smtkWarningMacro(
    m_logger, "Resource items only supported starting Attribute Version 3 Format" << item->name());
}

void XmlDocV1Parser::processComponentItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  (void)node;
  smtkWarningMacro(
    m_logger, "Component items only supported starting Attribute Version 3 Format" << item->name());
}

void XmlDocV1Parser::processFileItem(pugi::xml_node& node, attribute::FileItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  if (!numRequiredVals)
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

void XmlDocV1Parser::processGroupItem(pugi::xml_node& node, attribute::GroupItemPtr item)
{
  std::size_t i, j, m, n;
  std::size_t numRequiredGroups = item->numberOfRequiredGroups();
  xml_node itemNode;
  xml_attribute xatt;

  if (item->isConditional())
  {
    xatt = node.attribute("MinNumberOfChoices");
    if (xatt)
    {
      item->setMinNumberOfChoices(xatt.as_uint());
    }
    xatt = node.attribute("MaxNumberOfChoices");
    if (xatt)
    {
      item->setMaxNumberOfChoices(xatt.as_uint());
    }
  }

  n = item->numberOfGroups();
  m = item->numberOfItemsPerGroup();
  if (item->isExtensible())
  {
    // If node has no children, then number of groups is zero
    if (!node.first_child())
    {
      n = 0;
    }
    else
    {
      // The node should have an attribute indicating how many groups are
      // associated with the item
      xatt = node.attribute("NumberOfGroups");
      if (!xatt)
      {
        smtkErrorMacro(
          m_logger, "XML Attribute NumberOfGroups is missing for Group Item: " << item->name());
        return;
      }
      n = xatt.as_uint();
    }

    if (!item->setNumberOfGroups(n))
    {
      smtkErrorMacro(m_logger, "Invalid number of sub-groups for Group Item: " << item->name());
      return;
    }
  }

  if (!n) // There are no sub-groups for this item
  {
    return;
  }
  // There are 2 formats - one is for any number of sub groups and the other
  // is a custom case is for 1 subGroup
  xml_node cluster, clusters = node.child("GroupClusters");
  if (clusters)
  {
    for (cluster = clusters.first_child(), i = 0; cluster; cluster = cluster.next_sibling(), ++i)
    {
      if (i >= n)
      {
        smtkErrorMacro(m_logger, "Too many sub-groups for Group Item: " << item->name());
        continue;
      }
      for (itemNode = cluster.first_child(), j = 0; itemNode;
           itemNode = itemNode.next_sibling(), j++)
      {
        if (j >= m)
        {
          smtkErrorMacro(
            m_logger,
            "Too many item nodes for subGroup: " << i << " for Group Item: " << item->name());
          continue;
        }
        this->processItem(itemNode, item->item(static_cast<int>(i), static_cast<int>(j)));
      }
    }
  }
  else if (numRequiredGroups == 1)
  {
    for (itemNode = node.first_child(), j = 0; itemNode; itemNode = itemNode.next_sibling(), j++)
    {
      if (j >= m)
      {
        smtkErrorMacro(
          m_logger,
          "Too many item nodes for subGroup: 0"
            << " for Group Item: " << item->name());
        continue;
      }
      this->processItem(itemNode, item->item(static_cast<int>(j)));
    }
  }
  else
  {
    smtkErrorMacro(m_logger, "XML Node GroupClusters is missing for Item: " << item->name());
  }
}

bool XmlDocV1Parser::getColor(xml_node& node, double color[4], const std::string& colorName)
{
  std::string s = node.text().get();
  if (s.empty())
  {
    smtkErrorMacro(m_logger, "Color Format Problem - empty input for " << colorName);
    return false;
  }

  int i = XmlDocV1Parser::decodeColorInfo(s, color);
  if (i)
  {
    smtkErrorMacro(
      m_logger,
      "Color Format Problem - only found " << 4 - i << " components for " << colorName
                                           << " from string " << s);
    return false;
  }
  return true;
}

void XmlDocV1Parser::processViews(xml_node& root)
{
  xml_node views = root.child("RootView");
  if (!views)
  {
    return;
  }
  smtk::view::ConfigurationPtr rootView = this->createView(views, "Group");
  rootView->details().setAttribute("TopLevel", "true");

  if (!rootView)
  {
    smtkErrorMacro(m_logger, "Can't process Root View");
    return;
  }

  xml_node node;
  xml_attribute xatt;
  node = views.child("DefaultColor");
  if (node)
  {
    rootView->details().addChild("DefaultColor").setContents(node.text().get());
  }
  node = views.child("InvalidColor");
  if (node)
  {
    rootView->details().addChild("InvalidColor").setContents(node.text().get());
  }

  node = views.child("AdvancedFontEffects");
  if (node)
  {
    smtk::view::Configuration::Component comp = rootView->details().addChild("AdvancedFontEffects");
    if (node.attribute("Bold"))
    {
      comp.setAttribute("Bold", "t");
    }
    if (node.attribute("Italic"))
    {
      comp.setAttribute("Italic", "t");
    }
  }
  node = views.child("MaxValueLabelLength");
  if (node)
  {
    rootView->details().addChild("MaxValueLabelLength").setContents(node.text().get());
  }
  node = views.child("MinValueLabelLength");
  if (node)
  {
    rootView->details().addChild("MinValueLabelLength").setContents(node.text().get());
  }

  this->processGroupView(views, rootView);
}

void XmlDocV1Parser::processAttributeView(xml_node& node, smtk::view::ConfigurationPtr view)
{
  xml_attribute xatt;
  attribute::DefinitionPtr def;
  xml_node child, attTypes;
  std::string defType;
  xatt = node.attribute("ModelEntityFilter");
  if (xatt)
  {
    view->details().setAttribute("ModelEntityFilter", xatt.value());

    xatt = node.attribute("CreateEntities");
    if (xatt)
    {
      view->details().setAttribute("CreateEntities", xatt.value());
    }
  }
  attTypes = node.child("AttributeTypes");
  if (!attTypes)
  {
    return;
  }
  smtk::view::Configuration::Component& comp = view->details().addChild("AttributeTypes");

  for (child = attTypes.child("Type"); child; child = child.next_sibling("Type"))
  {
    comp.addChild("Att").setAttribute("Type", child.text().get());
  }
}

void XmlDocV1Parser::processInstancedView(xml_node& node, smtk::view::ConfigurationPtr view)
{
  xml_attribute xatt;
  xml_node child, instances = node.child("InstancedAttributes");

  if (!instances)
  {
    return; // No instances are in the view
  }
  smtk::view::Configuration::Component& comp = view->details().addChild("InstancedAttributes");
  for (child = instances.child("Att"); child; child = child.next_sibling("Att"))
  {
    xatt = child.attribute("Type");
    if (!xatt)
    {
      smtkErrorMacro(
        m_logger,
        "XML Attribute Type is missing"
          << "and is required to create attribute: " << child.text().get()
          << " for Instanced View: " << view->name());
      continue;
    }

    comp.addChild("Att")
      .setAttribute("Type", xatt.value())
      .setAttribute("Name", child.text().get());
  }
}

void XmlDocV1Parser::processModelEntityView(xml_node& node, smtk::view::ConfigurationPtr view)
{
  xml_attribute xatt = node.attribute("ModelEntityFilter");
  xml_node child = node.child("Definition");
  if (xatt)
  {
    view->details().setAttribute("ModelEntityFilter", xatt.value());
  }

  if (child)
  {
    view->details().addChild("Type").setContents(child.text().get());
  }
}

void XmlDocV1Parser::processSimpleExpressionView(xml_node& node, smtk::view::ConfigurationPtr view)
{
  xml_node child = node.child("Definition");
  if (child)
  {
    view->details().addChild("Att").setAttribute("Type", child.text().get());
  }
}

void XmlDocV1Parser::processGroupView(xml_node& node, smtk::view::ConfigurationPtr group)
{
  // Group style (Optional), Tabbed (default) or Tiled
  xml_attribute xatt;
  xatt = node.attribute("Style");
  if (xatt)
  {
    group->details().setAttribute("Style", xatt.value());
  }

  // Add Views Component
  smtk::view::Configuration::Component& vcomp = group->details().addChild("Views");
  xml_node child;
  smtk::view::ConfigurationPtr childView;
  std::string childName;
  for (child = node.first_child(); child; child = child.next_sibling())
  {
    childName = child.name();
    if (childName == "AttributeView")
    {
      childView = this->createView(child, "Attribute");
      if (childView)
      {
        vcomp.addChild("View").setAttribute("Title", childView->name());
        this->processAttributeView(child, childView);
      }
      continue;
    }

    if (childName == "GroupView")
    {
      childView = this->createView(child, "Group");
      if (childView)
      {
        vcomp.addChild("View").setAttribute("Title", childView->name());
        this->processGroupView(child, childView);
      }
      continue;
    }

    if (childName == "InstancedView")
    {
      childView = this->createView(child, "Instanced");
      if (childView)
      {
        vcomp.addChild("View").setAttribute("Title", childView->name());
        this->processInstancedView(child, childView);
      }
      continue;
    }

    if (childName == "ModelEntityView")
    {
      childView = this->createView(child, "ModelEntity");
      if (childView)
      {
        vcomp.addChild("View").setAttribute("Title", childView->name());
        this->processModelEntityView(child, childView);
      }
      continue;
    }

    if (childName == "SimpleExpressionView")
    {
      childView = this->createView(child, "SimpleExpression");
      if (childView)
      {
        vcomp.addChild("View").setAttribute("Title", childView->name());
        this->processSimpleExpressionView(child, childView);
      }
      continue;
    }
  }
}

smtk::view::ConfigurationPtr XmlDocV1Parser::createView(xml_node& node, const std::string& viewType)
{
  xml_attribute xatt;
  std::string val;
  xatt = node.attribute("Title"); // Required
  if (!xatt)
  {
    smtkErrorMacro(m_logger, "View is missing XML Attribute Title");
    smtk::view::ConfigurationPtr dummy;
    return dummy;
  }
  val = xatt.value();
  smtk::view::ConfigurationPtr view = smtk::view::Configuration::New(viewType, val);

  xatt = node.attribute("Icon"); // optional
  if (xatt)
  {
    val = xatt.value();
    view->details().setAttribute("Icon", val);
  }
  //set the include index
  view->setIncludeIndex(m_includeIndex);
  m_resource->addView(view);
  return view;
}

int XmlDocV1Parser::decodeColorInfo(const std::string& s, double* color)
{
  // Assume that the string is seperated by spaces and or commas
  std::istringstream iss(s);
  int i = 0;
  char c;
  while (!((i == 4) || iss.eof()))
  {
    iss.get(c);
    if ((c == ' ') || (c == ','))
    {
      continue;
    }
    iss.putback(c);
    iss >> color[i];
    i++;
  }
  return 4 - i; // If we processed all the components this would be 0
}

smtk::model::BitFlags XmlDocV1Parser::decodeModelEntityMask(const std::string& s)
{
  smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(s);
  if (!s.empty() && flags == 0 && s != "none" && s != "none|nodim")
  {
    smtkErrorMacro(
      m_logger, "Decoding Model Entity Mask - Option \"" << s << "\" is not supported");
  }
  return flags;
}

bool XmlDocV1Parser::getCategoryComboMode(
  pugi::xml_attribute& xmlAtt,
  smtk::attribute::Categories::Set::CombinationMode& mode)
{
  if (xmlAtt)
  {
    std::string val = xmlAtt.value();
    if (smtk::attribute::Categories::combinationModeFromString(val, mode))
    {
      return true;
    }
  }
  return false;
}

void XmlDocV1Parser::processHints(pugi::xml_node& root)
{
  (void)root;
  if (m_resource)
  {
    // By default, old readers will make their resources visible.
    // Starting with V5, this property will only be added if an
    // XML attribute ("DisplayHint") exists on the root document node.
    m_resource->properties().get<bool>()["smtk.attribute_panel.display_hint"] = true;
  }
}
