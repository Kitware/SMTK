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
#include "pugixml/src/pugixml.cpp"

#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/common/DateTimeZonePair.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Manager.h"

#include "smtk/common/StringUtil.h"
#include "smtk/common/View.h"

#include <iostream>
#include <algorithm>

using namespace pugi;
using namespace smtk::io;
using namespace smtk::common;
using namespace smtk;

// Some helper functions
namespace {

  int getValueFromXMLElement(xml_node &node, int)
  {
    return node.text().as_int();
  }

//----------------------------------------------------------------------------
  double getValueFromXMLElement(xml_node &node, double)
  {
    return node.text().as_double();
  }

//----------------------------------------------------------------------------
  const char *getValueFromXMLElement(xml_node &node, std::string)
  {
    return node.text().get();
  }

//----------------------------------------------------------------------------
  std::vector<int> getValueFromXMLElement(xml_node &node, const std::string& sep, std::vector<int>)
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

//----------------------------------------------------------------------------
  std::vector<double> getValueFromXMLElement(xml_node &node, const std::string& sep, std::vector<double>)
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

//----------------------------------------------------------------------------
  std::vector<std::string> getValueFromXMLElement(xml_node &node, const std::string& sep, std::vector<std::string>)
  {
    std::vector<std::string> vals;
    vals = smtk::common::StringUtil::split(node.text().get(), sep, false, false);
    return vals;
  }

//----------------------------------------------------------------------------
  template<typename ItemDefType, typename BasicType>
  void processDerivedValueDef(pugi::xml_node &node,
                              ItemDefType idef, Logger &logger)
  {
    xml_node dnode, child, rnode;
    xml_attribute xatt;
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
        if ( cname == "Structure")
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
          smtkErrorMacro(logger,
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
        if (cname != "Structure")
          {
          continue;
          }
        // Ok lets read in the items associated with this value
        // First grab the associated enum
        std::string v = idef->discreteEnum(i);
        xml_node inode, items = child.child("Items");
        if (!items)
          {
          continue;
          }
        for (inode = items.child("Item"); inode; inode = inode.next_sibling("Item"))
          {
          std::string iname = inode.text().get();
          idef->addConditionalItem(v, iname);
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
        smtkErrorMacro(logger,
          "XML DefaultValue has incorrect size: " << defs.size()
          << " for item " << idef->type());
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
//----------------------------------------------------------------------------
  template<typename ItemType, typename BasicType>
  void processDerivedValue(pugi::xml_node &node,
                           ItemType item, attribute::System &asys,
                           std::vector<ItemExpressionInfo> &itemExpressionInfo,
                           Logger &logger)
  {
    if (item->isDiscrete())
      {
      return; // nothing left to do
      }

    xml_attribute xatt;
    xml_node valsNode;
    std::size_t i, n = item->numberOfValues();
    xml_node val, noVal;
    std::size_t  numRequiredVals = item->numberOfRequiredValues();
    std::string nodeName, expName;
    attribute::AttributePtr expAtt;
    bool allowsExpressions = item->allowsExpressions();
    ItemExpressionInfo info;
    if (item->isExtensible())
      {
      // The node should have an attribute indicating how many values are
      // associated with the item
      xatt = node.attribute("NumberOfValues");
      if (!xatt)
        {
        smtkErrorMacro(logger,
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
          smtkErrorMacro(logger,
                         "XML Attribute Ith is missing for Item: " << item->name());
          continue;
          }
        i = xatt.as_uint();
        if (i >= n)
          {
          smtkErrorMacro(logger, "XML Attribute Ith = " << i
                         << " is out of range for Item: " << item->name());
          continue;
          }
        if (nodeName == "Val")
          {
          item->setValue(static_cast<int>(i), getValueFromXMLElement(val, BasicType()));
          }
        else if (allowsExpressions && (nodeName == "Expression"))
          {
          expName = val.text().get();
          expAtt = asys.findAttribute(expName);
          if (!expAtt)
            {
            info.item = item; info.pos = static_cast<int>(i); info.expName = expName;
            itemExpressionInfo.push_back(info);
            }
          else
            {
            item->setExpression(static_cast<int>(i), expAtt);
            }
          }
        else
          {
          smtkErrorMacro(logger, "Unsupported Value Node Type  Item: "
                         << item->name());
          }
        }
      }
    else if ((numRequiredVals == 1) && !item->isExtensible())
      {
      // Lets see if there is an unset val element
      noVal = node.child("UnsetVal");
      if (!noVal)
        {
        // Is this an exapression?
        xatt = node.attribute("Expression");
        if (allowsExpressions && xatt)
          {
          expName = node.text().get();
          expAtt = asys.findAttribute(expName);
          if (!expAtt)
            {
            info.item = item; info.pos = 0; info.expName = expName;
            itemExpressionInfo.push_back(info);
            }
          else
            {
            item->setExpression(expAtt);
            }
          }
        else
          {
          item->setValue(getValueFromXMLElement(node, BasicType()));
          }
        }
      }
    else
      {
      smtkErrorMacro(logger, "XML Node Values is missing for Item: " << item->name());
      }
  }
};
//----------------------------------------------------------------------------
XmlDocV1Parser::XmlDocV1Parser(smtk::attribute::System &mySystem):
  m_reportAsError(true), m_system(mySystem)
{
}

//----------------------------------------------------------------------------
XmlDocV1Parser::~XmlDocV1Parser()
{
}
//----------------------------------------------------------------------------
xml_node XmlDocV1Parser::getRootNode(xml_document &doc)
{
  xml_node amnode = doc.child("SMTK_AttributeManager");
  return amnode;
}

//----------------------------------------------------------------------------
bool XmlDocV1Parser::canParse(xml_document &doc)
{
  // Get the attribute system node
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
  if (versionNum != 1)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool XmlDocV1Parser::canParse(xml_node &node)
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
  if (versionNum != 1)
    {
    return false;
    }

  return true;
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::process(xml_document &doc)
{
  // Get the attribute system node
  xml_node amnode = doc.child("SMTK_AttributeManager");

  // Check that there is content
  if (amnode.empty())
    {
    smtkWarningMacro(m_logger, "Missing SMTK_AttributeManager element");
    return;
    }

  this->process(amnode);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::process(xml_node &amnode)
{
  // Reset the message log
  this->m_logger.reset();
  // Clear the vectors for dealing with attribute references
  this->m_itemExpressionDefInfo.clear();
  this->m_attRefDefInfo.clear();
  this->m_itemExpressionInfo.clear();
  this->m_attRefInfo.clear();
  this->m_logger.reset();
  xml_node node, cnode;

  // Get the category information, starting with current set
  std::set<std::string> secCatagories = m_system.categories();
  std::string s;
  node = amnode.child("Categories");
  if (node)
    {
    // Get the default category if one is specified
    s = node.attribute("Default").value();
    if (s != "")
      {
      this->m_defaultCategory = s;
      secCatagories.insert(s.c_str());
      }
    for (cnode = node.first_child(); cnode; cnode = cnode.next_sibling())
      {
      if (cnode.text().empty())
        {
        continue;
        }
      secCatagories.insert(cnode.text().get());
      }
    }

  // Process Analysis Info
  std::set<std::string> catagories;
  node = amnode.child("Analyses");
  if (node)
    {
    xml_node anode;
    for (anode = node.first_child(); anode; anode = anode.next_sibling())
      {
      s = anode.attribute("Type").value();
      catagories.clear();
      for (cnode = anode.first_child(); cnode; cnode = cnode.next_sibling())
        {
        if (cnode.text().empty())
          {
          continue;
          }
        catagories.insert(cnode.text().get());
        }
      this->m_system.defineAnalysis(s, catagories);
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
      if(s.empty())
        {
        std::stringstream tmp;
        tmp << "Level " << val;
        s = tmp.str();
        }
      this->m_system.addAdvanceLevel(val, s);

      xml_attribute xatt = anode.attribute("Color");
      if(xatt)
        {
        double color[4];
        s = xatt.value();
        if(!s.empty() && this->decodeColorInfo(s, color) == 0)
          {
          this->m_system.setAdvanceLevelColor(val, color);
          }
        }
      }
    }

  this->processAttributeInformation(amnode);
  this->processViews(amnode);
  this->processModelInfo(amnode);

  // Now we need to check to see if there are any catagories in the system
  // that were not explicitly listed in the catagories section - first update catagories
  this->m_system.updateCategories();

  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = this->m_system.categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    if (secCatagories.find(*it) == secCatagories.end())
      {
      smtkErrorMacro(this->m_logger, "Category: " << *it
                     << " was not listed in System's Category Section");
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processAttributeInformation(xml_node &root)
{
  // Process definitions first
  xml_node child, node = root.child("Definitions");
  std::size_t i;
  if (node)
    {
    for (child = node.first_child(); child; child = child.next_sibling())
      {
      this->createDefinition(child);
      }

    // At this point we have all the definitions read in so lets
    // fix up all of the attribute definition references
    attribute::DefinitionPtr def;
    for (i = 0; i < this->m_itemExpressionDefInfo.size(); i++)
      {
      def = this->m_system.findDefinition(this->m_itemExpressionDefInfo[i].second);
      if (def)
        {
        this->m_itemExpressionDefInfo[i].first->setExpressionDefinition(def);
        }
      else
        {
        smtkErrorMacro(this->m_logger,
                       "Referenced Attribute Definition: "
                       << this->m_itemExpressionDefInfo[i].second
                       << " is missing and required by Item Definition: "
                       << this->m_itemExpressionDefInfo[i].first->name());
        }
      }

    for (i = 0; i < this->m_attRefDefInfo.size(); i++)
      {
      def = this->m_system.findDefinition(this->m_attRefDefInfo[i].second);
      if (def)
        {
        this->m_attRefDefInfo[i].first->setAttributeDefinition(def);
        }
      else
        {
        smtkErrorMacro(this->m_logger,
                       "Referenced Attribute Definition: "
                       << this->m_attRefDefInfo[i].second
                       << " is missing and required by Item Definition: "
                       << this->m_attRefDefInfo[i].first->name());
        }
      }
    }
  node = root.child("Attributes");
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
  for (i = 0; i < this->m_itemExpressionInfo.size(); i++)
    {
    att = this->m_system.findAttribute(this->m_itemExpressionInfo[i].expName);
    if (att)
      {
      this->m_itemExpressionInfo[i].
        item->setExpression(m_itemExpressionInfo[i].pos, att);
      }
    else
      {
      smtkErrorMacro(this->m_logger,
                     "Expression Attribute: "
                     << this->m_itemExpressionInfo[i].expName
                     << " is missing and required by Item : "
                     << this->m_itemExpressionInfo[i].item->name());
      }
    }

  for (i = 0; i < this->m_attRefInfo.size(); i++)
    {
    att = this->m_system.findAttribute(this->m_attRefInfo[i].attName);
    if (att)
      {
      this->m_attRefInfo[i].item->setValue(this->m_attRefInfo[i].pos, att);
      }
    else
      {
      smtkErrorMacro(this->m_logger,
                     "Referenced Attribute: "
                     << this->m_attRefInfo[i].attName
                     << " is missing and required by Item: "
                     << this->m_attRefInfo[i].item->name());
      }
    }

}
//----------------------------------------------------------------------------
void XmlDocV1Parser::createDefinition(xml_node &defNode)
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
    smtkErrorMacro(this->m_logger, "Definition missing Type XML Attribute");
    return;
    }
  baseType = defNode.attribute("BaseType").value();
  if (baseType != "")
    {
    baseDef = this->m_system.findDefinition(baseType);
    if (!baseDef)
      {
      smtkErrorMacro(this->m_logger, "Could not find Base Definition: "
                     << baseType << " needed to create Definition: "
                     << type);
      return;
      }
    def = this->m_system.createDefinition(type, baseDef);
    }
  else
    {
    def = this->m_system.createDefinition(type);
    }
  if (!def)
    {
    if (m_reportAsError)
      {
      smtkErrorMacro(this->m_logger, "Definition: "
                     << type << " already exists in the System");
      }
    else
      {
      smtkWarningMacro(this->m_logger, "Definition: "
                       << type << " already exists in the System");
      }
    return;
    }
  this->processDefinition(defNode, def);
}

//----------------------------------------------------------------------------
  void XmlDocV1Parser::processDefinition(xml_node &defNode,
                                         smtk::attribute::DefinitionPtr def)
{
  xml_attribute xatt;
  xml_node node;
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
    def->setAdvanceLevel(xatt.as_int());
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

  // Read old-style association mask first.  Note that the association is set
  // as extensible.
  // It will be overwritten if a new-style AssociationsDef
  // is also provided.
  xatt = defNode.attribute("Associations");
  if (xatt)
    {
    model::BitFlags mask =
      this->decodeModelEntityMask(xatt.value());
    def->setAssociationMask(mask);
    def->associationRule()->setIsExtensible(true);
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
    std::string assocName = node.attribute("Name").value();
    if (assocName.empty())
      assocName = def->type() + "Associations";
    smtk::attribute::ModelEntityItemDefinitionPtr assocDef =
      smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(
        smtk::attribute::ModelEntityItemDefinition::New(assocName));
    this->processModelEntityDef(node, assocDef);
    def->setAssociationRule(assocDef);
    }

  // Now lets process its items
  xml_node itemsNode = defNode.child("ItemDefinitions");
  std::string itemName;
  smtk::attribute::Item::Type itype;
  smtk::attribute::ItemDefinitionPtr idef;
  for (node = itemsNode.first_child(); node; node = node.next_sibling())
    {
    itype = smtk::attribute::Item::string2Type(node.name());
    itemName = node.attribute("Name").value();
    switch (itype)
      {
      case smtk::attribute::Item::ATTRIBUTE_REF:
        idef = def->addItemDefinition<smtk::attribute::RefItemDefinition>(itemName);
        this->processRefDef(node, smtk::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DOUBLE:
        idef = def->addItemDefinition<smtk::attribute::DoubleItemDefinition>(itemName);
        this->processDoubleDef(node, smtk::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DIRECTORY:
        idef = def->addItemDefinition<smtk::attribute::DirectoryItemDefinition>(itemName);
        this->processDirectoryDef(node, smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(idef));
        break;
      case smtk::attribute::Item::FILE:
        idef = def->addItemDefinition<smtk::attribute::FileItemDefinition>(itemName);
        this->processFileDef(node, smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(idef));
        break;
      case smtk::attribute::Item::GROUP:
        idef = def->addItemDefinition<smtk::attribute::GroupItemDefinition>(itemName);
        this->processGroupDef(node, smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(idef));
        break;
      case smtk::attribute::Item::INT:
        idef = def->addItemDefinition<smtk::attribute::IntItemDefinition>(itemName);
        this->processIntDef(node, smtk::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(idef));
        break;
      case smtk::attribute::Item::STRING:
        idef = def->addItemDefinition<smtk::attribute::StringItemDefinition>(itemName);
        this->processStringDef(node, smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(idef));
        break;
      case smtk::attribute::Item::MODEL_ENTITY:
        idef = def->addItemDefinition<smtk::attribute::ModelEntityItemDefinition>(itemName);
        this->processModelEntityDef(node, smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(idef));
        break;
      case smtk::attribute::Item::VOID:
        idef = def->addItemDefinition<smtk::attribute::VoidItemDefinition>(itemName);
        this->processItemDef(node, idef);
      break;
      case smtk::attribute::Item::MESH_SELECTION:
        idef = def->addItemDefinition<smtk::attribute::MeshSelectionItemDefinition>(itemName);
        this->processMeshSelectionDef(node, smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItemDefinition>(idef));
        break;
      case smtk::attribute::Item::MESH_ENTITY:
        idef = def->addItemDefinition<smtk::attribute::MeshItemDefinition>(itemName);
        this->processMeshEntityDef(node, smtk::dynamic_pointer_cast<smtk::attribute::MeshItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DATE_TIME:
        idef = def->addItemDefinition<smtk::attribute::DateTimeItemDefinition>(itemName);
        this->processDateTimeDef(
          node, smtk::dynamic_pointer_cast<smtk::attribute::DateTimeItemDefinition>(idef));
        break;

    default:
      smtkErrorMacro(this->m_logger, "Unsupported Item definition Type: "
                     << node.name()
                     << " needed to create Definition: " << def->type());
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processItemDef(xml_node &node,
                                    smtk::attribute::ItemDefinitionPtr idef)
{
  xml_attribute xatt;
  xml_node catNodes, child;
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
  // If using AdvanceLevel then we are setting
  // both read and write
  xatt = node.attribute("AdvanceLevel");
  if (xatt)
    {
    idef->setAdvanceLevel(0, xatt.as_int());
    idef->setAdvanceLevel(1, xatt.as_int());
    }
  else
    {
    xatt = node.attribute("AdvanceReadLevel");
    if (xatt)
      {
      idef->setAdvanceLevel(0, xatt.as_int());
      }
    xatt = node.attribute("AdvanceWriteLevel");
    if (xatt)
      {
      idef->setAdvanceLevel(1, xatt.as_int());
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

  catNodes = node.child("Categories");
  if (catNodes)
    {
    for (child = catNodes.first_child(); child; child = child.next_sibling())
      {
      idef->addCategory(child.text().get());
      }
    }
  else if (this->m_defaultCategory != "" &&
           !smtk::dynamic_pointer_cast<attribute::GroupItemDefinition>(idef))
    { // group item definitions don't get categories
    idef->addCategory(this->m_defaultCategory.c_str());
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processDoubleDef(pugi::xml_node &node,
                                         attribute::DoubleItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  processDerivedValueDef<attribute::DoubleItemDefinitionPtr, double>
    (node, idef, this->m_logger);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processIntDef(pugi::xml_node &node,
                                      attribute::IntItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  processDerivedValueDef<attribute::IntItemDefinitionPtr, int>
    (node, idef, this->m_logger);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processStringDef(pugi::xml_node &node,
                                         attribute::StringItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  if( xml_attribute xatt = node.attribute("MultipleLines") )
    {
    (void)xatt;
    idef->setIsMultiline(true);
    }
  processDerivedValueDef<attribute::StringItemDefinitionPtr, std::string>
    (node, idef, this->m_logger);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processModelEntityDef(pugi::xml_node &node,
                                         attribute::ModelEntityItemDefinitionPtr idef)
{
  xml_node labels, mmask, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);
  mmask = node.child("MembershipMask");
  if (mmask)
    {
    idef->setMembershipMask(
      this->decodeModelEntityMask(
        mmask.text().as_string()));
    }

  xatt = node.attribute("NumberOfRequiredValues");
  if (xatt)
    {
    idef->setNumberOfRequiredValues(xatt.as_int());
    }
  else
    {
    smtkErrorMacro(this->m_logger,
                   "Missing XML Attribute NumberOfRequiredValues for Item Definition : "
                   << idef->name());
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
  if(node.child("Labels"))
    {
    smtkErrorMacro(this->m_logger,
                   "Labels has been changed to ComponentLabels : "
                   << idef->name());
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
//----------------------------------------------------------------------------
void XmlDocV1Parser::processMeshSelectionDef(pugi::xml_node &node,
                                         attribute::MeshSelectionItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "The Mesh Selection defs should only be availabe starting Attribute Version 2 Format"
                 << idef->name());
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processMeshEntityDef(pugi::xml_node &node,
                                         attribute::MeshItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "The Mesh Entity defs should only be availabe starting Attribute Version 2 Format"
                 << idef->name());
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processDateTimeDef(
  pugi::xml_node &node,
  attribute::DateTimeItemDefinitionPtr idef)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "DateTime item defs only supported starting Attribute Version 3 Format"
                 << idef->name());
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processValueDef(pugi::xml_node &node,
                                        attribute::ValueItemDefinitionPtr idef)
{
  xml_node labels, child;
  xml_attribute xatt;
  std::size_t i;
  this->processItemDef(node, idef);

  xatt = node.attribute("NumberOfRequiredValues");
  std::size_t numberOfComponents = 0;
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
  if(node.child("Labels"))
    {
    smtkErrorMacro(this->m_logger,
                   "Labels has been changed to ComponentLabels : "
                   << idef->name());
    }
  labels = node.child("ComponentLabels");
  if (labels)
    {
    if((numberOfComponents == 1) && !idef->isExtensible())
      {
      smtkErrorMacro(this->m_logger,
                     "Should not use ComponentLabels when NumberOfRequiredValues=1 : "
                     << idef->name());
      }

    // Are we using a common label?
    xatt = labels.attribute("CommonLabel");
    if (xatt)
      {
      idef->setCommonValueLabel(xatt.value());
      if(labels.first_child())
        {
        smtkErrorMacro(this->m_logger,
                       "Cannot combine CommonLabel with Label child nodes : "
                       << idef->name());
        }
      }
    else
      {
      for (child = labels.first_child(), i = 0; child; child = child.next_sibling(), i++)
        {
        if(i<numberOfComponents)
          {
          idef->setValueLabel(i, child.text().get());
          }
        }
      if(i!=numberOfComponents)
        {
        smtkErrorMacro(this->m_logger,
                       "Wrong number of component values for : "
                       << idef->name());
        }
      }
    }
  child = node.child("ExpressionType");
  if (child)
    {
    // Is the attribute definition already in the system?
    std::string etype = child.text().get();
    attribute::DefinitionPtr adef = this->m_system.findDefinition(etype);
    if (adef)
      {
      idef->setExpressionDefinition(adef);
      }
    else
      {
      // We need to queue up this item to be assigned its definition later
      this->m_itemExpressionDefInfo.push_back(ItemExpressionDefInfo(idef, etype));
      }
    }
  xatt = node.attribute("Units");
  if (xatt)
    {
    idef->setUnits(xatt.value());
    }

  // Now lets process its children items
  xml_node cinode, citemsNode = node.child("ChildrenDefinitions");
  std::string citemName;
  smtk::attribute::Item::Type citype;
  smtk::attribute::ItemDefinitionPtr cidef;
  for (cinode = citemsNode.first_child(); cinode; cinode = cinode.next_sibling())
    {
    citype = smtk::attribute::Item::string2Type(cinode.name());
    citemName = cinode.attribute("Name").value();
    switch (citype)
      {
      case smtk::attribute::Item::ATTRIBUTE_REF:
        if( (cidef = idef->addItemDefinition<smtk::attribute::RefItemDefinition>(citemName)) )
          {
          this->processRefDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::DOUBLE:
        if( (cidef = idef->addItemDefinition<smtk::attribute::DoubleItemDefinition>(citemName)) )
          {
          this->processDoubleDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::DIRECTORY:
        if( (cidef = idef->addItemDefinition<smtk::attribute::DirectoryItemDefinition>(citemName)) )
          {
          this->processDirectoryDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::FILE:
        if( (cidef = idef->addItemDefinition<smtk::attribute::FileItemDefinition>(citemName)) )
          {
          this->processFileDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::GROUP:
        if( (cidef = idef->addItemDefinition<smtk::attribute::GroupItemDefinition>(citemName)) )
          {
          this->processGroupDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::INT:
        if( (cidef = idef->addItemDefinition<smtk::attribute::IntItemDefinition>(citemName)) )
          {
          this->processIntDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::STRING:
        if( (cidef = idef->addItemDefinition<smtk::attribute::StringItemDefinition>(citemName)) )
          {
          this->processStringDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::MODEL_ENTITY:
        if( (cidef = idef->addItemDefinition<smtk::attribute::ModelEntityItemDefinition>(citemName)) )
          {
          this->processModelEntityDef(cinode, smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
        break;
      case smtk::attribute::Item::VOID:
        if( (cidef = idef->addItemDefinition<smtk::attribute::VoidItemDefinition>(citemName)) )
          {
          this->processItemDef(cinode, cidef);
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
      break;
      case smtk::attribute::Item::MESH_SELECTION:
        if( (cidef = idef->addItemDefinition<smtk::attribute::MeshSelectionItemDefinition>(citemName)) )
          {
          this->processItemDef(cinode, cidef);
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
      break;
      case smtk::attribute::Item::MESH_ENTITY:
        if( (cidef = idef->addItemDefinition<smtk::attribute::MeshItemDefinition>(citemName)) )
          {
          this->processMeshEntityDef(cinode,
            smtk::dynamic_pointer_cast<smtk::attribute::MeshItemDefinition>(cidef));
          }
        else
          {
          smtkErrorMacro(this->m_logger, "Item definition " << citemName << " already exists");
          }
      break;
    default:
      smtkErrorMacro(this->m_logger, "Unsupported Item definition Type: "
                     << cinode.name()
                     << " needed to create Definition: " << citype);
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processRefDef(pugi::xml_node &node,
                                   attribute::RefItemDefinitionPtr idef)
{
  xml_node labels, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);

  // Has the attribute definition been set?
  child = node.child("AttDef");
  if (child)
    {
    // Is the attribute definition already in the system?
    std::string etype = child.text().get();
    attribute::DefinitionPtr adef = this->m_system.findDefinition(etype);
    if (adef)
      {
      idef->setAttributeDefinition(adef);
      }
    else
      {
      // We need to queue up this item to be assigned its definition later
      this->m_attRefDefInfo.push_back(AttRefDefInfo(idef, etype));
      }
    }

  xatt = node.attribute("NumberOfRequiredValues");
  if (xatt)
    {
    idef->setNumberOfRequiredValues(xatt.as_int());
    }
  else
    {
    smtkErrorMacro(this->m_logger,
                   "Missing XML Attribute NumberOfRequiredValues for Item Definition : "
                   << idef->name());
    }

  // Lets see if there are labels
  if(node.child("Labels"))
    {
    smtkErrorMacro(this->m_logger,
                   "Labels has been changed to ComponentLabels : "
                   << idef->name());
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
//----------------------------------------------------------------------------
void XmlDocV1Parser::processDirectoryDef(pugi::xml_node &node,
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
  else
    {
    smtkErrorMacro(this->m_logger,
                   "Missing XML Attribute NumberOfRequiredValues for Item Definition : "
                   << idef->name());
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
//----------------------------------------------------------------------------
void XmlDocV1Parser::processFileDef(pugi::xml_node &node,
                                       attribute::FileItemDefinitionPtr idef)
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
  else
    {
    smtkErrorMacro(this->m_logger,
                   "Missing XML Attribute NumberOfRequiredValues for Item Definition : "
                   << idef->name());
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
  if(node.child("Labels"))
    {
    smtkErrorMacro(this->m_logger,
                   "Labels has been changed to ComponentLabels : "
                   << idef->name());
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
//----------------------------------------------------------------------------
void XmlDocV1Parser::processGroupDef(pugi::xml_node &node,
                                        attribute::GroupItemDefinitionPtr def)
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
  // Lets see if there are labels
  if(node.child("Labels"))
    {
    smtkErrorMacro(this->m_logger,
                   "Labels has been changed to ComponentLabels : "
                   << def->name());
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
        def->setSubGroupLabel(i, child.value());
        }
      }
    }
  xml_node itemsNode = node.child("ItemDefinitions");
  std::string itemName;
  smtk::attribute::Item::Type itype;
  smtk::attribute::ItemDefinitionPtr idef;
  for (child = itemsNode.first_child(); child; child = child.next_sibling())
    {
    itype = smtk::attribute::Item::string2Type(child.name());
    itemName = child.attribute("Name").value();
    switch (itype)
      {
      case smtk::attribute::Item::ATTRIBUTE_REF:
        idef = def->addItemDefinition<smtk::attribute::RefItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create Ref Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processRefDef(child, smtk::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DOUBLE:
        idef = def->addItemDefinition<smtk::attribute::DoubleItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create Double Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processDoubleDef(child, smtk::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DIRECTORY:
        idef = def->addItemDefinition<smtk::attribute::DirectoryItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create Directory Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processDirectoryDef(child,
                                  smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(idef));
        break;
      case smtk::attribute::Item::FILE:
        idef = def->addItemDefinition<smtk::attribute::FileItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create File Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processFileDef(child, smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(idef));
        break;
      case smtk::attribute::Item::GROUP:
        idef = def->addItemDefinition<smtk::attribute::GroupItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create Group Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processGroupDef(child, smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(idef));
        break;
      case smtk::attribute::Item::INT:
        idef = def->addItemDefinition<smtk::attribute::IntItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create Int Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processIntDef(child, smtk::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(idef));
        break;
      case smtk::attribute::Item::STRING:
        idef = def->addItemDefinition<smtk::attribute::StringItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create String Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processStringDef(child, smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(idef));
        break;
      case smtk::attribute::Item::MODEL_ENTITY:
        idef = def->addItemDefinition<smtk::attribute::ModelEntityItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create String Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processModelEntityDef(child,
                                    smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(idef));
        break;
      case smtk::attribute::Item::VOID:
        idef = def->addItemDefinition<smtk::attribute::VoidItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create Void Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processItemDef(child, idef);
        break;
      case smtk::attribute::Item::MESH_SELECTION:
        idef = def->addItemDefinition<smtk::attribute::MeshSelectionItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create String Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processItemDef(child, idef);
        break;
      case smtk::attribute::Item::MESH_ENTITY:
        idef = def->addItemDefinition<smtk::attribute::MeshItemDefinition>(itemName);
        if (!idef)
          {
          smtkErrorMacro(this->m_logger,
                         "Failed to create String Item definition Type: " << child.name()
                         << " needed to create Group Definition: " << def->name());
          continue;
          }
        this->processMeshEntityDef(child,
          smtk::dynamic_pointer_cast<smtk::attribute::MeshItemDefinition>(idef));
        break;
      default:
        smtkErrorMacro(this->m_logger,
                       "Unsupported Item definition Type: " << child.name()
                       << " needed to create Group Definition: " << def->name());
      }
    }
}

//----------------------------------------------------------------------------
smtk::common::UUID XmlDocV1Parser::getAttributeID(xml_node &attNode)
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
    smtkWarningMacro(this->m_logger,
                   "Attribute: " << name
                   << " is being assigned a new ID (Version 1 Format)!");

  return smtk::common::UUID::null();
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processAttribute(xml_node &attNode)
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
    smtkErrorMacro(this->m_logger,
                   "Invalid Attribute! - Missing XML Attribute Name");
    return;
    }
  name = xatt.value();
  xatt = attNode.attribute("Type");
  if (!xatt)
    {
    smtkErrorMacro(this->m_logger,
                   "Invalid Attribute: " << name
                   << "  - Missing XML Attribute Type");
    return;
    }
  type = xatt.value();

  id = this->getAttributeID(attNode);

  def = this->m_system.findDefinition(type);
  if (!def)
    {
    smtkErrorMacro(this->m_logger,
                   "Attribute: " << name << " of Type: " << type
                   << "  - can not find attribute definition");
    return;
    }

  // Is the definition abstract?
  if (def->isAbstract())
    {
    smtkErrorMacro(this->m_logger,
                   "Attribute: " << name << " of Type: " << type
                   << "  - is based on an abstract definition");
    return;
    }

  // Do we have a valid uuid?
  if (id.isNull())
    {
    att = this->m_system.createAttribute(name, def);
    }
  else
    {
    att = this->m_system.createAttribute(name, def, id);
    }


  if (!att)
    {
    smtkErrorMacro(this->m_logger,
                   "Attribute: " << name << " of Type: " << type
                   << "  - could not be created - is the name in use");
    return;
    }
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
        smtkErrorMacro(this->m_logger,
          "Bad Item for Attribute : " << name
          << "- missing XML Attribute Name");
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
        smtkErrorMacro(this->m_logger,
          "Can not locate XML Item node :" << att->item(i)->name()
          << " for Attribute : " << name);
        continue;
        }
      this->processItem(node, att->item(i));
      }
    if (iNode || (i != n))
      {
      smtkErrorMacro(this->m_logger,
        "Number of Items does not match XML for Attribute : " << name);
      }
    }

  assocsNode = attNode.child("Associations");
  if (assocsNode)
    {
    smtk::attribute::ModelEntityItem::Ptr assocsItem = att->associations();
    this->processItem(assocsNode, assocsItem);
    // Now the ModelEntityItem is deserialized but we need
    // to let the model manager know about the associations
    // (assuming we have a model manager):
    smtk::model::Manager::Ptr mmgr = att->modelManager();
    if (mmgr)
      {
      smtk::attribute::ModelEntityItem::const_iterator eit;
      for (eit = assocsItem->begin(); eit != assocsItem->end(); ++eit)
        {
        // Calling associateAttribute() with a NULL attribute system
        // prevents the model manager from attempting to add the association
        // back to the attribute we are currently deserializing:
        mmgr->associateAttribute(NULL, att->id(), eit->entity());
        }
      }
    }
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processItem(xml_node &node,
                                 smtk::attribute::ItemPtr item)
{
  xml_attribute xatt;
  if (item->isOptional())
    {
    xatt = node.attribute("Enabled");
    if (xatt)
      {
      item->setIsEnabled(xatt.as_bool());
      }
    }
  // If using AdvanceLevel then we are setting
  // both read and write
  xatt = node.attribute("AdvanceLevel");
  if (xatt)
    {
    item->setAdvanceLevel(0, xatt.as_int());
    item->setAdvanceLevel(1, xatt.as_int());
    }
  else
    {
    xatt = node.attribute("AdvanceReadLevel");
    if (xatt)
      {
      item->setAdvanceLevel(0, xatt.as_int());
      }
    xatt = node.attribute("AdvanceWriteLevel");
    if (xatt)
      {
      item->setAdvanceLevel(1, xatt.as_int());
      }
    }

  switch (item->type())
    {
    case smtk::attribute::Item::ATTRIBUTE_REF:
      this->processRefItem(node, smtk::dynamic_pointer_cast<smtk::attribute::RefItem>(item));
      break;
    case smtk::attribute::Item::DOUBLE:
      this->processDoubleItem(node, smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(item));
      break;
    case smtk::attribute::Item::DIRECTORY:
      this->processDirectoryItem(node, smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(item));
      break;
    case smtk::attribute::Item::FILE:
      this->processFileItem(node, smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(item));
      break;
    case smtk::attribute::Item::GROUP:
      this->processGroupItem(node, smtk::dynamic_pointer_cast<smtk::attribute::GroupItem>(item));
      break;
    case smtk::attribute::Item::INT:
      this->processIntItem(node, smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(item));
      break;
    case smtk::attribute::Item::STRING:
      this->processStringItem(node, smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(item));
      break;
    case smtk::attribute::Item::MODEL_ENTITY:
      this->processModelEntityItem(node, smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(item));
      break;
    case smtk::attribute::Item::MESH_SELECTION:
      this->processMeshSelectionItem(node, smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItem>(item));
      break;
    case smtk::attribute::Item::MESH_ENTITY:
      this->processMeshEntityItem(node, smtk::dynamic_pointer_cast<smtk::attribute::MeshItem>(item));
      break;
    case smtk::attribute::Item::DATE_TIME:
      this->processDateTimeItem(
        node, smtk::dynamic_pointer_cast<smtk::attribute::DateTimeItem>(item));
      break;
    case smtk::attribute::Item::VOID:
      // Nothing to do!
      break;
    default:
      smtkErrorMacro(this->m_logger,
                     "Unsupported Item Type: "
                     << smtk::attribute::Item::type2String(item->type()));
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processValueItem(pugi::xml_node &node,
                                         attribute::ValueItemPtr item)
{
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::size_t i=0, n = item->numberOfValues();
  xml_attribute xatt;
  if (item->isExtensible())
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
    std::map<std::string, smtk::attribute::ItemPtr>::const_iterator iter;
    const std::map<std::string, smtk::attribute::ItemPtr> &childrenItems = item->childrenItems();
    for (childNode = childrenNodes.first_child(), iter = childrenItems.begin();
         (iter != childrenItems.end()) && childNode;
         iter++, childNode = childNode.next_sibling())
      {
      // See if the name of the item matches the name of node
      xatt = childNode.attribute("Name");
      if (!xatt)
        {
        smtkErrorMacro(this->m_logger,
                       "Bad Child Item for Item : " << item->name()
                       << "- missing XML Attribute Name");
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
        smtkErrorMacro(this->m_logger,
                       "Can not locate XML Child Item node :" << iter->second->name()
                       << " for Item : " << item->name());
        continue;
        }
      this->processItem(inode, iter->second);
      }
    if (childNode || (iter != childrenItems.end()))
      {
      smtkErrorMacro(this->m_logger,
                     "Number of Children Items does not match XML for Item : " << item->name());
      }
    }
  int index=0;
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
        smtkErrorMacro(this->m_logger,
                       "XML Attribute Ith is missing for Item: " << item->name());
        continue;
        }
      i = xatt.as_int();
      if (i >= n)
        {
        smtkErrorMacro(this->m_logger,
                       "XML Attribute Ith = " << i
                       << " and is out of range for Item: " << item->name());
        continue;
        }
      index = val.text().as_int();
      if (!item->setDiscreteIndex(static_cast<int>(i), index))
        {
        smtkErrorMacro(this->m_logger,
                       "Discrete Index " << index
                       << " for  ith value : " << i
                       << " is not valid for Item: " << item->name());
        }
      }
    return;
    }
  if (numRequiredVals == 1) // Special Common Case
    {
    if (!item->setDiscreteIndex(node.text().as_int()))
      {
      smtkErrorMacro(this->m_logger,
                     "Discrete Index " << index
                     << " for  ith value : " << i
                     << " is not valid for Item: " << item->name());
      }
    return;
    }
  smtkErrorMacro(this->m_logger, "Missing Discrete Values for Item: " << item->name());
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processRefItem(pugi::xml_node &node,
                                               attribute::RefItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
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
      attName = val.text().get();
      att = this->m_system.findAttribute(attName);
      if (!att)
        {
        info.item = item; info.pos = static_cast<int>(i); info.attName = attName;
        this->m_attRefInfo.push_back(info);
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
      att = this->m_system.findAttribute(attName);
      if (!att)
        {
        info.item = item; info.pos = 0; info.attName = attName;
        this->m_attRefInfo.push_back(info);
        }
      else
        {
        item->setValue(att);
        }
      }
    }
  else
    {
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processDirectoryItem(pugi::xml_node &node,
                                             attribute::DirectoryItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  if (!numRequiredVals)
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
      item->setValue(static_cast<int>(i), val.text().get());
      }
    }
  else if (numRequiredVals == 1)
    {
    item->setValue(node.text().get());
    }
  else
    {
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processDoubleItem(pugi::xml_node &node,
                                          attribute::DoubleItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<smtk::attribute::ValueItem>(item));
  processDerivedValue<attribute::DoubleItemPtr, double>
    (node, item, this->m_system, this->m_itemExpressionInfo, this->m_logger);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processIntItem(pugi::xml_node &node,
                                       attribute::IntItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<smtk::attribute::ValueItem>(item));
  processDerivedValue<attribute::IntItemPtr, int>
    (node, item, this->m_system, this->m_itemExpressionInfo, this->m_logger);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processStringItem(pugi::xml_node &node,
                                          attribute::StringItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<smtk::attribute::ValueItem>(item));
  processDerivedValue<attribute::StringItemPtr, std::string>
    (node, item, this->m_system, this->m_itemExpressionInfo, this->m_logger);
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processModelEntityItem(pugi::xml_node &node,
                                          attribute::ModelEntityItemPtr item)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "All Model Entity Items will be ignored for Attribute Version 1 Format"
                 << item->name());
  return;
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processMeshSelectionItem(pugi::xml_node &node,
  attribute::MeshSelectionItemPtr item)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "All Mesh Selection Items will be ignored for Attribute Version 1 Format"
                 << item->name());
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processMeshEntityItem(pugi::xml_node &node,
  attribute::MeshItemPtr item)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "All Mesh Entity Items will be ignored for Attribute Version 1 Format"
                 << item->name());
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processDateTimeItem(
  pugi::xml_node &node, attribute::DateTimeItemPtr item)
{
  (void)node;
  smtkWarningMacro(this->m_logger,
                 "DateTime items only supported starting Attribute Version 3 Format"
                 << item->name());
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processFileItem(pugi::xml_node &node,
                                        attribute::FileItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  if (!numRequiredVals)
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
      item->setValue(static_cast<int>(i), val.text().get());
      }
    }
  else if (numRequiredVals == 1)
    {
    item->setValue(node.text().get());
    }
  else
    {
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processGroupItem(pugi::xml_node &node,
                                         attribute::GroupItemPtr item)
{
  std::size_t i, j, m, n;
  std::size_t  numRequiredGroups = item->numberOfRequiredGroups();
  xml_node itemNode;
  xml_attribute xatt;
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
        smtkErrorMacro(this->m_logger,
                       "XML Attribute NumberOfGroups is missing for Group Item: "
                       << item->name());
        return;
        }
      n = xatt.as_uint();
      }

    if (!item->setNumberOfGroups(n))
      {
      smtkErrorMacro(this->m_logger,
                     "Invalid number of sub-groups for Group Item: " << item->name());
      return;
      }
    }

  if (!n) // There are no sub-groups for this item
    {
    return;
    }
  // There are 2 formats - one is for any number of sub groups and the other
  // is a custon case is for 1 subGroup
  xml_node cluster, clusters = node.child("GroupClusters");
  if (clusters)
    {
    for (cluster = clusters.first_child(), i = 0; cluster;
         cluster = cluster.next_sibling(), ++i)
      {
      if (i >= n)
        {
        smtkErrorMacro(this->m_logger,
                       "Too many sub-groups for Group Item: " << item->name());
        continue;
        }
      for (itemNode = cluster.first_child(), j = 0; itemNode;
           itemNode = itemNode.next_sibling(), j++)
        {
        if (j >= m)
          {
        smtkErrorMacro(this->m_logger,
                       "Too many item nodes for subGroup: " << i
                       << " for Group Item: " << item->name());
          continue;
          }
        this->processItem(itemNode, item->item(static_cast<int>(i),static_cast<int>(j)));
        }
      }
    }
  else if (numRequiredGroups == 1)
    {
    for (itemNode = node.first_child(), j = 0; itemNode;
         itemNode = itemNode.next_sibling(), j++)
      {
      if (j >= m)
        {
        smtkErrorMacro(this->m_logger,
                       "Too many item nodes for subGroup: 0"
                       << " for Group Item: " << item->name());
          continue;
          }
        this->processItem(itemNode, item->item(static_cast<int>(j)));
        }
    }
  else
    {
    smtkErrorMacro(this->m_logger,"XML Node GroupClusters is missing for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
bool XmlDocV1Parser::getColor(xml_node &node, double color[4],
                                 const std::string &colorName)
{
  std::string s = node.text().get();
  if(s.empty())
    {
    smtkErrorMacro(this->m_logger, "Color Format Problem - empty input for "
                   << colorName);
    return false;
    }

  int i = this->decodeColorInfo(s, color);
  if (i)
    {
    smtkErrorMacro(this->m_logger, "Color Format Problem - only found " << 4-i
                   << " components for " << colorName << " from string " << s);
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processViews(xml_node &root)
{
  xml_node views = root.child("RootView");
  if (!views)
    {
    return;
    }
  smtk::common::ViewPtr rootView = this->createView(views, "Group");
  rootView->details().setAttribute("TopLevel", "true");
  
  if (!rootView)
    {
    smtkErrorMacro(this->m_logger, "Can't process Root View");
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
    smtk::common::View::Component comp = rootView->details().addChild("AdvancedFontEffects");
    if(node.attribute("Bold"))
      {
      comp.setAttribute("Bold", "t");
      }
    if(node.attribute("Italic"))
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

  this->processGroupView(views,rootView);

}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processAttributeView(xml_node &node,
                                          smtk::common::ViewPtr view)
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
  smtk::common::View::Component &comp = view->details().addChild("AttributeTypes");
  
  for (child = attTypes.child("Type"); child; child = child.next_sibling("Type"))
    {
    comp.addChild("Att").setAttribute("Type", child.text().get());
    }
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processInstancedView(xml_node &node,
                                          smtk::common::ViewPtr view)
{
  xml_attribute xatt;
  xml_node child, instances = node.child("InstancedAttributes");

  if (!instances)
    {
    return; // No instances are in the view
    }
  smtk::common::View::Component &comp = view->details().addChild("InstancedAttributes");
  for (child = instances.child("Att"); child; child = child.next_sibling("Att"))
    {
    xatt = child.attribute("Type");
    if (!xatt)
      {
      smtkErrorMacro(this->m_logger,
                     "XML Attribute Type is missing"
                     << "and is required to create attribute: " << child.text().get()
                     << " for Instanced View: " << view->title());
      continue;
      }
   
    comp.addChild("Att").setAttribute("Type", xatt.value())
      .setAttribute("Name",child.text().get());
    }
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processModelEntityView(xml_node &node,
                                            smtk::common::ViewPtr view)
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

//----------------------------------------------------------------------------
void XmlDocV1Parser::processSimpleExpressionView(xml_node &node,
                                                 smtk::common::ViewPtr view)
{
  xml_node child = node.child("Definition");
  if (child)
    {
    view->details().addChild("Att").setAttribute("Type",child.text().get());
    }
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processGroupView(xml_node &node,
                                      smtk::common::ViewPtr group)
{
  // Group style (Optional), Tabbed (default) or Tiled
  xml_attribute xatt;
  xatt = node.attribute("Style");
  if (xatt)
    {
    group->details().setAttribute("Style", xatt.value());
    }

  // Add Views Component
  smtk::common::View::Component &vcomp = group->details().addChild("Views");
  xml_node child;
  smtk::common::ViewPtr childView;
  std::string childName;
  for (child = node.first_child(); child; child = child.next_sibling())
    {
    childName = child.name();
    if (childName == "AttributeView")
      {
      childView = this->createView(child, "Attribute");
      if (childView)
        {
        vcomp.addChild("View").setAttribute("Title", childView->title());
        this->processAttributeView(child, childView);
        }
      continue;
      }

    if (childName == "GroupView")
      {
      childView = this->createView(child, "Group");
      if (childView)
        {
        vcomp.addChild("View").setAttribute("Title", childView->title());
        this->processGroupView(child, childView);
        }
      continue;
      }

    if (childName == "InstancedView")
      {
      childView = this->createView(child, "Instanced");
      if (childView)
        {
        vcomp.addChild("View").setAttribute("Title", childView->title());
        this->processInstancedView(child, childView);
        }
      continue;
      }

    if (childName == "ModelEntityView")
      {
      childView = this->createView(child, "ModelEntity");
      if (childView)
        {
        vcomp.addChild("View").setAttribute("Title", childView->title());
        this->processModelEntityView(child, childView);
        }
      continue;
      }

    if (childName == "SimpleExpressionView")
      {
      childView = this->createView(child, "SimpleExpression");
      if (childView)
        {
        vcomp.addChild("View").setAttribute("Title", childView->title());
        this->processSimpleExpressionView(child, childView);
        }
      continue;
      }
    }
}

//----------------------------------------------------------------------------
smtk::common::ViewPtr  XmlDocV1Parser::createView(xml_node &node,
                                                  const std::string &viewType)
{
  xml_attribute xatt;
  std::string val;
  xatt = node.attribute("Title"); // Required
  if (!xatt)
    {
    smtkErrorMacro(this->m_logger, "View is missing XML Attribute Title");
    smtk::common::ViewPtr dummy;
    return dummy;
    }
  val = xatt.value();
  smtk::common::ViewPtr view = smtk::common::View::New(viewType, val);

  xatt = node.attribute("Icon"); // optional
  if (xatt)
    {
    val = xatt.value();
    view->details().setAttribute("Icon", val);
    }
  this->m_system.addView(view);
  return view;
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processModelInfo(xml_node &root)
{
  xml_node modelInfo = root.child("ModelInfo");
  if (modelInfo)
    {
    smtkWarningMacro(this->m_logger, "Model Information is not supported in this version");
    }
}

//----------------------------------------------------------------------------
int XmlDocV1Parser::decodeColorInfo(const std::string &s, double *color)
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
//----------------------------------------------------------------------------
smtk::model::BitFlags
XmlDocV1Parser::decodeModelEntityMask(const std::string &s)
{
  smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(s);
  if (!s.empty() && flags == 0 && s != "none" && s != "none|nodim")
    {
    smtkErrorMacro(this->m_logger,
      "Decoding Model Entity Mask - Option \""
      << s << "\" is not supported");
    }
  return flags;
}
//----------------------------------------------------------------------------
