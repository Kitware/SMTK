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


#include "smtk/util/XmlDocV1Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.cpp"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
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
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/model/Item.h"
#include "smtk/model/GroupItem.h"
#include "smtk/model/Model.h"
#include "smtk/view/Attribute.h"
#include "smtk/view/Instanced.h"
#include "smtk/view/Group.h"
#include "smtk/view/ModelEntity.h"
#include "smtk/view/Root.h"
#include "smtk/view/SimpleExpression.h"
#include <iostream>

using namespace pugi;
using namespace smtk::util;
using namespace smtk;
//----------------------------------------------------------------------------
XmlDocV1Parser::XmlDocV1Parser(smtk::attribute::Manager &myManager):
m_manager(myManager)
{
}

//----------------------------------------------------------------------------
XmlDocV1Parser::~XmlDocV1Parser()
{
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::process(xml_document &doc)
{
  // Reset the message log
  this->m_logger.reset();
  // Clear the vectors for dealing with attribute references
  this->m_itemExpressionDefInfo.clear();
  this->m_attRefDefInfo.clear();
  this->m_itemExpressionInfo.clear();
  this->m_attRefInfo.clear();
  this->m_logger.reset();
  xml_node amnode, node, cnode;
  // Get the attribute manager node
  amnode = doc.child("SMTK_AttributeManager");

  // Get the category information
  std::set<std::string> secCatagories;
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

  // Process Analsis Info
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
      this->m_manager.defineAnalysis(s, catagories);
      }
    }
  this->processAttributeInformation(amnode);
  this->processViews(amnode);
  this->processModelInfo(amnode);

  // Now we need to check to see if there are any catagories in the manager
  // that were not explicitly listed in the catagories section - first update catagories
  this->m_manager.updateCategories();

  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = this->m_manager.categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    if (secCatagories.find(*it) == secCatagories.end())
      {
      smtkErrorMacro(this->m_logger, "Catagory: " << *it
                     << " was not listed in Manger's Catagory Section");
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processAttributeInformation(xml_node &root)
{
  // Process definitions first
  xml_node child, node = root.child("Definitions");
  if (!node)
    {
    smtkErrorMacro(this->m_logger, "Definition Section is missing!");
    return;
    }
  for (child = node.first_child(); child; child = child.next_sibling())
    {
    this->processDefinition(child);
    }

  // At this point we have all the definitions read in so lets
  // fix up all of the attribute definition references
  std::size_t i;
  attribute::DefinitionPtr def;
  for (i = 0; i < this->m_itemExpressionDefInfo.size(); i++)
    {
    def = this->m_manager.findDefinition(this->m_itemExpressionDefInfo[i].second);
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
    def = this->m_manager.findDefinition(this->m_attRefDefInfo[i].second);
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

  node = root.child("Attributes");
  if (!node)
    {
    smtkErrorMacro(this->m_logger,"Attributes Section is missing!");
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
  attribute::AttributePtr att;
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
      smtkErrorMacro(this->m_logger,
                     "Expression Attribute: "
                     << this->m_itemExpressionInfo[i].expName
                     << " is missing and required by Item : "
                     << this->m_itemExpressionInfo[i].item->name());
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
      smtkErrorMacro(this->m_logger,
                     "Referenced Attribute: "
                     << this->m_attRefInfo[i].attName
                     << " is missing and required by Item: "
                     << this->m_attRefInfo[i].item->name());
      }
    }

}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processDefinition(xml_node &defNode)
{
  xml_node node;
  attribute::DefinitionPtr def, baseDef;
  std::string type = defNode.attribute("Type").value();
  std::string baseType = defNode.attribute("BaseType").value();
  if (baseType != "")
    {
    baseDef = this->m_manager.findDefinition(baseType);
    if (!baseDef)
      {
      smtkErrorMacro(this->m_logger, "Could not find Base Definition: "
                     << baseType << " needed to create Definition: "
                     << type);
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
      case smtk::attribute::Item::VOID:
        idef = def->addItemDefinition<smtk::attribute::VoidItemDefinition>(itemName);
        this->processItemDef(node, idef);
      break;
    default:
      smtkErrorMacro(this->m_logger, "Unsupported Item definition Type: "
                     << node.name()
                     << " needed to create Definition: " << type);
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
        smtkErrorMacro(this->m_logger,
                       "Missing XML Attribute Enum in DiscreteInfo section of Double Item Definition : "
                       << idef->name());
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
void XmlDocV1Parser::processIntDef(pugi::xml_node &node,
                                      attribute::IntItemDefinitionPtr idef)
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
        smtkErrorMacro(this->m_logger,
                       "Missing XML Attribute Enum in DiscreteInfo section of Int Item Definition : "
                       << idef->name());
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
void XmlDocV1Parser::processStringDef(pugi::xml_node &node,
                                         attribute::StringItemDefinitionPtr idef)
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
        smtkErrorMacro(this->m_logger,
                       "Missing XML Attribute Enum in DiscreteInfo section of String Item Definition : "
                       << idef->name());
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
void XmlDocV1Parser::processValueDef(pugi::xml_node &node,
                                        attribute::ValueItemDefinitionPtr idef)
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
    attribute::DefinitionPtr adef = this->m_manager.findDefinition(etype);
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
    // Is the attribute definition already in the manager?
    std::string etype = child.text().get();
    attribute::DefinitionPtr adef = this->m_manager.findDefinition(etype);
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
    def->setNumberOfRequiredGroups(xatt.as_int());
    }
  else
    {
    smtkErrorMacro(this->m_logger,
                   "Missing XML Attribute NumberOfRequiredGroups for Item Definition : "
                   << def->name());
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
        this->processRefDef(child, smtk::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DOUBLE:
        idef = def->addItemDefinition<smtk::attribute::DoubleItemDefinition>(itemName);
        this->processDoubleDef(child, smtk::dynamic_pointer_cast<smtk::attribute::DoubleItemDefinition>(idef));
        break;
      case smtk::attribute::Item::DIRECTORY:
        idef = def->addItemDefinition<smtk::attribute::DirectoryItemDefinition>(itemName);
        this->processDirectoryDef(child, smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(idef));
        break;
      case smtk::attribute::Item::FILE:
        idef = def->addItemDefinition<smtk::attribute::FileItemDefinition>(itemName);
        this->processFileDef(child, smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(idef));
        break;
      case smtk::attribute::Item::GROUP:
        idef = def->addItemDefinition<smtk::attribute::GroupItemDefinition>(itemName);
        this->processGroupDef(child, smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(idef));
        break;
      case smtk::attribute::Item::INT:
        idef = def->addItemDefinition<smtk::attribute::IntItemDefinition>(itemName);
        this->processIntDef(child, smtk::dynamic_pointer_cast<smtk::attribute::IntItemDefinition>(idef));
        break;
      case smtk::attribute::Item::STRING:
        idef = def->addItemDefinition<smtk::attribute::StringItemDefinition>(itemName);
        this->processStringDef(child, smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(idef));
        break;
      case smtk::attribute::Item::VOID:
        idef = def->addItemDefinition<smtk::attribute::VoidItemDefinition>(itemName);
        this->processItemDef(child, idef);
        break;
      default:
        smtkErrorMacro(this->m_logger,
                       "Unsupported Item definition Type: " << child.name()
                       << " needed to create Group Definition: " << def->name());
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processAttribute(xml_node &attNode)
{
  xml_node itemsNode, iNode, node;
  std::string name, type;
  xml_attribute xatt;
  attribute::AttributePtr att;
  attribute::DefinitionPtr def;
  unsigned long id, maxId = 0;
  std::size_t i, n;

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
  xatt = attNode.attribute("ID");
  if (!xatt)
    {
    smtkErrorMacro(this->m_logger,
                   "Invalid Attribute: " << name
                   << "  - Missing XML Attribute ID");
    return;
    }
  id = xatt.as_uint();

  def = this->m_manager.findDefinition(type);
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

  att = this->m_manager.createAttribute(name, def, id);

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
  if (!item->isDiscrete())
    {
    return; // there is nothing to be done
    }
  xml_node val, values;
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
      if (!item->setDiscreteIndex(i, index))
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
      att = this->m_manager.findAttribute(attName);
      if (!att)
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
    if (val)
      {
      attName = val.text().get();
      att = this->m_manager.findAttribute(attName);
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
      item->setValue(i, val.text().get());
      }
    }
  else if (numRequiredVals == 1)
    {
    val = node.child("Val");
    if (val)
      {
      item->setValue(val.text().get());
      }
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
  attribute::AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;
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
      if (nodeName == "Val")
        {
        item->setValue(i, val.text().as_double());
        }
      else if (allowsExpressions && (nodeName == "Expression"))
        {
        expName = val.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (!expAtt)
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
        smtkErrorMacro(this->m_logger, "Unsupported Value Node Type  Item: "
                       << item->name());
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
        if (!expAtt)
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
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
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
      item->setValue(i, val.text().get());
      }
    }
  else if (numRequiredVals == 1)
    {
    val = node.child("Val");
    if (val)
      {
      item->setValue(val.text().get());
      }
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
  if (!numRequiredGroups)
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
        smtkErrorMacro(this->m_logger,
                       "Too many item nodes for subGroup: 0"
                       << " for Group Item: " << item->name());
          continue;
          }
        this->processItem(itemNode, item->item(j));
        }
    }
  else
    {
    smtkErrorMacro(this->m_logger,"XML Node GroupClusters is missing for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processIntItem(pugi::xml_node &node,
                                       attribute::IntItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<smtk::attribute::ValueItem>(item));
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
  attribute::AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;
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
      if (nodeName == "Val")
        {
        item->setValue(i, val.text().as_int());
        }
      else if (allowsExpressions && (nodeName == "Expression"))
        {
        expName = val.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (!expAtt)
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
        smtkErrorMacro(this->m_logger, "Unsupported Value Node Type  Item: "
                       << item->name());
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
        if (!expAtt)
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
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processStringItem(pugi::xml_node &node,
                                          attribute::StringItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<smtk::attribute::ValueItem>(item));
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
  attribute::AttributePtr expAtt;
  bool allowsExpressions = item->allowsExpressions();
  ItemExpressionInfo info;
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
      if (nodeName == "Val")
        {
        item->setValue(i, val.text().get());
        }
      else if (allowsExpressions && (nodeName == "Expression"))
        {
        expName = val.text().get();
        expAtt = this->m_manager.findAttribute(expName);
        if (!expAtt)
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
        smtkErrorMacro(this->m_logger, "Unsupported Value Node Type  Item: "
                       << item->name());
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
        if (!expAtt)
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
    smtkErrorMacro(this->m_logger, "XML Node Values is missing for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
bool XmlDocV1Parser::getColor(xml_node &node, double color[4],
                                 const std::string &colorName)
{
  std::string s = node.text().get();
  int i = this->decodeColorInfo(s, color);
  if (i)
    {
    smtkErrorMacro(this->m_logger, "Color Format Probem - only found " << i
                   << "components for "
                   << colorName);
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
  smtk::view::RootPtr rs = this->m_manager.rootView();
  xml_node node;
  xml_attribute xatt;
  double c[4];
  node = views.child("DefaultColor");
  if (node && this->getColor(node, c, "DefaultColor"))
    {
    rs->setDefaultColor(c);
    }
  node = views.child("InvalidColor");
  if (node && this->getColor(node, c, "InvalidColor"))
    {
    rs->setInvalidColor(c);
    }
  node = views.child("AdvancedFontEffects");
  if (node)
    {
    if(xml_attribute xatt = node.attribute("Bold"))
      {
      rs->setAdvancedBold(strcmp(xatt.value(), "1"));
      }
    if(xml_attribute xatt = node.attribute("Italic"))
      {
      rs->setAdvancedItalic(strcmp(xatt.value(), "1"));
      }
    }

  this->processGroupView(views,
                         smtk::dynamic_pointer_cast<smtk::view::Group>(rs));
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processAttributeView(xml_node &node,
                                          smtk::view::AttributePtr v)
{
  this->processBasicView(node,
                            smtk::dynamic_pointer_cast<smtk::view::Base>(v));
  xml_attribute xatt;
  attribute::DefinitionPtr def;
  xml_node child, attTypes;
  std::string defType;
  xatt = node.attribute("ModelEntityFilter");
  if (xatt)
    {
    smtk::model::MaskType mask = this->decodeModelEntityMask(xatt.value());
    v->setModelEntityMask(mask);

    xatt = node.attribute("CreateEntities");
    if (xatt)
      {
      v->setOkToCreateModelEntities(xatt.as_bool());
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
      v->addDefinition(def);
      }
    else
      {
      smtkErrorMacro(this->m_logger,
                     "Cannot find attribute definition: " << defType
                     << " required for Attribute View: " << v->title());
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processInstancedView(xml_node &node,
                                          smtk::view::InstancedPtr v)
{
  this->processBasicView(node,
                         smtk::dynamic_pointer_cast<smtk::view::Base>(v));
  xml_attribute xatt;
  xml_node child, instances = node.child("InstancedAttributes");
  std::string attName, defName;
  attribute::AttributePtr att;
  attribute::DefinitionPtr attDef;

  if (!instances)
    {
    return; // No instances are in the view
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
          smtkErrorMacro(this->m_logger,
                         "Cannot find attribute definition: " << defName
                         << " required to create attribute: " << attName
                         << " for Instanced View: " << v->title());
          continue;
          }
        else
          {
          att = this->m_manager.createAttribute(attName, attDef);
          }
        }
      else
        {
        smtkErrorMacro(this->m_logger,
                       "XML Attribute Type is missing"
                       << "and is required to create attribute: " << attName
                       << " for Instanced View: " << v->title());
        continue;
        }
      }
    v->addInstance(att);
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processModelEntityView(xml_node &node,
                                            smtk::view::ModelEntityPtr v)
{
  this->processBasicView(node,
                         smtk::dynamic_pointer_cast<smtk::view::Base>(v));
  xml_attribute xatt = node.attribute("ModelEntityFilter");
  xml_node child = node.child("Definition");
  if (xatt)
    {
    smtk::model::MaskType mask = this->decodeModelEntityMask(xatt.value());
    v->setModelEntityMask(mask);
    }

  if (child)
    {
    std::string defType = child.text().get();
    attribute::DefinitionPtr def = this->m_manager.findDefinition(defType);
    if (!def)
      {
      smtkErrorMacro(this->m_logger,
                     "Cannot find attribute definition: " << defType
                     << " for Model Entity View: " << v->title());
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processSimpleExpressionView(xml_node &node,
                                                 smtk::view::SimpleExpressionPtr v)
{
  this->processBasicView(node,
                            smtk::dynamic_pointer_cast<smtk::view::Base>(v));
  xml_node child = node.child("Definition");
  if (child)
    {
    std::string defType = child.text().get();
    attribute::DefinitionPtr def = this->m_manager.findDefinition(defType);
    if (!def)
      {
      smtkErrorMacro(this->m_logger,
                     "Cannot find attribute definition: " << defType
                     << " for Simple Expression View: " << v->title());
      }
    else
      {
      v->setDefinition(def);
      }
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processGroupView(xml_node &node,
                                      smtk::view::GroupPtr group)
{
  this->processBasicView(node,
                         smtk::dynamic_pointer_cast<smtk::view::Base>(group));

  xml_node child;
  std::string childName;
  for (child = node.first_child(); child; child = child.next_sibling())
    {
    childName = child.name();
    if (childName == "AttributeView")
      {
      this->processAttributeView(child,
                                 group->addSubView<smtk::view::AttributePtr>(""));
      continue;
      }

    if (childName == "GroupView")
      {
      this->processGroupView(child,
                             group->addSubView<smtk::view::GroupPtr>(""));
      continue;
      }

    if (childName == "InstancedView")
      {
      this->processInstancedView(child,
                                 group->addSubView<smtk::view::InstancedPtr>(""));
      continue;
      }

    if (childName == "ModelEntityView")
      {
      this->processModelEntityView(child,
                                   group->addSubView<smtk::view::ModelEntityPtr>(""));
      continue;
      }

    if (childName == "SimpleExpressionView")
      {
      this->processSimpleExpressionView(child,
                                        group->addSubView<smtk::view::SimpleExpressionPtr>(""));
      continue;
      }

    // In case this was root section
    if ((group->type() == smtk::view::Base::ROOT) && ((childName == "DefaultColor") ||
                                                      (childName == "InvalidColor") ||
                                                      (childName == "AdvancedFontEffects")))
      {
      continue;
      }

    smtkErrorMacro(this->m_logger, "Unsupported View Type: " << childName
                   << " for Group View: " << group->title());
    }
}
//----------------------------------------------------------------------------
void XmlDocV1Parser::processBasicView(xml_node &node,
                                      smtk::view::BasePtr v)
{
  xml_attribute xatt;
  xatt = node.attribute("Title"); // Required
  if (!xatt)
    {
    smtkErrorMacro(this->m_logger, "View is missing XML Attribute Title");
    }
  else
    {
    v->setTitle(xatt.value());
    }
  xatt = node.attribute("Icon"); // optional
  if (xatt)
    {
    v->setIconName(xatt.value());
    }
}

//----------------------------------------------------------------------------
void XmlDocV1Parser::processModelInfo(xml_node &root)
{
  xml_node modelInfo = root.child("ModelInfo");
  smtk::model::ModelPtr refModel = this->m_manager.refModel();
  if ( modelInfo && refModel)
    {
    std::string name;
    smtk::model::MaskType mask;
    int gid;
    smtk::model::GroupItemPtr modelGroup;
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
      gid = xatt.as_int();
      xatt = gnode.attribute("Mask");
      if(!xatt)
        {
        smtkErrorMacro(this->m_logger, "Model Group is missing XML Attribute Mask");
        continue;
        }
      mask = xatt.as_int();
      xatt = gnode.attribute("Name");
      name = xatt ? xatt.value() : "noname-group";
      modelGroup = refModel->createModelGroup(name, gid, mask);
      if(modelGroup)
        {
        xml_node anode;
        for (anode = gnode.child("Attribute"); anode; anode = anode.next_sibling("Attribute"))
          {
          xatt = anode.attribute("Name");
          if(!xatt)
            {
            smtkErrorMacro(this->m_logger, "Model Group associated Attribute is missing XML Attribute Name");
            continue;
            }
          name = xatt.value();
          if(smtk::attribute::AttributePtr att = this->m_manager.findAttribute(name))
            {
            modelGroup->attachAttribute(att);
            }
          else
            {
            smtkErrorMacro(this->m_logger, "Can find Model Group associated Attribute with Name: " << name);
            }
          }
        }
      }
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
smtk::model::MaskType
XmlDocV1Parser::decodeModelEntityMask(const std::string &s)
{
  smtk::model::MaskType m = 0;
  std::size_t i, n = s.length();
  for (i = 0; i < n; i++)
    {
    switch (s[i])
      {
      case 'g':
        m |= smtk::model::Item::GROUP;
        break;
      case 'm':
        m |= smtk::model::Item::MODEL_DOMAIN;
        break;
      case 'r':
        m |= smtk::model::Item::REGION;
        break;
      case 'f':
        m |= smtk::model::Item::FACE;
        break;
      case 'e':
        m |= smtk::model::Item::EDGE;
        break;
      case 'v':
        m |= smtk::model::Item::VERTEX;
        break;
      default:
        smtkErrorMacro(this->m_logger,
                       "Decoding Model Entity Mask - Option "
                       << s[i] << " is not supported");
      }
    }
  return m;
}
//----------------------------------------------------------------------------
