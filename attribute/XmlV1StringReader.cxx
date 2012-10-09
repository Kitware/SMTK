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


#include "attribute/XmlV1StringReader.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.cpp"
#include "attribute/AttributeRefItemDefinition.h"
#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/DirectoryItem.h"
#include "attribute/DirectoryItemDefinition.h"
#include "attribute/FileItem.h"
#include "attribute/FileItemDefinition.h"
#include "attribute/GroupItem.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/Item.h"
#include "attribute/ItemDefinition.h"
#include "attribute/Manager.h"
#include "attribute/AttributeSection.h"
#include "attribute/InstancedSection.h"
#include "attribute/GroupSection.h"
#include "attribute/ModelEntitySection.h"
#include "attribute/RootSection.h"
#include "attribute/SimpleExpressionSection.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"
#include "attribute/ValueItem.h"
#include "attribute/ValueItemDefinition.h"
#include "attribute/VoidItem.h"
#include "attribute/VoidItemDefinition.h"
#include <sstream>
#include <iostream>

using namespace pugi;
using namespace slctk::attribute; 
using namespace slctk;
//----------------------------------------------------------------------------
XmlV1StringReader::XmlV1StringReader(Manager &myManager):
m_manager(myManager)
{
}

//----------------------------------------------------------------------------
XmlV1StringReader::~XmlV1StringReader()
{
}
//----------------------------------------------------------------------------
void XmlV1StringReader::process(xml_document &doc)
{
  // Clear the vectors for dealing with attribute references
  this->m_itemExpressionDefInfo.clear();
  this->m_attRefDefInfo.clear();
  this->m_itemExpressionInfo.clear();
  this->m_attRefInfo.clear();
  this->m_errorStatus.str("");
  xml_node amnode, node, cnode;
  // Get the attribute manager node
  amnode = doc.child("SLCTK_AttributeManager");
  
  // Get the category information
  std::set<std::string> catagories;
  std::string s;
  node = amnode.child("Categories");
  if (node)
    {
    for (cnode = node.first_child(); cnode; cnode = cnode.next_sibling())
      {
      if (cnode.text().empty())
        {
        continue;
        }
      catagories.insert(cnode.text().get());
      }
    }
  std::set<std::string>::const_iterator it;
  std::cout << "Manager Categories:\n";
  for(it = catagories.begin(); it != catagories.end(); it++)
    {
    std::cout << "\t" << *it << "\n";
    }

  // Process Analsis Info
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
      this->m_manager.defineAnalysis(s, catagories);
      }
        
    }
  this->processAttributeInformation(amnode);
  this->processSections(amnode);
  this->processModelInfo(amnode);

  // Now we need to check to see if there are any catagories in the manager
  // that were not explicitly listed in the catagories section - first update catagories
  this->m_manager.updateCategories();

  const std::set<std::string> &cats = this->m_manager.categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    if (catagories.find(*it) == catagories.end())
      {
      this->m_errorStatus << "Error: Catagory: " << *it << " was not listed in Manger's Catagory Section\n";
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttributeInformation(xml_node &root)
{
  // Process definitions first
  xml_node child, node = root.child("Definitions");
  if (!node)
    {
    this->m_errorStatus << "ERROR:Definition Section is missing!\n";
    return;
    }
  for (child = node.first_child(); child; child = child.next_sibling())
    {
    this->processDefinition(child);
    }

  // At this point we have all the definitions read in so lets
  // fix up all of the attribute definition references
  std::size_t i;
  AttributeDefinitionPtr def;
  for (i = 0; i < this->m_itemExpressionDefInfo.size(); i++)
    {
    def = this->m_manager.findDefinition(this->m_itemExpressionDefInfo[i].second);
    if (def)
      {
      this->m_itemExpressionDefInfo[i].first->setExpressionDefinition(def);
      }
    else
      {
      this->m_errorStatus << "ERROR:Referenced Attribute Definition: " 
                          << this->m_itemExpressionDefInfo[i].second
                          << " is missing and required by Item Definition: "
                          << this->m_itemExpressionDefInfo[i].first->name() << "/n";
        }
    }
      
  for (i = 0; i < this->m_attRefDefInfo.size(); i++)
    {
    def = this->m_manager.findDefinition(this->m_attRefDefInfo[i].second);
    if (def)
      {
      this->m_attRefDefInfo[i].first->setAttributeDefinition(def);
      }
    else
      {
      this->m_errorStatus << "ERROR:Referenced Attribute Definition: " 
                          << this->m_attRefDefInfo[i].second
                          << " is missing and required by Item Definition: "
                          << this->m_attRefDefInfo[i].first->name() << "/n";
        }
    }
      
  node = root.child("Attributes");
  if (!node)
    {
    this->m_errorStatus << "ERROR:Attributes Section is missing!\n";
    return;
    }

  for (child = node.first_child(); child; child = child.next_sibling())
    {
    this->processAttribute(child);
    }

  // Have the manager reset its next attribute id properly
  this->m_manager.recomputeNextAttributeID();

  // At this point we have all the attributes read in so lets
  // fix up all of the attribute  references
  AttributePtr att;
  for (i = 0; i < this->m_itemExpressionInfo.size(); i++)
    {
    att = this->m_manager.findAttribute(this->m_itemExpressionInfo[i].expName);
    if (att)
      {
      this->m_itemExpressionInfo[i].
        item->setExpression(m_itemExpressionInfo[i].pos, att);
      }
    else
      {
      this->m_errorStatus << "ERROR:Expression Attribute: " 
                          << this->m_itemExpressionInfo[i].expName
                          << " is missing and required by Item : "
                          << this->m_itemExpressionInfo[i].item->name() << "/n";
        }
    }
      
  for (i = 0; i < this->m_attRefInfo.size(); i++)
    {
    att = this->m_manager.findAttribute(this->m_attRefInfo[i].attName);
    if (att)
      {
      this->m_attRefInfo[i].item->setValue(this->m_attRefInfo[i].pos, att);
      }
    else
      {
      this->m_errorStatus << "ERROR:Referenced Attribute: " 
                          << this->m_attRefInfo[i].attName
                          << " is missing and required by Item: "
                          << this->m_attRefInfo[i].item->name() << "/n";
        }
    }
      
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processDefinition(xml_node &defNode)
{
  xml_node node;
  AttributeDefinitionPtr def, baseDef;
  std::string type = defNode.attribute("Type").value();
  std::string baseType = defNode.attribute("BaseType").value();
  if (baseType != "")
    {
    baseDef = this->m_manager.findDefinition(baseType);
    if (!baseDef)
      {
      this->m_errorStatus << "ERROR: Could not find Base Definition: " << baseType
                          << " needed to create Definition: " << type << "\n";
      return;
      }
    def = this->m_manager.createDefinition(type, baseDef);
    }
  else
    {
    def = this->m_manager.createDefinition(type);
    }
  xml_attribute xatt;
  xatt = defNode.attribute("Label");
  if (xatt)
    {
    def->setLabel(xatt.value());
    }
  def->setVersion(defNode.attribute("Version").as_int());

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

  unsigned int mask = 
    this->decodeModelEntityMask(defNode.attribute("Associations").value());
  def->setAssociationMask(mask);

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

  // Now lets process its items
  xml_node itemsNode = defNode.child("ItemDefinitions");
  std::string itemName;
  Item::Type itype;
  AttributeItemDefinitionPtr idef;
  for (node = itemsNode.first_child(); node; node = node.next_sibling())
    {
    itype = Item::string2Type(node.name());
    itemName = node.attribute("Name").value();
    switch (itype)
      {
      case Item::ATTRIBUTE_REF:
        idef = def->addItemDefinition<AttributeRefItemDefinition>(itemName);
        this->processAttributeRefDef(node, slctk::dynamicCastPointer<AttributeRefItemDefinition>(idef));
        break;
      case Item::DOUBLE:
        idef = def->addItemDefinition<DoubleItemDefinition>(itemName);
        this->processDoubleDef(node, slctk::dynamicCastPointer<DoubleItemDefinition>(idef));
        break;
      case Item::DIRECTORY:
        idef = def->addItemDefinition<DirectoryItemDefinition>(itemName);
        this->processDirectoryDef(node, slctk::dynamicCastPointer<DirectoryItemDefinition>(idef));
        break;
      case Item::FILE:
        idef = def->addItemDefinition<FileItemDefinition>(itemName);
        this->processFileDef(node, slctk::dynamicCastPointer<FileItemDefinition>(idef));
        break;
      case Item::GROUP:
        idef = def->addItemDefinition<GroupItemDefinition>(itemName);
        this->processGroupDef(node, slctk::dynamicCastPointer<GroupItemDefinition>(idef));
        break;
      case Item::INT:
        idef = def->addItemDefinition<IntItemDefinition>(itemName);
        this->processIntDef(node, slctk::dynamicCastPointer<IntItemDefinition>(idef));
        break;
      case Item::STRING:
        idef = def->addItemDefinition<StringItemDefinition>(itemName);
        this->processStringDef(node, slctk::dynamicCastPointer<StringItemDefinition>(idef));
        break;
      case Item::VOID:
        idef = def->addItemDefinition<VoidItemDefinition>(itemName);
        this->processItemDef(node, idef);
      break;
    default:
      this->m_errorStatus << "ERROR: Unsupported Item definition Type: " << node.name()
                          << " needed to create Definition: " << type << "\n";
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processItemDef(xml_node &node, 
                                       AttributeItemDefinitionPtr idef)
{
  xml_attribute xatt;
  xml_node catNodes, child;
  xatt = node.attribute("Label");
  if (xatt)
    {
    idef->setLabel(xatt.value());
    }
  idef->setVersion(node.attribute("Version").as_int());
  xatt = node.attribute("Optional");
  if (xatt)
    {
    idef->setIsOptional(xatt.as_bool());
    idef->setIsEnabledByDefault(node.attribute("IsEnabledByDefault").as_bool());
    }
  xatt = node.attribute("AdvanceLevel");
  if (xatt)
    {
    idef->setAdvanceLevel(xatt.as_int());
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
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processDoubleDef(pugi::xml_node &node,
                                         DoubleItemDefinitionPtr idef)
{
  xml_node dnode, child, rnode;
  xml_attribute xatt;
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  // Is the item discrete?
  dnode = node.child("DiscreteInfo");
  if (dnode)
    {
    double val;
    for (child = dnode.first_child(); child; child = child.next_sibling())
      {
      xatt = child.attribute("Enum");
      if (xatt)
        {
        val = child.text().as_double();
        idef->addDiscreteValue(val, xatt.value());
        }
      else
        {
      this->m_errorStatus 
        << "ERROR: Missing XML Attribute Enum in DiscreteInfo section of Double Item Definition : " 
        << idef->name() << "\n";
        }
      }
    xatt = node.attribute("DefaultIndex");
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
    idef->setDefaultValue(dnode.text().as_double());
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
      idef->setMinRange(child.text().as_double(), inclusive);
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
      idef->setMaxRange(child.text().as_double(), inclusive);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processIntDef(pugi::xml_node &node,
                                      IntItemDefinitionPtr idef)
{
  xml_node dnode, child, rnode;
  xml_attribute xatt;
  // First process the common value item def stuff
  this->processValueDef(node, idef);
  // Is the item discrete?
  dnode = node.child("DiscreteInfo");
  if (dnode)
    {
    int val;
    for (child = dnode.first_child(); child; child = child.next_sibling())
      {
      xatt = child.attribute("Enum");
      if (xatt)
        {
        val = child.text().as_int();
        idef->addDiscreteValue(val, xatt.value());
        }
      else
        {
      this->m_errorStatus 
        << "ERROR: Missing XML Attribute Enum in DiscreteInfo section of Int Item Definition : " 
        << idef->name() << "\n";
        }
      }
    xatt = node.attribute("DefaultIndex");
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
    idef->setDefaultValue(dnode.text().as_int());
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
      idef->setMinRange(child.text().as_int(), inclusive);
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
      idef->setMaxRange(child.text().as_int(), inclusive);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processStringDef(pugi::xml_node &node,
                                         StringItemDefinitionPtr idef)
{
  xml_node dnode, child, rnode;
  xml_attribute xatt;
  // First process the common value item def stuff
  this->processValueDef(node, idef);

  xatt = node.attribute("MultipleLines");
  if (xatt)
    {
    idef->setIsMultiline(xatt.as_bool());
    }

  // Is the item discrete?
  dnode = node.child("DiscreteInfo");
  if (dnode)
    {
    std::string val;
    for (child = dnode.first_child(); child; child = child.next_sibling())
      {
      xatt = child.attribute("Enum");
      if (xatt)
        {
        val = child.text().get();
        idef->addDiscreteValue(val, xatt.value());
        }
      else
        {
      this->m_errorStatus 
        << "ERROR: Missing XML Attribute Enum in DiscreteInfo section of String Item Definition : " 
        << idef->name() << "\n";
        }
      }
    xatt = node.attribute("DefaultIndex");
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
    idef->setDefaultValue(dnode.text().get());
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
      idef->setMinRange(child.text().get(), inclusive);
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
      idef->setMaxRange(child.text().get(), inclusive);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processValueDef(pugi::xml_node &node,
                                        ValueItemDefinitionPtr idef)
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
    this->m_errorStatus 
      << "ERROR: Missing XML Attribute NumberOfRequiredValues for Item Definition : " 
      << idef->name() << "\n";
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
  child = node.child("ExpressionType");
  if (child)
    {
    // Is the attribute definition already in the manager?
    std::string etype = child.text().get();
    AttributeDefinitionPtr adef = this->m_manager.findDefinition(etype);
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
  child = node.child("Units");
  if (child)
    {
    idef->setUnits(child.text().get());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttributeRefDef(pugi::xml_node &node,
                                               AttributeRefItemDefinitionPtr idef)
{
  xml_node labels, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, idef);

  // Has the attribute definition been set?
  child = node.child("AttDef");
  if (child)
    {
    // Is the attribute definition already in the manager?
    std::string etype = child.text().get();
    AttributeDefinitionPtr adef = this->m_manager.findDefinition(etype);
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
    this->m_errorStatus 
      << "ERROR: Missing XML Attribute NumberOfRequiredValues for Item Definition : " 
      << idef->name() << "\n";
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
void XmlV1StringReader::processDirectoryDef(pugi::xml_node &node,
                                            DirectoryItemDefinitionPtr idef)
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
    this->m_errorStatus 
      << "ERROR: Missing XML Attribute NumberOfRequiredValues for Item Definition : " 
      << idef->name() << "\n";
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
void XmlV1StringReader::processFileDef(pugi::xml_node &node,
                                       FileItemDefinitionPtr idef)
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
    this->m_errorStatus 
      << "ERROR: Missing XML Attribute NumberOfRequiredValues for Item Definition : " 
      << idef->name() << "\n";
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
void XmlV1StringReader::processGroupDef(pugi::xml_node &node,
                                        GroupItemDefinitionPtr def)
{
  xml_node labels, child;
  xml_attribute xatt;
  int i;
  this->processItemDef(node, def);

  xatt = node.attribute("NumberOfRequiredGroups");
  if (xatt)
    {
    def->setNumberOfRequiredGroups(xatt.as_int());
    }
  else
    {
    this->m_errorStatus 
      << "ERROR: Missing XML Attribute NumberOfRequiredGroups for Item Definition : " 
      << def->name() << "\n";
    }

  // Lets see if there are labels
  labels = node.child("Labels");
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
  Item::Type itype;
  AttributeItemDefinitionPtr idef;
  for (child = itemsNode.first_child(); child; child = child.next_sibling())
    {
    itype = Item::string2Type(child.name());
    itemName = child.attribute("Name").value();
    switch (itype)
      {
      case Item::ATTRIBUTE_REF:
        idef = def->addItemDefinition<AttributeRefItemDefinition>(itemName);
        this->processAttributeRefDef(child, slctk::dynamicCastPointer<AttributeRefItemDefinition>(idef));
        break;
      case Item::DOUBLE:
        idef = def->addItemDefinition<DoubleItemDefinition>(itemName);
        this->processDoubleDef(child, slctk::dynamicCastPointer<DoubleItemDefinition>(idef));
        break;
      case Item::DIRECTORY:
        idef = def->addItemDefinition<DirectoryItemDefinition>(itemName);
        this->processDirectoryDef(child, slctk::dynamicCastPointer<DirectoryItemDefinition>(idef));
        break;
      case Item::FILE:
        idef = def->addItemDefinition<FileItemDefinition>(itemName);
        this->processFileDef(child, slctk::dynamicCastPointer<FileItemDefinition>(idef));
        break;
      case Item::GROUP:
        idef = def->addItemDefinition<GroupItemDefinition>(itemName);
        this->processGroupDef(child, slctk::dynamicCastPointer<GroupItemDefinition>(idef));
        break;
      case Item::INT:
        idef = def->addItemDefinition<IntItemDefinition>(itemName);
        this->processIntDef(child, slctk::dynamicCastPointer<IntItemDefinition>(idef));
        break;
      case Item::STRING:
        idef = def->addItemDefinition<StringItemDefinition>(itemName);
        this->processStringDef(child, slctk::dynamicCastPointer<StringItemDefinition>(idef));
        break;
      case Item::VOID:
        idef = def->addItemDefinition<VoidItemDefinition>(itemName);
        this->processItemDef(child, idef);
      break;
    default:
      this->m_errorStatus << "ERROR: Unsupported Item definition Type: " << child.name()
                          << " needed to create Group Definition: " << def->name() << "\n";
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttribute(xml_node &attNode)
{
  xml_node itemsNode, iNode, node;
  std::string name, type;
  xml_attribute xatt;
  AttributePtr att;
  AttributeDefinitionPtr def;
  unsigned long id, maxId = 0;
  std::size_t i, n;

  xatt = attNode.attribute("Name");
  if (!xatt)
    {
    this->m_errorStatus << "ERROR: Invalid Attribute! - Missing XML Attribute Name\n";
    return;
    }
  name = xatt.value();
  xatt = attNode.attribute("Type");
  if (!xatt)
    {
    this->m_errorStatus << "ERROR: Invalid Attribute: " << name
                        << "  - Missing XML Attribute Type\n";
    return;
    }
  type = xatt.value();
  xatt = attNode.attribute("ID");
  if (!xatt)
    {
    this->m_errorStatus << "ERROR: Invalid Attribute: " << name
                        << "  - Missing XML Attribute ID\n";
    return;
    }
  id = xatt.as_uint();
  
  def = this->m_manager.findDefinition(type);
  if (!def)
    {
    this->m_errorStatus << "ERROR: Attribute: " << name << " of Type: " << type
                        << "  - can not find attribute definition\n";
    return;
    }
  
  // Is the definition abstract?
  if (def->isAbstract())
    {
    this->m_errorStatus << "ERROR: Attribute: " << name << " of Type: " << type
                        << "  - is based on an abstract definition\n";
    return;
    }
  
  att = this->m_manager.createAttribute(name, def, id);
  
  if (att == NULL)
    {
    this->m_errorStatus << "ERROR: Attribute: " << name << " of Type: " << type
                        << "  - could not be created - is the name in use?\n";
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
  
  itemsNode = attNode.child("Items");
  if (!itemsNode)
    {
    return;
    }
  // Process all of the items in the attribute w/r to the XML
  // NOTE That the writer processes the items in order - lets assume
  // that for speed and if that fails we can try to search for the correct
  // xml node
  n = att->numberOfItems();
  for (i = 0, iNode = itemsNode.first_child(); (i < n) && iNode; 
       i++, iNode = iNode.next_sibling())
    {
    // See if the name of the item matches the name of node
    xatt = iNode.attribute("Name");
    if (!xatt)
      {
      this->m_errorStatus << "ERROR: Bad Item for Attribute : " << name 
                          << "- missing XML Attribute Name\n";
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
      this->m_errorStatus << "Error: Can not locate XML Item node :" << att->item(i)->name() 
                          << " for Attribute : " << name 
                          << "\n";
      continue;
      }
    this->processItem(node, att->item(i));
    }
  if (iNode || (i != n))
    {
    this->m_errorStatus << "Error: Number of Items does not match XML for Attribute : " << name 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processItem(xml_node &node, 
                                    AttributeItemPtr item)
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
  switch (item->type())
    {
    case Item::ATTRIBUTE_REF:
      this->processAttributeRefItem(node, slctk::dynamicCastPointer<AttributeRefItem>(item));
      break;
    case Item::DOUBLE:
      this->processDoubleItem(node, slctk::dynamicCastPointer<DoubleItem>(item));
      break;
    case Item::DIRECTORY:
      this->processDirectoryItem(node, slctk::dynamicCastPointer<DirectoryItem>(item));
      break;
    case Item::FILE:
      this->processFileItem(node, slctk::dynamicCastPointer<FileItem>(item));
      break;
    case Item::GROUP:
      this->processGroupItem(node, slctk::dynamicCastPointer<GroupItem>(item));
      break;
    case Item::INT:
      this->processIntItem(node, slctk::dynamicCastPointer<IntItem>(item));
      break;
    case Item::STRING:
      this->processStringItem(node, slctk::dynamicCastPointer<StringItem>(item));
      break;
    case Item::VOID:
      // Nothing to do!
      break;
    default:
      this->m_errorStatus << "Error: Unsupported Item Type: " << Item::type2String(item->type())
                          << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processValueItem(pugi::xml_node &node,
                                         ValueItemPtr item)
{
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::size_t i, n = item->numberOfValues();
  xml_attribute xatt;
  if (!numRequiredVals)
    {
    // The node should have an attribute indicating how many values are 
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
      {
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
      return;
      }
    n = xatt.as_uint();
    item->setNumberOfValues(n);
    }
  if (!item->isDiscrete())
    {
    return; // there is nothing to be done
    }
  xml_node val, values;
  int index;
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_int();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      index = val.text().as_int();
      if (!item->setDiscreteIndex(i, index))
        {
        this->m_errorStatus << "Error: Discrete Index " << index
                            << " for  ith value : " << i
                            << " is not valid for Item: " << item->name() 
                            << "\n";
        }
      }
    return;
    }
  if (numRequiredVals == 1) // Special Common Case
    {
    if (!item->setDiscreteIndex(node.text().as_int()))
      {
        this->m_errorStatus << "Error: Discrete Index " << index
                            << " for  ith value : " << i
                            << " is not valid for Item: " << item->name() 
                            << "\n";
      }
    return;
    }
  this->m_errorStatus << "Error: Missing Discrete Values for Item: " << item->name() 
                      << "\n";
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttributeRefItem(pugi::xml_node &node,
                                               AttributeRefItemPtr item)
{
  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::string attName;
  AttributePtr att;
  AttRefInfo info;
  if (!numRequiredVals)
    {
    // The node should have an attribute indicating how many values are 
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
      {
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      attName = val.text().get();
      att = this->m_manager.findAttribute(attName);
      if (att == NULL)
        {
        info.item = item; info.pos = i; info.attName = attName;
        this->m_attRefInfo.push_back(info);
        }
      else
        {
        item->setValue(i, att);
        }
      }
    }
  else if (numRequiredVals == 1)
    {
    val = node.child("Val");
    if (val != NULL)
      {
      attName = val.text().get();
      att = this->m_manager.findAttribute(attName);
      if (att == NULL)
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
    this->m_errorStatus << "Error: XML Node Values is missing for Item: " << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processDirectoryItem(pugi::xml_node &node,
                                             DirectoryItemPtr item)
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
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      item->setValue(i, val.text().get());
      }
    }
  else if (numRequiredVals == 1)
    {
    val = node.child("Val");
    if (val != NULL)
      {
      item->setValue(val.text().get());
      }
    }
  else
    {
    this->m_errorStatus << "Error: XML Node Values is missing for Item: " << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processDoubleItem(pugi::xml_node &node,
                                          DoubleItemPtr item)
{
  this->processValueItem(node,
                         dynamicCastPointer<ValueItem>(item));
  if (item->isDiscrete())
    {
    return; // nothing left to do
    }

  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::string nodeName, expName;
  AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;
  if (!numRequiredVals)
    {
    // The node should have an attribute indicating how many values are 
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
      {
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      if (nodeName == "Val")
        {
        item->setValue(i, val.text().as_double());
        }
      else if (allowsExpressions && (nodeName == "Expression"))
        {
        expName = val.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (expAtt == NULL)
          {
          info.item = item; info.pos = i; info.expName = expName;
          this->m_itemExpressionInfo.push_back(info);
          }
        else
          {
          item->setExpression(i, expAtt);
          }
        }
      else
        {
        this->m_errorStatus << "Error: Unsupported Value Node Type  Item: " 
                            << item->name() 
                            << "\n";
        }
      }
    }
  else if (numRequiredVals == 1)
    {
    // Lets see if the value is set
    if (node.text())
      {
      // Is this an exapression?
      xatt = node.attribute("Expression");
      if (allowsExpressions && xatt)
        {
        expName = node.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (expAtt == NULL)
          {
          info.item = item; info.pos = 0; info.expName = expName;
          this->m_itemExpressionInfo.push_back(info);
          }
        else
          {
          item->setExpression(expAtt);
          }
        }
      else
        {
        item->setValue(node.text().as_double());
        }
      }
    }
  else
    {
    this->m_errorStatus << "Error: XML Node Values is missing for Item: " << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processFileItem(pugi::xml_node &node,
                                        FileItemPtr item)
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
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      item->setValue(i, val.text().get());
      }
    }
  else if (numRequiredVals == 1)
    {
    val = node.child("Val");
    if (val != NULL)
      {
      item->setValue(val.text().get());
      }
    }
  else
    {
    this->m_errorStatus << "Error: XML Node Values is missing for Item: " << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processGroupItem(pugi::xml_node &node,
                                         GroupItemPtr item)
{
  std::size_t i, j, m, n;
  std::size_t  numRequiredGroups = item->numberOfRequiredGroups();
  xml_node itemNode;
  xml_attribute xatt;
  n = item->numberOfGroups();
  m = item->numberOfItemsPerGroup();
  if (!numRequiredGroups)
    {
    // The node should have an attribute indicating how many groups are 
    // associated with the item
    xatt = node.attribute("NumberOfGroups");
    if (!xatt)
      {
      this->m_errorStatus << "Error: XML Attribute NumberOfGroups is missing for Group Item: " 
                          << item->name() 
                          << "\n";
      return;
      }
    n = xatt.as_uint();
    item->setNumberOfGroups(n);
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
        this->m_errorStatus << "Error: Too many sub-groups for Group Item: " << item->name() 
                            << "\n";
        continue;
        }
      for (itemNode = cluster.first_child(), j = 0; itemNode;
           itemNode = itemNode.next_sibling(), j++)
        {
        if (j >= m)
          {
          this->m_errorStatus << "Error: Too many item nodes for subGroup: " << i
                              << " for Group Item: " << item->name() 
                              << "\n";
          continue;
          }
        this->processItem(itemNode, item->item(i,j));
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
        this->m_errorStatus << "Error: Too many item nodes for subGroup:0"
                              << " for Group Item: " << item->name() 
                              << "\n";
          continue;
          }
        this->processItem(itemNode, item->item(j));
        }
    }
  else
    {
    this->m_errorStatus << "Error: XML Node GroupClusters is missing for Item: " << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processIntItem(pugi::xml_node &node,
                                       IntItemPtr item)
{
  this->processValueItem(node,
                         dynamicCastPointer<ValueItem>(item));
  if (item->isDiscrete())
    {
    return; // nothing left to do
    }

  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::string nodeName, expName;
  AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;
  if (!numRequiredVals)
    {
    // The node should have an attribute indicating how many values are 
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
      {
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      if (nodeName == "Val")
        {
        item->setValue(i, val.text().as_int());
        }
      else if (allowsExpressions && (nodeName == "Expression"))
        {
        expName = val.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (expAtt == NULL)
          {
          info.item = item; info.pos = i; info.expName = expName;
          this->m_itemExpressionInfo.push_back(info);
          }
        else
          {
          item->setExpression(i, expAtt);
          }
        }
      else
        {
        this->m_errorStatus << "Error: Unsupported Value Node Type  Item: " 
                            << item->name() 
                            << "\n";
        }
      }
    }
  else if (numRequiredVals == 1)
    {
    // Lets see if the value is set
    if (node.text())
      {
      // Is this an exapression?
      xatt = node.attribute("Expression");
      if (allowsExpressions && xatt)
        {
        expName = node.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (expAtt == NULL)
          {
          info.item = item; info.pos = 0; info.expName = expName;
          this->m_itemExpressionInfo.push_back(info);
          }
        else
          {
          item->setExpression(expAtt);
          }
        }
      else
        {
        item->setValue(node.text().as_int());
        }
      }
    }
  else
    {
    this->m_errorStatus << "Error: XML Node Values is missing for Item: " 
                        << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processStringItem(pugi::xml_node &node,
                                          StringItemPtr item)
{
  this->processValueItem(node,
                         dynamicCastPointer<ValueItem>(item));
  if (item->isDiscrete())
    {
    return; // nothing left to do
    }

  xml_attribute xatt;
  xml_node valsNode;
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  std::string nodeName, expName;
  AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;
  if (!numRequiredVals)
    {
    // The node should have an attribute indicating how many values are 
    // associated with the item
    xatt = node.attribute("NumberOfValues");
    if (!xatt)
      {
      this->m_errorStatus << "Error: XML Attribute NumberOfValues is missing for Item: " 
                          << item->name() 
                          << "\n";
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
        this->m_errorStatus << "Error: XML Attribute Ith is missing for Item: " << item->name() 
                            << "\n";
        continue;
        }
      i = xatt.as_uint();
      if (i >= n)
        {
        this->m_errorStatus << "Error: XML Attribute Ith = " << i
                            << " and is out of range for Item: " << item->name() 
                            << "\n";
        continue;
        }
      if (nodeName == "Val")
        {
        item->setValue(i, val.text().get());
        }
      else if (allowsExpressions && (nodeName == "Expression"))
        {
        expName = val.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (expAtt == NULL)
          {
          info.item = item; info.pos = i; info.expName = expName;
          this->m_itemExpressionInfo.push_back(info);
          }
        else
          {
          item->setExpression(i, expAtt);
          }
        }
      else
        {
        this->m_errorStatus << "Error: Unsupported Value Node Type  Item: " 
                            << item->name() 
                            << "\n";
        }
      }
    }
  else if (numRequiredVals == 1)
    {
    // Lets see if the value is set
    if (node.text())
      {
      // Is this an exapression?
      xatt = node.attribute("Expression");
      if (allowsExpressions && xatt)
        {
        expName = node.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (expAtt == NULL)
          {
          info.item = item; info.pos = 0; info.expName = expName;
          this->m_itemExpressionInfo.push_back(info);
          }
        else
          {
          item->setExpression(expAtt);
          }
        }
      else
        {
        item->setValue(node.text().get());
        }
      }
    }
  else
    {
    this->m_errorStatus << "Error: XML Node Values is missing for Item: " 
                        << item->name() 
                        << "\n";
    }
}
//----------------------------------------------------------------------------
bool XmlV1StringReader::getColor(xml_node &node, double color[3],
                                 const std::string &colorName)
{
  xml_attribute xatt;
  xatt = node.attribute("R");
  if (xatt)
    {
    color[0] = xatt.as_double();
    }
  else
    {
    this->m_errorStatus << "Error: Missing XML Attribute R for " 
                        << colorName
                        << "\n";
    return false;
    }

  xatt = node.attribute("G");
  if (xatt)
    {
    color[1] = xatt.as_double();
    }
  else
    {
    this->m_errorStatus << "Error: Missing XML Attribute G for " 
                        << colorName
                        << "\n";
    return false;
    }

  xatt = node.attribute("B");
  if (xatt)
    {
    color[2] = xatt.as_double();
    }
  else
    {
    this->m_errorStatus << "Error: Missing XML Attribute B for " 
                        << colorName
                        << "\n";
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void XmlV1StringReader::processSections(xml_node &root)
{
  xml_node sections = root.child("RootSection");
  if (!sections)
    {
    return;
    }
  slctk::RootSectionPtr rs = this->m_manager.rootSection();
  xml_node node;
  xml_attribute xatt;
  double c[3];
  node = sections.child("DefaultColor");
  if (node && this->getColor(node, c, "DefaultColor"))
    {
    rs->setDefaultColor(c);
    }
  node = sections.child("InvalidColor");
  if (node && this->getColor(node, c, "InvalidColor"))
    {
    rs->setInvalidColor(c);
    }
  this->processGroupSection(sections,
                            slctk::dynamicCastPointer<GroupSection>(rs));
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttributeSection(xml_node &node,
                                            slctk::AttributeSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  xml_attribute xatt;
  AttributeDefinitionPtr def;
  xml_node child, attTypes;
  std::string defType;
  xatt = node.attribute("ModelEnityFilter");
  if (xatt)
    {
    unsigned long mask = this->decodeModelEntityMask(xatt.value());
    sec->setModelEntityMask(mask);

    xatt = node.attribute("CreateEntities");
    if (xatt)
      {
      sec->setOkToCreateModelEntities(xatt.as_bool());
      }
    }
  attTypes = node.child("AttributeTypes");
  if (!attTypes)
    {
    return;
    }
  for (child = attTypes.child("Type"); child; child = child.next_sibling("Type"))
    {
    defType = child.text().get();
    def = this->m_manager.findDefinition(defType);
    if (def)
      {
      sec->addDefinition(def);
      }
    else
      {
      this->m_errorStatus << "Error: Cannot find attribute definition: " << defType 
                          << " required for Attribute Section: " << sec->title()
                          << "\n";
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processInstancedSection(xml_node &node,
                                                  slctk::InstancedSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  xml_attribute xatt;
  xml_node child, instances = node.child("InstancedAttributes");
  std::string attName, defName;
  AttributePtr att;
  AttributeDefinitionPtr attDef;

  if (!instances)
    {
    return; // No instances are in the section
    }

  for (child = instances.child("Att"); child; child = child.next_sibling("Att"))
    {
    attName = child.text().get();
    // See if the attribute exists and if not then create it
    att = this->m_manager.findAttribute(attName);
    if (!att)
      {
      xatt = child.attribute("Type");
      if (xatt)
        {
        defName = xatt.value();
        attDef = this->m_manager.findDefinition(defName);
        if (!attDef)
          {
          this->m_errorStatus << "Error: Cannot find attribute definition: " << defName
                              << " required to create attribute: " << attName 
                              << " for Instanced Section: " << sec->title()
                              << "\n";
          continue;
          }
        else
          {
          att = this->m_manager.createAttribute(attName, attDef);
          }
        }
      else
        {
        this->m_errorStatus << "Error: XML Attribute Type is missing"
                            << "and is required to create attribute: " << attName 
                            << " for Instanced Section: " << sec->title()
                            << "\n";
        continue;
        }
      }
    sec->addInstance(att);
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processModelEntitySection(xml_node &node,
                                                  slctk::ModelEntitySectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  xml_attribute xatt = node.attribute("ModelEnityFilter");
  xml_node child = node.child("Definition");
  if (xatt)
    {
    unsigned long mask = this->decodeModelEntityMask(xatt.value());
    sec->setModelEntityMask(mask);
    }

  if (child)
    {
    std::string defType = child.text().get();
    AttributeDefinitionPtr def = this->m_manager.findDefinition(defType);
    if (!def)
      {
      this->m_errorStatus << "Error: Cannot find attribute definition: " << defType
                          << " for Model Entity Section: " << sec->title()
                          << "\n";
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processSimpleExpressionSection(xml_node &node,
                                                       slctk::SimpleExpressionSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  xml_node child = node.child("Definition");
  if (child)
    {
    std::string defType = child.text().get();
    AttributeDefinitionPtr def = this->m_manager.findDefinition(defType);
    if (!def)
      {
      this->m_errorStatus << "Error: Cannot find attribute definition: " << defType
                          << " for Simple Expression Section: " << sec->title()
                          << "\n";
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processGroupSection(xml_node &node,
                                            slctk::GroupSectionPtr group)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(group));

  xml_node child;
  std::string sectionType;
  for (child = node.first_child(); child; child = child.next_sibling())
    {
    sectionType = child.name();
    if (sectionType == "AttributeSection")
      {
      this->processAttributeSection(child, 
                                    group->addSubsection<AttributeSectionPtr>(""));
      continue;
      }
    
    if (sectionType == "GroupSection")
      {
      this->processGroupSection(child, 
                                group->addSubsection<GroupSectionPtr>(""));
      continue;
      }
    
    if (sectionType == "InstancedSection")
      {
      this->processInstancedSection(child, 
                                    group->addSubsection<InstancedSectionPtr>(""));
      continue;
      }
    
    if (sectionType == "ModelEntitySection")
      {
      this->processModelEntitySection(child, 
                                      group->addSubsection<ModelEntitySectionPtr>(""));
      continue;
      }
    
    if (sectionType == "SimpleExpressionSection")
      {
      this->processSimpleExpressionSection(child, 
                                           group->addSubsection<SimpleExpressionSectionPtr>(""));
      continue;
      }
    
    // In case this was root section
    if ((group->type() == Section::ROOT) && ((sectionType == "DefaultColor") ||
                                             (sectionType == "InvalidColor")))
      {
      continue;
      }

    this->m_errorStatus << "Error: Unsupported Section Type: " << sectionType
                        << " for Group Section: " << group->title()
                        << "\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processBasicSection(xml_node &node,
                                            slctk::SectionPtr sec)
{
  xml_attribute xatt;
  xatt = node.attribute("Title"); // Required
  if (!xatt)
    {
    this->m_errorStatus << "Error: Section is missing XML Attribute Title\n";
    }
  else
    {
    sec->setTitle(xatt.value());
    }
  xatt = node.attribute("Icon"); // optional
  if (xatt)
    {
    sec->setIconName(xatt.value());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processModelInfo(xml_node &root)
{
  //xml_node modelInfo = this->m_root.append_child("ModelInfo");
}
//----------------------------------------------------------------------------
unsigned long  XmlV1StringReader::decodeModelEntityMask(const std::string &s)
{
  unsigned long m = 0;
  std::size_t i, n = s.length();
  for (i = 0; i < n; i++)
    {
    switch (s[i])
      {
      case 'd':
        m |= 0x40;
        break;
      case 'b':
        m |= 0x20;
        break;
      case 'm':
        m |= 0x10;
        break;
      case 'r':
        m |= 0x8;
        break;
      case 'f':
        m |= 0x4;
        break;
      case 'e':
        m |= 0x2;
        break;
      case 'v':
        m |= 0x1;
        break;
      default:
        std::cerr << "Option " << s[i] << " is not supported\n";
      }
    }
  return m;
}
//----------------------------------------------------------------------------
