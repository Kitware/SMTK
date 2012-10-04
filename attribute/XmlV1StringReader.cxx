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

/*
  // Now lets process its items
  std::size_t i, n = idef->numberOfItemDefinitions();
  if (n != 0)
    {
    itemDefNodes = node.append_child("ItemDefinitions");
    for (i = 0; i < n; i++)
      {
      itemDefNode = itemDefNodes.append_child();
      itemDefNode.set_name(Item::type2String(idef->itemDefinition(i)->type()).c_str());
      this->processItemDefinition(itemDefNode, 
                                  idef->itemDefinition(i));
      }
    }
*/
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttribute(xml_node &attNode)
{
/*
  xml_node node = attributes.append_child("Att");
  node.append_attribute("Name").set_value(att->name().c_str());
  if (att->definition() != NULL)
    {
    node.append_attribute("Type").set_value(att->definition()->type().c_str());
    if (att->definition()->isNodal())
      {
      node.append_attribute("OnInteriorNodes").set_value(att->appliesToInteriorNodes());
      node.append_attribute("OnBoundaryNodes").set_value(att->appliesToBoundaryNodes());
      }
    }
  node.append_attribute("ID").set_value((unsigned int)att->id());
  std::size_t i, n = att->numberOfItems();
  if (n)
    {
    xml_node itemNode, items = node.append_child("Items");
    for (i = 0; i < n; i++)
      {
      itemNode = items.append_child();
      itemNode.set_name(Item::type2String(att->item(i)->type()).c_str());
      this->processItem(itemNode, att->item(i));
      }
    }
*/
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processItem(xml_node &node, 
                                    AttributeItemPtr item)
{
  node.append_attribute("Name").set_value(item->name().c_str());
  if (item->isOptional())
    {
    node.append_attribute("Enabled").set_value(item->isEnabled());
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
      std::cerr << "Unsupported Type!\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processValueItem(pugi::xml_node &node,
                                         ValueItemPtr item)
{
  if (!item->isDiscrete())
    {
    return; // there is nothing to be done
    }
  std::size_t i, n = item->numberOfValues();
  xml_node val, values;
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1) // Special Common Case
    {
    node.append_attribute("Discrete").set_value(true);
    if (item->isSet())
      {
      node.text().set(item->discreteIndex());
      }
    return;
    }
  values = node.append_child("DiscreteValues");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      val = values.append_child("Index");
      val.append_attribute("Ith").set_value((unsigned int) i);
      val.text().set(item->discreteIndex(i));
      }
    else
      {
      val = values.append_child("UnsetDiscreteVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttributeRefItem(pugi::xml_node &node,
                                               AttributeRefItemPtr item)
{
  std::size_t i, n = item->numberOfValues();
  xml_node val;
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1)
    {
    if (item->isSet())
      {
      val = node.append_child("Val");
      val.append_attribute("Ith").set_value((unsigned int) i);
      val.text().set(item->value(i)->name().c_str());
      }
    return;
    }
  xml_node values = node.append_child("Values");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value((unsigned int) i);
      val.text().set(item->value(i)->name().c_str());
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processDirectoryItem(pugi::xml_node &node,
                                             DirectoryItemPtr item)
{
  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1)
    {
    if (item->isSet())
      {
      node.text().set(item->value().c_str());
      }
    return;
    }
  xml_node val, values = node.append_child("Values");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value((unsigned int) i);
      val.text().set(item->value(i).c_str());
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
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
  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1)
    {
    if (item->isSet())
      {
      if (item->isExpression())
        {
        node.append_attribute("Expression").set_value(true);
        node.text().set(item->expression()->name().c_str());
        }
      else
        {
        node.text().set(item->value());
        }
      }
    return;
    }
  xml_node val, values = node.append_child("Values");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      if (item->isExpression(i))
        {
        val = values.append_child("Expression");
        val.append_attribute("Ith").set_value((unsigned int) i);
        val.text().set(item->expression(i)->name().c_str());
        }
      else
        {
        val = values.append_child("Val");
        val.append_attribute("Ith").set_value((unsigned int) i);
        val.text().set(item->value(i));
        }
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processFileItem(pugi::xml_node &node,
                                        FileItemPtr item)
{
  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1)
    {
    if (item->isSet())
      {
      node.text().set(item->value().c_str());
      }
    return;
    }
  xml_node val, values = node.append_child("Values");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value((unsigned int) i);
      val.text().set(item->value(i).c_str());
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processGroupItem(pugi::xml_node &node,
                                         GroupItemPtr item)
{
  std::size_t i, j, m, n;
  xml_node itemNode;
  n = item->numberOfGroups();
  m = item->numberOfItemsPerGroup();
  if (!n)
    {
    return;
    }
  // Optimize for number of required groups = 1
  if (item->numberOfRequiredGroups() == 1)
    {
    for (j = 0; j < m; j++)
      {
      itemNode = node.append_child();
      itemNode.set_name(Item::type2String(item->item(j)->type()).c_str());
      this->processItem(itemNode, item->item(j));
      }
    return;
    }
  xml_node cluster, clusters = node.append_child("GroupClusters");
  for(i = 0; i < n; i++)
    {
    cluster = clusters.append_child("Cluster");
    cluster.append_attribute("Ith").set_value((unsigned int) i);
    for (j = 0; j < m; j++)
      {
      itemNode = cluster.append_child();
      itemNode.set_name(Item::type2String(item->item(i,j)->type()).c_str());
      this->processItem(itemNode, item->item(i,j));
      }
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
  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1)
    {
    if (item->isSet())
      {
      if (item->isExpression())
        {
        node.append_attribute("Expression").set_value(true);
        node.text().set(item->expression()->name().c_str());
        }
      else
        {
        node.text().set(item->value());
        }
      }
    return;
    }
  xml_node val, values = node.append_child("Values");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      if (item->isExpression(i))
        {
        val = values.append_child("Expression");
        val.append_attribute("Ith").set_value((unsigned int) i);
        val.text().set(item->expression(i)->name().c_str());
        }
      else
        {
        val = values.append_child("Val");
        val.append_attribute("Ith").set_value((unsigned int) i);
        val.text().set(item->value(i));
        }
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
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
  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  if (item->numberOfRequiredValues() == 1)
    {
    if (item->isSet())
      {
      if (item->isExpression())
        {
        node.append_attribute("Expression").set_value(true);
        node.text().set(item->expression()->name().c_str());
        }
      else
        {
        node.text().set(item->value().c_str());
        }
      }
    return;
    }
  xml_node val, values = node.append_child("Values");
  for(i = 0; i < n; i++)
    {
    if (item->isSet(i))
      {
      if (item->isExpression(i))
        {
        val = values.append_child("Expression");
        val.append_attribute("Ith").set_value((unsigned int) i);
        val.text().set(item->expression(i)->name().c_str());
        }
      else
        {
        val = values.append_child("Val");
        val.append_attribute("Ith").set_value((unsigned int) i);
        val.text().set(item->value(i).c_str());
        }
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value((unsigned int) i);
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processSections(xml_node &root)
{
  this->m_root.append_child(node_comment).set_value("********** Workflow Sections ***********");
  xml_node sections = this->m_root.append_child("RootSection");
  slctk::RootSectionPtr rs = this->m_manager.rootSection();
  xml_node node;
  double c[3];
  rs->defaultColor(c);
  node = sections.append_child("DefaultColor");
  node.append_attribute("R").set_value(c[0]);
  node.append_attribute("G").set_value(c[1]);
  node.append_attribute("B").set_value(c[2]);
  rs->invalidColor(c);
  node = sections.append_child("InvalidColor");
  node.append_attribute("R").set_value(c[0]);
  node.append_attribute("G").set_value(c[1]);
  node.append_attribute("B").set_value(c[2]);
  this->processGroupSection(sections,
                            slctk::dynamicCastPointer<GroupSection>(rs));
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processAttributeSection(xml_node &node,
                                            slctk::AttributeSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  if (sec->modelEntityMask())
    {
    // std::string s = this->encodeModelEntityMask(sec->modelEntityMask());
    // node.append_attribute("ModelEnityFilter").set_value(s.c_str());
    if (sec->okToCreateModelEntities())
      {
      node.append_attribute("CreateEntities").set_value(true);
      }
    }
  std::size_t i, n = sec->numberOfDefinitions();
  if (n)
    {
    xml_node atypes = node.append_child("AttributeTypes");
    for (i = 0; i < n; i++)
      {
      atypes.append_child("Type").text().set(sec->definition(i)->type().c_str());
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processInstancedSection(xml_node &node,
                                                  slctk::InstancedSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  std::size_t i, n = sec->numberOfInstances();
   if (n)
    {
    xml_node instances = node.append_child("InstancedAttributes");
    for (i = 0; i < n; i++)
      {
      instances.append_child("Att").text().set(sec->instance(i)->name().c_str());
      }
    }
 
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processModelEntitySection(xml_node &node,
                                                  slctk::ModelEntitySectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  if (sec->modelEntityMask())
    {
    // std::string s = this->encodeModelEntityMask(sec->modelEntityMask());
    // node.append_attribute("ModelEnityFilter").set_value(s.c_str());
    }
  if (sec->definition() != NULL)
    {
    node.append_child("Definition").text().set(sec->definition()->type().c_str());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processSimpleExpressionSection(xml_node &node,
                                                       slctk::SimpleExpressionSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  if (sec->definition() != NULL)
    {
    node.append_child("Definition").text().set(sec->definition()->type().c_str());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processGroupSection(xml_node &node,
                                            slctk::GroupSectionPtr group)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(group));
  std::size_t i, n = group->numberOfSubsections();
  xml_node child;
  slctk::SectionPtr sec;
  for (i = 0; i < n; i++)
    {
    sec = group->subsection(i);
    switch(sec->type())
      {
      case Section::ATTRIBUTE:
        child = node.append_child("AttributeSection");
        this->processAttributeSection(child, 
                                      slctk::dynamicCastPointer<AttributeSection>(sec));
        break;
      case Section::GROUP:
        child = node.append_child("GroupSection");
        this->processGroupSection(child, 
                                  slctk::dynamicCastPointer<GroupSection>(sec));
        break;
      case Section::INSTANCED:
        child = node.append_child("InstancedSection");
        this->processInstancedSection(child, 
                                      slctk::dynamicCastPointer<InstancedSection>(sec));
        break;
      case Section::MODEL_ENTITY:
        child = node.append_child("ModelEntitySection");
        this->processModelEntitySection(child, 
                                        slctk::dynamicCastPointer<ModelEntitySection>(sec));
        break;
      case Section::SIMPLE_EXPRESSION:
        child = node.append_child("SimpleExpressionSection");
        this->processSimpleExpressionSection(child, 
                                             slctk::dynamicCastPointer<SimpleExpressionSection>(sec));
        break;
      default:
        std::cerr << "Unsupport Section Type " << Section::type2String(sec->type()) << std::endl;
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processBasicSection(xml_node &node,
                                            slctk::SectionPtr sec)
{
  node.append_attribute("Title").set_value(sec->title().c_str());
  if (sec->iconName() != "")
    {
    node.append_attribute("Icon").set_value(sec->title().c_str());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringReader::processModelInfo(xml_node &root)
{
  xml_node modelInfo = this->m_root.append_child("ModelInfo");
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
