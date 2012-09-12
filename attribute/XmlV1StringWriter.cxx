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


#include "attribute/XmlV1StringWriter.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.cpp"
#include "attribute/AttributeRefItemDefinition.h"
#include "attribute/Definition.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/DirectoryItemDefinition.h"
#include "attribute/FileItemDefinition.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/ItemDefinition.h"
#include "attribute/Manager.h"
#include "attribute/AttributeSection.h"
#include "attribute/InstancedSection.h"
#include "attribute/GroupSection.h"
#include "attribute/ModelEntitySection.h"
#include "attribute/RootSection.h"
#include "attribute/SimpleExpressionSection.h"
#include "attribute/StringItemDefinition.h"
#include "attribute/ValueItemDefinition.h"
#include <sstream>
#include <iostream>

using namespace pugi;
using namespace slctk::attribute; 
 
//----------------------------------------------------------------------------
XmlV1StringWriter::XmlV1StringWriter(const Manager &myManager):
m_manager(myManager)
{
  this->m_doc.append_child(node_comment).set_value("Created by XmlV1StringWriter");
  this->m_root = this->m_doc.append_child("SLCTK_AttributeManager");
  this->m_root.append_attribute("Version").set_value(1);
}

//----------------------------------------------------------------------------
XmlV1StringWriter::~XmlV1StringWriter()
{
}
//----------------------------------------------------------------------------
std::string XmlV1StringWriter::convertToString()
{
  // Write out the category and analysis information
  if (this->m_manager.numberOfCategories())
    {
    xml_node cnode, catNodes = this->m_root.append_child("Categories");
    std::set<std::string>::const_iterator it;
    const std::set<std::string> &cats = this->m_manager.categories();
    for (it = cats.begin(); it != cats.end(); it++)
      {
      catNodes.append_child("Cat").text().set(it->c_str());
      }
    }

  if (this->m_manager.numberOfAnalyses())
    {
    xml_node cnode, catNodes = this->m_root.append_child("Analyses");
    std::map<std::string, std::set<std::string> >::const_iterator it;
    const std::map<std::string, std::set<std::string> > &analyses = 
      this->m_manager.analyses();
    for (it = analyses.begin(); it != analyses.end(); it++)
      {
      xml_node anode = catNodes.append_child("Analysis");
      anode.append_attribute("Type").set_value(it->first.c_str());
      std::set<std::string>::const_iterator cit;
      for (cit = it->second.begin(); cit != it->second.end(); cit++)
        {
        anode.append_child("Cat").text().set(cit->c_str());
        }
      }
    }
  this->processDefinitions();
  this->processInstances();
  this->processSections();
  this->processModelInfo();
  std::stringstream oss;
  this->m_doc.save(oss, "  ");
  std::string result = oss.str();
  return result;
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDefinitions()
{
  std::vector<slctk::AttributeDefinitionPtr> baseDefs;
  this->m_manager.findBaseDefinitions(baseDefs);
  std::size_t i, n = baseDefs.size();
  xml_node definitions = this->m_root.append_child("Definitions");
  for (i = 0; i < n; i++)
    {
    this->processDefinition(definitions, baseDefs[i]);
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDefinition(xml_node &definitions,
                                          slctk::AttributeDefinitionPtr def)
{
  xml_node itemDefNode, itemDefNodes, 
    child, node = definitions.append_child();
  node.set_name("AttDef");
  node.append_attribute("Type").set_value(def->type().c_str());
  if (def->label() != "")
    {
    node.append_attribute("Label").set_value(def->label().c_str());
    }
  if (def->baseDefinition() != NULL)
    {
    node.append_attribute("BaseType").set_value(def->baseDefinition()->type().c_str());
    }
  else
    {
    node.append_attribute("BaseType").set_value("");
    }
  node.append_attribute("Version") = def->version();
  if (def->isAbstract())
    {
    node.append_attribute("Abstract").set_value("true");
    }
  if (def->advanceLevel())
    {
    node.append_attribute("AdvanceLevel") = def->advanceLevel();
    }
  if (def->isUnique())
    {
    node.append_attribute("Unique").set_value("true");
    }
  if (def->isNodal())
    {
    node.append_attribute("Nodal").set_value("true");
    }
  // Create association string
  std::string s = this->encodeModelEntityMask(def->associationMask());
  node.append_attribute("Associations").set_value(s.c_str());

  if (def->briefDescription() != "")
    {
    node.append_child("BriefDescription").text().set(def->briefDescription().c_str());
    }
  if (def->detailedDescription() != "")
    {
    node.append_child("DetailedDescription").text().set(def->detailedDescription().c_str());
    }
  // Now lets process its items
  std::size_t i, n = def->numberOfItemDefinitions();
  if (n != 0)
    {
    itemDefNodes = node.append_child("ItemDefinitions");
    for (i = 0; i < n; i++)
      {
      itemDefNode = itemDefNodes.append_child();
      itemDefNode.set_name(Item::type2String(def->itemDefinition(i)->type()).c_str());
      this->processItemDefinition(itemDefNode, 
                                  def->itemDefinition(i));
      }
    }
  // Now process all of its derived classes
  std::vector<slctk::AttributeDefinitionPtr> defs;
  this->m_manager.derivedDefinitions(def, defs);
  n = defs.size();
  for (i = 0; i < n; i++)
    {
    this->processDefinition(definitions, defs[i]);
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processItemDefinition(xml_node &node, 
                                              AttributeItemDefinitionPtr idef)
{
  xml_node child;
  node.append_attribute("Name").set_value(idef->name().c_str());
  node.append_attribute("DataType").set_value(Item::type2String(idef->type()).c_str());
  if (idef->label() != "")
    {
    node.append_attribute("Label").set_value(idef->label().c_str());
    }
  node.append_attribute("Version") = idef->version();
  if (idef->isOptional())
    {
    node.append_attribute("Optional").set_value("true");
    node.append_attribute("IsEnabledByDefault") = idef->isEnabledByDefault();
    }
  if (idef->advanceLevel() != 0)
    {
    node.append_attribute("AdvanceLevel") = idef->advanceLevel();
    }
  if (idef->numberOfCategories() && (idef->type() != Item::GROUP))
    {
    xml_node cnode, catNodes = node.append_child("Categories");
    std::set<std::string>::const_iterator it;
    const std::set<std::string> &cats = idef->categories();
    for (it = cats.begin(); it != cats.end(); it++)
      {
      catNodes.append_child("Cat").text().set(it->c_str());
      }
    }
  if (idef->briefDescription() != "")
    {
    node.append_child("BriefDescription").text().set(idef->briefDescription().c_str());
    }
  if (idef->detailedDescription() != "")
    {
    node.append_child("DetailedDescription").text().set(idef->detailedDescription().c_str());
    }
  switch (idef->type())
    {
    case Item::ATTRIBUTE_REF:
      this->processAttributeRefDef(node, slctk::dynamicCastPointer<AttributeRefItemDefinition>(idef));
      break;
    case Item::DOUBLE:
      this->processDoubleDef(node, slctk::dynamicCastPointer<DoubleItemDefinition>(idef));
      break;
    case Item::DIRECTORY:
      this->processDirectoryDef(node, slctk::dynamicCastPointer<DirectoryItemDefinition>(idef));
      break;
    case Item::FILE:
      this->processFileDef(node, slctk::dynamicCastPointer<FileItemDefinition>(idef));
      break;
    case Item::GROUP:
      this->processGroupDef(node, slctk::dynamicCastPointer<GroupItemDefinition>(idef));
      break;
    case Item::INT:
      this->processIntDef(node, slctk::dynamicCastPointer<IntItemDefinition>(idef));
      break;
    case Item::STRING:
      this->processStringDef(node, slctk::dynamicCastPointer<StringItemDefinition>(idef));
      break;
    case Item::VOID:
      // Nothing to do!
      break;
    default:
      std::cerr << "Unsupported Type!\n";
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDoubleDef(pugi::xml_node &node,
                                         DoubleItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, 
                        slctk::dynamicCastPointer<ValueItemDefinition>(idef));
  if (idef->isDiscrete())
    {
    xml_node dnodes = node.append_child("DiscreteInfo");
    int i, n = idef->numberOfDiscreteValues();
    xml_node dnode;
    for (i = 0; i < n; i++)
      {
      dnode = dnodes.append_child("Value");
      dnode.append_attribute("Enum").set_value(idef->discreteEnum(i).c_str());
      dnode.text().set(idef->discreteValue(i));
      }
    if (idef->hasDefault())
      {
      dnodes.append_attribute("DefaultIndex").set_value(idef->defaultDiscreteIndex());
      }
    return;
    }
  // Does this def have a default value
  if (idef->hasDefault())
    {
    xml_node defnode = node.append_child("DefaultValue");
    defnode.text().set(idef->defaultValue());
    }
  // Does this node have a range?
  if (idef->hasRange())
    {
    xml_node rnode = node.append_child("RangeInfo");
    xml_node r;
    if (idef->hasMinRange())
      {
      r = rnode.append_child("Min");
      if (idef->minRangeInclusive())
        {
        r.append_attribute("Inclusive").set_value(true);
        }
      r.text().set(idef->minRange());
      }
    if (idef->hasMaxRange())
      {
      r = rnode.append_child("Max");
      if (idef->maxRangeInclusive())
        {
        r.append_attribute("Inclusive").set_value(true);
        }
      r.text().set(idef->maxRange());
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processIntDef(pugi::xml_node &node,
                                      IntItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, 
                        slctk::dynamicCastPointer<ValueItemDefinition>(idef));
  if (idef->isDiscrete())
    {
    xml_node dnodes = node.append_child("DiscreteInfo");
    int i, n = idef->numberOfDiscreteValues();
    xml_node dnode;
    for (i = 0; i < n; i++)
      {
      dnode = dnodes.append_child("Value");
      dnode.append_attribute("Enum").set_value(idef->discreteEnum(i).c_str());
      dnode.text().set(idef->discreteValue(i));
      }
    if (idef->hasDefault())
      {
      dnodes.append_attribute("DefaultIndex").set_value(idef->defaultDiscreteIndex());
      }
    return;
    }
  // Does this def have a default value
  if (idef->hasDefault())
    {
    xml_node defnode = node.append_child("DefaultValue");
    defnode.text().set(idef->defaultValue());
    }
  // Does this node have a range?
  if (idef->hasRange())
    {
    xml_node rnode = node.append_child("RangeInfo");
    xml_node r;
    if (idef->hasMinRange())
      {
      r = rnode.append_child("Min");
      if (idef->minRangeInclusive())
        {
        r.append_attribute("Inclusive").set_value(true);
        }
      r.text().set(idef->minRange());
      }
    if (idef->hasMaxRange())
      {
      r = rnode.append_child("Max");
      if (idef->maxRangeInclusive())
        {
        r.append_attribute("Inclusive").set_value(true);
        }
      r.text().set(idef->maxRange());
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processStringDef(pugi::xml_node &node,
                                         StringItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, 
                        slctk::dynamicCastPointer<ValueItemDefinition>(idef));
  if (idef->isMultiline())
    {
    node.append_attribute("MultipleLines").set_value(true);
    }
  if (idef->isDiscrete())
    {
    xml_node dnodes = node.append_child("DiscreteInfo");
    int i, n = idef->numberOfDiscreteValues();
    xml_node dnode;
    for (i = 0; i < n; i++)
      {
      dnode = dnodes.append_child("Value");
      dnode.append_attribute("Enum").set_value(idef->discreteEnum(i).c_str());
      dnode.text().set(idef->discreteValue(i).c_str());
      }
    if (idef->hasDefault())
      {
      dnodes.append_attribute("DefaultIndex").set_value(idef->defaultDiscreteIndex());
      }
    return;
    }
  // Does this def have a default value
  if (idef->hasDefault())
    {
    xml_node defnode = node.append_child("DefaultValue");
    defnode.text().set(idef->defaultValue().c_str());
    }
  // Does this node have a range?
  if (idef->hasRange())
    {
    xml_node rnode = node.append_child("RangeInfo");
    xml_node r;
    if (idef->hasMinRange())
      {
      r = rnode.append_child("Min");
      if (idef->minRangeInclusive())
        {
        r.append_attribute("Inclusive").set_value(true);
        }
      r.text().set(idef->minRange().c_str());
      }
    if (idef->hasMaxRange())
      {
      r = rnode.append_child("Max");
      if (idef->maxRangeInclusive())
        {
        r.append_attribute("Inclusive").set_value(true);
        }
      r.text().set(idef->maxRange().c_str());
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processValueDef(pugi::xml_node &node,
                                        ValueItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfValues") = idef->numberOfValues();
  if (idef->hasValueLabels())
    {
    xml_node lnode = node.append_child();
    lnode.set_name("Labels");
    if (idef->usingCommonLabel())
      {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
      }
    else
      {
      int i, n = idef->numberOfValues();
      xml_node ln;
      for (i = 0; i < n; i++)
        {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
        }
      }
    }
  if (idef->allowsExpressions())
    {
    AttributeDefinitionPtr  exp = idef->expressionDefinition();
    if (exp != NULL)
      {
      xml_node enode = node.append_child("ExpressionType");
      enode.text().set(exp->type().c_str());
      }
    }
  if (idef->units() != "")
    {
    xml_node unode = node.append_child("Units");
    unode.text().set(idef->units().c_str());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processAttributeRefDef(pugi::xml_node &node,
                                               AttributeRefItemDefinitionPtr idef)
{
  AttributeDefinitionPtr  adp = idef->attributeDefinition();
  if (adp != NULL)
    {
    node.append_attribute("AttDef").set_value(adp->type().c_str());
    }
  node.append_attribute("NumberOfValues") = idef->numberOfValues();
  if (idef->hasValueLabels())
    {
    xml_node lnode = node.append_child();
    lnode.set_name("Labels");
    if (idef->usingCommonLabel())
      {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
      }
    else
      {
      int i, n = idef->numberOfValues();
      xml_node ln;
      for (i = 0; i < n; i++)
        {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
        }
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processDirectoryDef(pugi::xml_node &node,
                                            DirectoryItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfValues") = idef->numberOfValues();
  if (idef->shouldExist())
    {
    node.append_attribute("ShouldExist").set_value(true);
    }
  if (idef->shouldBeRelative())
    {
    node.append_attribute("ShouldBeRelative").set_value(true);
    }
  if (idef->hasValueLabels())
    {
    xml_node lnode = node.append_child();
    lnode.set_name("Labels");
    if (idef->usingCommonLabel())
      {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
      }
    else
      {
      int i, n = idef->numberOfValues();
      xml_node ln;
      for (i = 0; i < n; i++)
        {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
        }
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processFileDef(pugi::xml_node &node,
                                       FileItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfValues") = idef->numberOfValues();
  if (idef->shouldExist())
    {
    node.append_attribute("ShouldExist").set_value(true);
    }
  if (idef->shouldBeRelative())
    {
    node.append_attribute("ShouldBeRelative").set_value(true);
    }
  if (idef->hasValueLabels())
    {
    xml_node lnode = node.append_child();
    lnode.set_name("Labels");
    if (idef->usingCommonLabel())
      {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
      }
    else
      {
      int i, n = idef->numberOfValues();
      xml_node ln;
      for (i = 0; i < n; i++)
        {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
        }
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processGroupDef(pugi::xml_node &node,
                                        GroupItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfGroups") = idef->numberOfGroups();
  xml_node itemDefNode, itemDefNodes;
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
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processInstances()
{
  xml_node instances = this->m_root.append_child("Attribute");
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processSections()
{
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
void XmlV1StringWriter::processAttributeSection(xml_node &node,
                                            slctk::AttributeSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
  if (sec->modelEntityMask())
    {
    std::string s = this->encodeModelEntityMask(sec->modelEntityMask());
    node.append_attribute("ModelEnityFilter").set_value(s.c_str());
    }
  std::size_t i, n = sec->numberOfAttributeTypes();
  if (n)
    {
    xml_node atypes = node.append_child("Attribute Types");
    for (i = 0; i < n; i++)
      {
      atypes.append_child("Type").text().set(sec->attributeType(i).c_str());
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processInstancedSection(xml_node &node,
                                                  slctk::InstancedSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processModelEntitySection(xml_node &node,
                                                  slctk::ModelEntitySectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processSimpleExpressionSection(xml_node &node,
                                                       slctk::SimpleExpressionSectionPtr sec)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(sec));
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processGroupSection(xml_node &node,
                                            slctk::GroupSectionPtr group)
{
  this->processBasicSection(node,
                            slctk::dynamicCastPointer<Section>(group));
  std::size_t i, n = group->numberOfSubsections();
  xml_node child;
  slctk::SectionPtr sec;
  for (i = 0; i < n; i++)
    {
    child = node.append_child("SubSection");
    sec = group->subsection(i);
    switch(sec->type())
      {
      case Section::ATTRIBUTE:
        this->processAttributeSection(child, 
                                      slctk::dynamicCastPointer<AttributeSection>(sec));
        break;
      case Section::GROUP:
        this->processGroupSection(child, 
                                  slctk::dynamicCastPointer<GroupSection>(sec));
        break;
      case Section::INSTANCED:
        this->processInstancedSection(child, 
                                      slctk::dynamicCastPointer<InstancedSection>(sec));
        break;
      case Section::MODEL_ENTITY:
        this->processModelEntitySection(child, 
                                        slctk::dynamicCastPointer<ModelEntitySection>(sec));
        break;
      case Section::SIMPLE_EXPRESSION:
        this->processSimpleExpressionSection(child, 
                                             slctk::dynamicCastPointer<SimpleExpressionSection>(sec));
        break;
      default:
        std::cerr << "Unsupport Section Type " << Section::type2String(sec->type()) << std::endl;
      }
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processBasicSection(xml_node &node,
                                            slctk::SectionPtr sec)
{
  node.append_attribute("Title").set_value(sec->title().c_str());
  if (sec->iconName() != "")
    {
    node.append_attribute("Icon").set_value(sec->title().c_str());
    }
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::processModelInfo()
{
  xml_node modelInfo = this->m_root.append_child("ModelInfo");
}
//----------------------------------------------------------------------------
void XmlV1StringWriter::convertStringToXML(std::string &str)
{
  int i, n = str.size();
  for (i = 0; i < n; i++)
    {
    // See if we have any special XML characters to escape
    if (str[i] == '\'')
      {
      str.replace(i, 1, "&apos;");
      i+=5;
      n+=5;
      continue;
      }

    if (str[i] == '>')
      {
      str.replace(i, 1, "&gt;");
      i+=3;
      n+=3;
      continue;
      }
    
    if (str[i] == '<')
      {
      str.replace(i, 1, "&lt;");
      i+=3;
      n+=3;
      continue;
      }
    
    if (str[i] == '\"')
      {
      str.replace(i, 1, "&quot;");
      i+=5;
      n+=5;
      continue;
      }
    
    if (str[i] == '&')
      {
      str.replace(i, 1, "&amp;");
      i+=4;
      n+=4;
      continue;
      }
    }
}
//----------------------------------------------------------------------------
std::string XmlV1StringWriter::encodeModelEntityMask(unsigned long m)
{
  std::string s;
  if (m & 0x40)
    {
    s.append("d");
    }
  if (m & 0x20)
    {
    s.append("b");
    }
  if (m & 0x10)
    {
    s.append("m");
    }
  if (m & 0x8)
    {
    s.append("r");
    }
  if (m & 0x4)
    {
    s.append("f");
    }
  if (m & 0x2)
    {
    s.append("e");
    }
  if (m & 0x1)
    {
    s.append("v");
    }
  return s;
}
//----------------------------------------------------------------------------
