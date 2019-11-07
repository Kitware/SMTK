//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV2StringWriter.h"

#include "smtk/attribute/Attribute.h"
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
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/view/View.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/json/jsonHandleRange.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"
#include "smtk/model/StringData.h"

#include <sstream>

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;

// Some helper functions
namespace
{

int getValueForXMLElement(int v)
{
  return v;
}

double getValueForXMLElement(double v)
{
  return v;
}

const char* getValueForXMLElement(const std::string& v)
{
  return v.c_str();
}

template <typename T>
std::string getValueForXMLElement(const T& v, std::string& sep)
{
  return smtk::io::XmlV2StringWriter::concatenate(v, sep);
}

template <typename ItemDefType>
void processDerivedValueDef(pugi::xml_node& node, ItemDefType idef)
{
  if (idef->isDiscrete())
  {
    xml_node dnodes = node.append_child("DiscreteInfo");
    size_t j, i, nItems, n = idef->numberOfDiscreteValues();
    xml_node dnode, snode, inodes;
    std::string ename;
    std::vector<std::string> citems;
    for (i = 0; i < n; i++)
    {
      ename = idef->discreteEnum(i);
      // Lets see if there are any conditional items
      citems = idef->conditionalItems(ename);
      nItems = citems.size();
      if (nItems)
      {
        snode = dnodes.append_child("Structure");
        dnode = snode.append_child("Value");
        dnode.append_attribute("Enum").set_value(ename.c_str());
        dnode.text().set(getValueForXMLElement(idef->discreteValue(i)));
        inodes = snode.append_child("Items");
        for (j = 0; j < nItems; j++)
        {
          inodes.append_child("Item").text().set(citems[j].c_str());
        }
      }
      else
      {
        dnode = dnodes.append_child("Value");
        dnode.append_attribute("Enum").set_value(ename.c_str());
        dnode.text().set(getValueForXMLElement(idef->discreteValue(i)));
      }
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
    std::string sep; // TODO: The writer could accept a user-provided separator.
    defnode.text().set(getValueForXMLElement(idef->defaultValues(), sep).c_str());
    if (!sep.empty() && sep != ",")
      defnode.append_attribute("Sep").set_value(sep.c_str());
  }
  // Does this node have a range?
  if (idef->hasRange())
  {
    xml_node rnode = node.append_child("RangeInfo");
    xml_node r;
    bool inclusive;
    if (idef->hasMinRange())
    {
      r = rnode.append_child("Min");
      inclusive = idef->minRangeInclusive();
      r.append_attribute("Inclusive").set_value(inclusive);
      r.text().set(getValueForXMLElement(idef->minRange()));
    }
    if (idef->hasMaxRange())
    {
      r = rnode.append_child("Max");
      inclusive = idef->maxRangeInclusive();
      r.append_attribute("Inclusive").set_value(inclusive);
      r.text().set(getValueForXMLElement(idef->maxRange()));
    }
  }
}

template <typename ItemType>
void processDerivedValue(pugi::xml_node& node, ItemType item)
{
  if (item->isDiscrete())
  {
    return; // nothing left to do
  }
  size_t i, n = item->numberOfValues();
  if (!n)
  {
    return;
  }
  if ((item->numberOfRequiredValues() == 1) && !item->isExtensible())
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
        node.text().set(getValueForXMLElement(item->value()));
      }
    }
    else //This is an unset value
    {
      node.append_child("UnsetVal");
    }
    return;
  }
  xml_node val, values = node.append_child("Values");
  for (i = 0; i < n; i++)
  {
    if (item->isSet(i))
    {
      if (item->isExpression(i))
      {
        val = values.append_child("Expression");
        val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
        val.text().set(item->expression(i)->name().c_str());
      }
      else
      {
        val = values.append_child("Val");
        val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
        val.text().set(getValueForXMLElement(item->value(i)));
      }
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}
};

namespace smtk
{
namespace io
{

struct XmlV2StringWriter::Internals
{
  std::vector<xml_document*> m_docs;
  std::vector<xml_node> m_roots, m_defs, m_atts, m_views;
};

XmlV2StringWriter::XmlV2StringWriter(
  const attribute::ResourcePtr myResource, smtk::io::Logger& logger)
  : XmlStringWriter(myResource, logger)
{
  m_internals = new Internals();
}

XmlV2StringWriter::~XmlV2StringWriter()
{
  delete m_internals;
}

std::string XmlV2StringWriter::className() const
{
  return std::string("XmlV2StringWriter");
}

std::string XmlV2StringWriter::rootNodeName() const
{
  return std::string("SMTK_AttributeSystem");
}

unsigned int XmlV2StringWriter::fileVersion() const
{
  return 2;
}

std::string XmlV2StringWriter::convertToString(bool no_declaration)
{
  // Initialize the xml document(s)
  std::size_t i, num = 1;
  if (m_useDirectoryInfo)
  {
    num = m_resource->directoryInfo().size();
  }

  // Get things in the proper size
  m_internals->m_docs.resize(num);
  m_internals->m_roots.resize(num);
  m_internals->m_defs.resize(num);
  m_internals->m_atts.resize(num);
  m_internals->m_views.resize(num);

  std::stringstream oss;
  oss << "Created by " << this->className();
  for (i = 0; i < num; i++)
  {
    m_internals->m_docs.at(i) = new xml_document();
    xml_document& doc(*(m_internals->m_docs.at(i)));
    xml_node& root(m_internals->m_roots.at(i));
    doc.append_child(node_comment).set_value(oss.str().c_str());
    root = doc.append_child(this->rootNodeName().c_str());
    root.append_attribute("Version").set_value(this->fileVersion());
    // Are there includes to save?
    if (m_useDirectoryInfo)
    {
      auto incs = m_resource->directoryInfo().at(i).includeFiles();
      if (incs.size())
      {
        auto inodes = root.append_child("Includes");
        for (auto inc : incs)
        {
          inodes.append_child("File").text().set(inc.c_str());
        }
      }
    }
  }

  // Generate the element tree
  this->generateXml();

  // Serialize the result
  oss.clear();
  oss.str("");
  unsigned int flags = pugi::format_indent;
  if (no_declaration)
  {
    flags |= pugi::format_no_declaration;
  }
  m_internals->m_docs.at(0)->save(oss, "  ", flags);
  std::string result = oss.str();
  return result;
}

std::string XmlV2StringWriter::getString(std::size_t i, bool no_declaration)
{
  std::stringstream oss;
  unsigned int flags = pugi::format_indent;
  if (no_declaration)
  {
    flags |= pugi::format_no_declaration;
  }
  m_internals->m_docs.at(i)->save(oss, "  ", flags);
  std::string result = oss.str();
  return result;
}

void XmlV2StringWriter::generateXml()
{
  auto uuidName = m_resource->id().toString();
  xml_node& root(m_internals->m_roots.at(0));
  root.append_attribute("ID").set_value(uuidName.c_str());
  Analyses& analyses = m_resource->analyses();
  if (m_resource->numberOfCategories() || analyses.size())
  {
    root.append_child(node_comment)
      .set_value("**********  Category and Analysis Information ***********");
    if (m_resource->numberOfCategories())
    {
      auto catNodes = root.append_child("Categories");
      auto cats = m_resource->categories();
      for (auto cat : cats)
      {
        catNodes.append_child("Cat").text().set(cat.c_str());
      }
    }
  }
  if (analyses.size())
  {
    auto aNodes = root.append_child("Analyses");
    if (analyses.areTopLevelExclusive())
    {
      aNodes.append_attribute("Exclusive").set_value(true);
    }
    // We need to write the analyses so that an analysis's parent
    // is always saved before the analysis itself
    std::vector<smtk::attribute::Analyses::Analysis*> alist;
    auto topLevelAnalyses = analyses.topLevel();
    alist.insert(alist.end(), topLevelAnalyses.begin(), topLevelAnalyses.end());
    while (!alist.empty())
    {
      auto analysis = alist.back();
      alist.pop_back();
      auto anode = aNodes.append_child("Analysis");
      anode.append_attribute("Type").set_value(analysis->name().c_str());
      if (analysis->isExclusive())
      {
        anode.append_attribute("Exclusive").set_value(true);
      }
      if (analysis->isRequired())
      {
        anode.append_attribute("Required").set_value(true);
      }
      auto aCats = analysis->localCategories();
      for (auto acat : aCats)
      {
        anode.append_child("Cat").text().set(acat.c_str());
      }
      // Does the analysis have a parent?
      auto parent = analysis->parent();
      if (parent != nullptr)
      {
        anode.append_attribute("BaseType").set_value(parent->name().c_str());
      }
      if (analysis->hasLabel())
      {
        anode.append_attribute("Label").set_value(analysis->label().c_str());
      }
      // Add the analysis' children to be processed
      auto children = analysis->children();
      if (!children.empty())
      {
        alist.insert(alist.end(), children.begin(), children.end());
      }
    }
  }

  // Write out the advance levels information
  if (m_resource->numberOfAdvanceLevels())
  {
    xml_node cnode, catNodes = m_internals->m_roots.at(0).append_child("AdvanceLevels");
    std::map<int, std::string>::const_iterator it;
    const std::map<int, std::string>& levels = m_resource->advanceLevels();
    for (it = levels.begin(); it != levels.end(); it++)
    {
      xml_node anode = catNodes.append_child("Level");
      anode.append_attribute("Label").set_value(it->second.c_str());
      if (m_resource->advanceLevelColor(it->first))
      {
        anode.append_attribute("Color").set_value(
          this->encodeColor(m_resource->advanceLevelColor(it->first)).c_str());
      }
      anode.text().set(getValueForXMLElement(it->first));
    }
  }

  // Are we saving included files?
  auto num = m_internals->m_docs.size();
  for (std::size_t i = 1; i < num; i++)
  {
    auto cats = m_resource->directoryInfo().at(i).catagories();
    if (cats.size())
    {
      xml_node& catsRoot(m_internals->m_roots.at(i));
      catsRoot.append_child(node_comment).set_value("**********  Category Information ***********");
      auto catNodes = catsRoot.append_child("Categories");
      for (auto cat : cats)
      {
        catNodes.append_child("Cat").text().set(cat.c_str());
      }
    }
  }

  if (m_includeDefinitions || m_includeInstances)
  {
    this->processAttributeInformation();
  }
  if (m_includeViews)
  {
    this->processViews();
  }
}

void XmlV2StringWriter::processAttributeInformation()
{
  std::vector<smtk::attribute::DefinitionPtr> baseDefs;
  m_resource->findBaseDefinitions(baseDefs);
  std::size_t i, n = baseDefs.size();
  xml_node definitions, attributes;
  for (i = 0; i < n; i++)
  {
    this->processDefinition(baseDefs[i]);
  }
}

void XmlV2StringWriter::processDefinition(smtk::attribute::DefinitionPtr def)
{
  if (m_includeDefinitions)
  {
    std::size_t index = 0;
    if (m_useDirectoryInfo)
    {
      index = def->includeIndex();
    }
    // Is this node properly prepped?
    xml_node& defsNode = m_internals->m_defs.at(index);
    if (!defsNode)
    {
      m_internals->m_roots.at(index)
        .append_child(node_comment)
        .set_value("**********  Attribute Definitions ***********");
      defsNode = m_internals->m_roots.at(index).append_child("Definitions");
    }
    xml_node node = defsNode.append_child();
    node.set_name("AttDef");
    this->processDefinitionInternal(node, def);
  }

  if (m_includeInstances)
  {
    // Process all attributes based on this class
    std::vector<smtk::attribute::AttributePtr> atts;
    m_resource->findDefinitionAttributes(def->type(), atts);
    std::size_t n = atts.size();
    for (std::size_t i = 0; i < n; i++)
    {
      // Find the proper doc to include the attribute
      std::size_t index = 0;
      if (m_useDirectoryInfo)
      {
        index = atts.at(i)->includeIndex();
      }
      // Is this node properly prepped?
      xml_node& attsNode = m_internals->m_atts.at(index);
      if (!attsNode)
      {
        m_internals->m_roots.at(index)
          .append_child(node_comment)
          .set_value("**********  Attribute Instances ***********");
        attsNode = m_internals->m_roots.at(index).append_child("Attributes");
      }
      this->processAttribute(attsNode, atts[i]);
    }
  }
  // Now process all of its derived classes
  std::vector<smtk::attribute::DefinitionPtr> defs;
  m_resource->derivedDefinitions(def, defs);
  std::size_t n = defs.size();
  for (std::size_t i = 0; i < n; i++)
  {
    this->processDefinition(defs[i]);
  }
}

void XmlV2StringWriter::processDefinitionInternal(
  xml_node& definition, smtk::attribute::DefinitionPtr def)
{
  definition.append_attribute("Type").set_value(def->type().c_str());
  if (def->label() != "")
  {
    definition.append_attribute("Label").set_value(def->label().c_str());
  }
  if (def->baseDefinition())
  {
    definition.append_attribute("BaseType").set_value(def->baseDefinition()->type().c_str());
  }
  else
  {
    definition.append_attribute("BaseType").set_value("");
  }
  definition.append_attribute("Version") = def->version();
  if (def->isAbstract())
  {
    definition.append_attribute("Abstract").set_value("true");
  }
  if (def->advanceLevel())
  {
    definition.append_attribute("AdvanceLevel") = def->advanceLevel();
  }
  if (def->isUnique())
  { // false is the default
    definition.append_attribute("Unique").set_value("true");
  }
  else
  {
    definition.append_attribute("Unique").set_value("false");
  }
  if (def->rootName() != def->type())
  {
    definition.append_attribute("RootName").set_value(def->rootName().c_str());
  }
  if (def->isNodal())
  {
    definition.append_attribute("Nodal").set_value("true");
  }
  // Save Color Information
  std::string s;
  if (def->isNotApplicableColorSet())
  {
    s = this->encodeColor(def->notApplicableColor());
    definition.append_child("NotApplicableColor").text().set(s.c_str());
  }
  if (def->isDefaultColorSet())
  {
    s = this->encodeColor(def->defaultColor());
    definition.append_child("DefaultColor").text().set(s.c_str());
  }

  if (def->localAssociationRule())
  {
    // Create association element if we need to.
    xml_node assocDefNode = definition.append_child("AssociationsDef");
    auto assocRule = def->localAssociationRule();
    this->processItemDefinition(assocDefNode, assocRule);
  }

  if (def->briefDescription() != "")
  {
    definition.append_child("BriefDescription").text().set(def->briefDescription().c_str());
  }
  if (def->detailedDescription() != "")
  {
    definition.append_child("DetailedDescription").text().set(def->detailedDescription().c_str());
  }
  // Now lets process its items
  std::size_t n = def->numberOfItemDefinitions();
  // Does this definition have items not derived from its base def?
  if (n != def->itemOffset())
  {
    xml_node itemDefNodes = definition.append_child("ItemDefinitions");
    xml_node itemDefNode;
    for (std::size_t i = def->itemOffset(); i < n; i++)
    {
      itemDefNode = itemDefNodes.append_child();
      itemDefNode.set_name(
        Item::type2String(def->itemDefinition(static_cast<int>(i))->type()).c_str());
      this->processItemDefinition(itemDefNode, def->itemDefinition(static_cast<int>(i)));
    }
  }
}

void XmlV2StringWriter::processItemDefinition(
  xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
{
  this->processItemDefinitionAttributes(node, idef);
  this->processItemDefinitionType(node, idef);
}

void XmlV2StringWriter::processItemDefinitionAttributes(
  xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
{
  xml_node child;
  node.append_attribute("Name").set_value(idef->name().c_str());
  if (idef->label() != "")
  {
    node.append_attribute("Label").set_value(idef->label().c_str());
  }
  node.append_attribute("Version") = idef->version();
  node.append_attribute("OkToInheritCategories") = idef->isOkToInherit();
  if (idef->isOptional())
  {
    node.append_attribute("Optional").set_value("true");
    node.append_attribute("IsEnabledByDefault") = idef->isEnabledByDefault();
  }
  if (idef->categoryCheckMode() == smtk::attribute::ItemDefinition::CategoryCheckMode::All)
  {
    node.append_attribute("CategoryCheckMode").set_value("All");
  }
  else
  {
    node.append_attribute("CategoryCheckMode").set_value("Any");
  }
  if (idef->advanceLevel(0) || idef->advanceLevel(1))
  {
    // OK - we have a non-zero advance level in either read or write
    // if they are both set the same use the AdvanceLevel xml attribute
    if (idef->advanceLevel(0) == idef->advanceLevel(1))
    {
      node.append_attribute("AdvanceLevel") = idef->advanceLevel(0);
    }
    else
    {
      if (idef->advanceLevel(0))
      {
        node.append_attribute("AdvanceReadLevel") = idef->advanceLevel(0);
      }
      if (idef->advanceLevel(1))
      {
        node.append_attribute("AdvanceWriteLevel") = idef->advanceLevel(1);
      }
    }
  }
  if (!idef->localCategories().empty())
  {
    xml_node cnode, catNodes = node.append_child("Categories");
    for (auto& str : idef->localCategories())
    {
      catNodes.append_child("Cat").text().set(str.c_str());
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
}

void XmlV2StringWriter::processItemDefinitionType(
  xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
{
  switch (idef->type())
  {
    case Item::DoubleType:
      this->processDoubleDef(node, smtk::dynamic_pointer_cast<DoubleItemDefinition>(idef));
      break;
    case Item::DirectoryType:
      this->processDirectoryDef(node, smtk::dynamic_pointer_cast<DirectoryItemDefinition>(idef));
      break;
    case Item::FileType:
      this->processFileDef(node, smtk::dynamic_pointer_cast<FileItemDefinition>(idef));
      break;
    case Item::GroupType:
      this->processGroupDef(node, smtk::dynamic_pointer_cast<GroupItemDefinition>(idef));
      break;
    case Item::IntType:
      this->processIntDef(node, smtk::dynamic_pointer_cast<IntItemDefinition>(idef));
      break;
    case Item::StringType:
      this->processStringDef(node, smtk::dynamic_pointer_cast<StringItemDefinition>(idef));
      break;
    case Item::ModelEntityType:
      this->processModelEntityDef(
        node, smtk::dynamic_pointer_cast<ModelEntityItemDefinition>(idef));
      break;
    case Item::VoidType:
      // Nothing to do!
      break;
      break;
    default:
      smtkErrorMacro(m_logger, "Unsupported Type: " << Item::type2String(idef->type())
                                                    << " for Item Definition: " << idef->name());
  }
}

void XmlV2StringWriter::processDoubleDef(
  pugi::xml_node& node, attribute::DoubleItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, dynamic_pointer_cast<ValueItemDefinition>(idef));
  processDerivedValueDef<attribute::DoubleItemDefinitionPtr>(node, idef);
}

void XmlV2StringWriter::processIntDef(pugi::xml_node& node, attribute::IntItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, smtk::dynamic_pointer_cast<ValueItemDefinition>(idef));
  processDerivedValueDef<attribute::IntItemDefinitionPtr>(node, idef);
}

void XmlV2StringWriter::processStringDef(
  pugi::xml_node& node, attribute::StringItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node, smtk::dynamic_pointer_cast<ValueItemDefinition>(idef));
  if (idef->isMultiline())
  {
    node.append_attribute("MultipleLines").set_value(true);
  }
  if (idef->isSecure())
  {
    node.append_attribute("Secure").set_value("true");
  }
  processDerivedValueDef<attribute::StringItemDefinitionPtr>(node, idef);
}

void XmlV2StringWriter::processModelEntityDef(
  pugi::xml_node& node, attribute::ModelEntityItemDefinitionPtr idef)
{
  smtk::model::BitFlags membershipMask = idef->membershipMask();
  std::string membershipMaskStr = this->encodeModelEntityMask(membershipMask);
  xml_node menode;
  menode = node.append_child("MembershipMask");
  menode.text().set(membershipMaskStr.c_str());

  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
  {
    node.append_attribute("Extensible") = true;

    if (idef->maxNumberOfValues())
      node.append_attribute("MaxNumberOfValues") =
        static_cast<unsigned int>(idef->maxNumberOfValues());
  }

  if (idef->hasValueLabels())
  {
    xml_node lnode = node.append_child();
    lnode.set_name("ComponentLabels");
    if (idef->usingCommonLabel())
    {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
    }
    else
    {
      size_t i, n = idef->numberOfRequiredValues();
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

void XmlV2StringWriter::processValueDef(
  pugi::xml_node& node, smtk::attribute::ValueItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
  {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfValues())
    {
      node.append_attribute("MaxNumberOfValues") =
        static_cast<unsigned int>(idef->maxNumberOfValues());
    }
  }
  if (idef->hasValueLabels())
  {
    xml_node lnode = node.append_child();
    lnode.set_name("ComponentLabels");
    if (idef->usingCommonLabel())
    {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
    }
    else
    {
      size_t i, n = idef->numberOfRequiredValues();
      xml_node ln;
      for (i = 0; i < n; i++)
      {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.text().set(idef->valueLabel(i).c_str());
      }
    }
  }
  if (idef->allowsExpressions())
  {
    attribute::DefinitionPtr exp = idef->expressionDefinition(m_resource);
    if (exp)
    {
      xml_node enode = node.append_child("ExpressionType");
      enode.text().set(exp->type().c_str());
    }
  }
  if (idef->units() != "")
  {
    node.append_attribute("Units") = idef->units().c_str();
  }
  // Now lets process its children items
  if (!idef->numberOfChildrenItemDefinitions())
  {
    return;
  }
  xml_node itemDefNode, itemDefNodes = node.append_child("ChildrenDefinitions");
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator iter;
  for (iter = idef->childrenItemDefinitions().begin();
       iter != idef->childrenItemDefinitions().end(); ++iter)
  {
    itemDefNode = itemDefNodes.append_child();
    itemDefNode.set_name(Item::type2String(iter->second->type()).c_str());
    this->processItemDefinition(itemDefNode, iter->second);
  }
}

void XmlV2StringWriter::processDirectoryDef(
  pugi::xml_node& node, attribute::DirectoryItemDefinitionPtr idef)
{
  this->processFileSystemDef(node, idef);
}

void XmlV2StringWriter::processFileDef(pugi::xml_node& node, attribute::FileItemDefinitionPtr idef)
{
  this->processFileSystemDef(node, idef);
  std::string fileFilters = idef->getFileFilters();
  if (fileFilters != "")
  {
    node.append_attribute("FileFilters") = fileFilters.c_str();
  }
}

void XmlV2StringWriter::processFileSystemDef(
  pugi::xml_node& node, attribute::FileSystemItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
  {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfValues())
    {
      node.append_attribute("MaxNumberOfValues") =
        static_cast<unsigned int>(idef->maxNumberOfValues());
    }
  }
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
    lnode.set_name("ComponentLabels");
    if (idef->usingCommonLabel())
    {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
    }
    else
    {
      size_t i, n = idef->numberOfRequiredValues();
      xml_node ln;
      for (i = 0; i < n; i++)
      {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
      }
    }
  }
  if (idef->hasDefault())
  {
    xml_node defaultNode = node.append_child();
    defaultNode.set_name("DefaultValue");
    defaultNode.text().set(idef->defaultValue().c_str());
  }
}

void XmlV2StringWriter::processGroupDef(
  pugi::xml_node& node, attribute::GroupItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredGroups") =
    static_cast<unsigned int>(idef->numberOfRequiredGroups());
  if (idef->isExtensible())
  {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfGroups())
    {
      node.append_attribute("MaxNumberOfGroups") =
        static_cast<unsigned int>(idef->maxNumberOfGroups());
    }
  }

  xml_node itemDefNode, itemDefNodes;
  if (idef->hasSubGroupLabels())
  {
    xml_node lnode = node.append_child();
    lnode.set_name("ComponentLabels");
    if (idef->usingCommonSubGroupLabel())
    {
      lnode.append_attribute("CommonLabel") = idef->subGroupLabel(0).c_str();
    }
    else
    {
      size_t i, n = idef->numberOfRequiredGroups();
      xml_node ln;
      for (i = 0; i < n; i++)
      {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->subGroupLabel(i).c_str());
      }
    }
  }
  // Now lets process its items
  int i, n = static_cast<int>(idef->numberOfItemDefinitions());
  if (n != 0)
  {
    itemDefNodes = node.append_child("ItemDefinitions");
    for (i = 0; i < n; i++)
    {
      itemDefNode = itemDefNodes.append_child();
      itemDefNode.set_name(Item::type2String(idef->itemDefinition(i)->type()).c_str());
      this->processItemDefinition(itemDefNode, idef->itemDefinition(i));
    }
  }
}

void XmlV2StringWriter::processAttribute(xml_node& attributes, attribute::AttributePtr att)
{
  xml_node node = attributes.append_child("Att");
  node.append_attribute("Name").set_value(att->name().c_str());
  if (att->definition())
  {
    node.append_attribute("Type").set_value(att->definition()->type().c_str());
    if (att->definition()->isNodal())
    {
      node.append_attribute("OnInteriorNodes").set_value(att->appliesToInteriorNodes());
      node.append_attribute("OnBoundaryNodes").set_value(att->appliesToBoundaryNodes());
    }
  }
  node.append_attribute("ID").set_value(att->id().toString().c_str());
  // Save associated entities
  auto assoc = att->associations();
  if (assoc && assoc->numberOfValues() > 0)
  {
    xml_node assocNode = node.append_child("Associations");
    this->processItem(assocNode, assoc);
  }
  // Save Color Information
  if (att->isColorSet())
  {
    std::string s;
    s = this->encodeColor(att->color());
    node.append_child("Color").text().set(s.c_str());
  }
  int i, n = static_cast<int>(att->numberOfItems());
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
}

void XmlV2StringWriter::processItem(xml_node& node, smtk::attribute::ItemPtr item)
{
  this->processItemAttributes(node, item);
  this->processItemType(node, item);
}

void XmlV2StringWriter::processItemAttributes(xml_node& node, smtk::attribute::ItemPtr item)
{
  node.append_attribute("Name").set_value(item->name().c_str());
  if (item->isOptional())
  {
    node.append_attribute("Enabled").set_value(item->isEnabled());
  }

  // Does the item have explicit advance level information
  if (!item->usingDefinitionAdvanceLevel(0))
  {
    node.append_attribute("AdvanceReadLevel") = item->advanceLevel(0);
  }

  if (!item->usingDefinitionAdvanceLevel(1))
  {
    node.append_attribute("AdvanceWriteLevel") = item->advanceLevel(1);
  }
}

void XmlV2StringWriter::processItemType(xml_node& node, smtk::attribute::ItemPtr item)
{
  switch (item->type())
  {
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
      this->processModelEntityItem(node, smtk::dynamic_pointer_cast<ModelEntityItem>(item));
      break;
    case Item::VoidType:
      // Nothing to do!
      break;
    default:
      smtkErrorMacro(m_logger, "Unsupported Type: " << Item::type2String(item->type())
                                                    << " for Item: " << item->name());
  }
}

void XmlV2StringWriter::processValueItem(pugi::xml_node& node, attribute::ValueItemPtr item)
{
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  size_t i, n = item->numberOfValues();

  // If the item can have variable number of values then store how many
  // values it has
  if (item->isExtensible())
  {
    node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
  }

  if (!n)
  {
    return; // nothing else to be done
  }

  if (!item->isDiscrete())
  {
    return; // there is nothing else to be done
  }

  if (item->numberOfChildrenItems())
  {
    xml_node childNode, childNodes = node.append_child("ChildrenItems");
    std::map<std::string, smtk::attribute::ItemPtr>::const_iterator iter;
    const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems = item->childrenItems();
    for (iter = childrenItems.begin(); iter != childrenItems.end(); iter++)
    {
      childNode = childNodes.append_child();
      childNode.set_name(Item::type2String(iter->second->type()).c_str());
      this->processItem(childNode, iter->second);
    }
  }

  xml_node val, values;
  if ((numRequiredVals == 1) && !item->isExtensible()) // Special Common Case
  {
    node.append_attribute("Discrete").set_value(true);
    if (item->isSet())
    {
      node.text().set(item->discreteIndex());
    }
    return;
  }
  values = node.append_child("DiscreteValues");
  for (i = 0; i < n; i++)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Index");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      val.text().set(item->discreteIndex(i));
    }
    else
    {
      val = values.append_child("UnsetDiscreteVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV2StringWriter::processDoubleItem(pugi::xml_node& node, attribute::DoubleItemPtr item)
{
  this->processValueItem(node, dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::DoubleItemPtr>(node, item);
}

void XmlV2StringWriter::processIntItem(pugi::xml_node& node, attribute::IntItemPtr item)
{
  this->processValueItem(node, dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::IntItemPtr>(node, item);
}

void XmlV2StringWriter::processStringItem(pugi::xml_node& node, attribute::StringItemPtr item)
{
  this->processValueItem(node, dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::StringItemPtr>(node, item);
}

void XmlV2StringWriter::processModelEntityItem(
  pugi::xml_node& node, attribute::ModelEntityItemPtr item)
{
  size_t i = 0, n = item->numberOfValues();
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  // we should always have "NumberOfValues" set
  node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));

  xml_node val;
  if (!n)
  {
    return;
  }

  if ((numRequiredVals == 1) && (!item->isExtensible()))
  {
    if (item->isSet())
    {
      val = node.append_child("Val");
      val.text().set(item->value(i).entity().toString().c_str());
    }
    return;
  }
  xml_node values = node.append_child("Values");
  for (i = 0; i < n; i++)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      val.text().set(item->value(i).entity().toString().c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV2StringWriter::processDirectoryItem(pugi::xml_node& node, attribute::DirectoryItemPtr item)
{
  this->processFileSystemItem(node, item);
}

void XmlV2StringWriter::processFileItem(pugi::xml_node& node, attribute::FileItemPtr item)
{
  // always write out all recentValues
  if (item->recentValues().size() > 0)
  {
    xml_node recval, recvalues = node.append_child("RecentValues");
    std::vector<std::string>::const_iterator it;
    for (it = item->recentValues().begin(); it != item->recentValues().end(); ++it)
    {
      recval = recvalues.append_child("Val");
      recval.text().set((*it).c_str());
    }
  }

  this->processFileSystemItem(node, item);
}

void XmlV2StringWriter::processFileSystemItem(
  pugi::xml_node& node, attribute::FileSystemItemPtr item)
{
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  size_t i, n = item->numberOfValues();
  if (!n)
  {
    return;
  }

  // If the item can have variable number of values then store how many
  // values it has
  if (item->isExtensible())
  {
    node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
  }

  if (numRequiredVals == 1 && !item->isExtensible()) // Special Common Case
  {
    if (item->isSet())
    {
      node.text().set(item->value().c_str());
    }
    return;
  }
  xml_node val, values = node.append_child("Values");
  for (i = 0; i < n; i++)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      val.text().set(item->value(i).c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV2StringWriter::processGroupItem(pugi::xml_node& node, attribute::GroupItemPtr item)
{
  size_t i, j, m, n;
  std::size_t numRequiredGroups = item->numberOfRequiredGroups();
  xml_node itemNode;
  n = item->numberOfGroups();
  m = item->numberOfItemsPerGroup();
  if (!n)
  {
    return;
  }

  // If the group can have variable number of subgroups then store how many
  //  it has
  if (item->isExtensible())
  {
    node.append_attribute("NumberOfGroups").set_value(static_cast<unsigned int>(n));
  }

  // Optimize for number of required groups = 1
  else if (numRequiredGroups == 1)
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
  for (i = 0; i < n; i++)
  {
    cluster = clusters.append_child("Cluster");
    cluster.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    for (j = 0; j < m; j++)
    {
      itemNode = cluster.append_child();
      itemNode.set_name(Item::type2String(item->item(i, j)->type()).c_str());
      this->processItem(itemNode, item->item(i, j));
    }
  }
}

void XmlV2StringWriter::processViews()
{
  // First write toplevel views and then write out the non-toplevel - note that the
  // attribute or view collection do not care about this - the assumption is that the designer would
  // probably like all the toplevel views clustered together
  std::map<std::string, smtk::view::ViewPtr>::const_iterator iter;
  bool isTop;
  for (iter = m_resource->views().begin(); iter != m_resource->views().end(); iter++)
  {
    if (iter->second->details().attributeAsBool("TopLevel", isTop) && isTop)
    {
      this->processView(iter->second);
    }
  }
  for (iter = m_resource->views().begin(); iter != m_resource->views().end(); iter++)
  {
    if (!(iter->second->details().attributeAsBool("TopLevel", isTop) && isTop))
    {
      this->processView(iter->second);
    }
  }
}

void XmlV2StringWriter::processView(smtk::view::ViewPtr view)
{
  std::size_t index = 0;
  // Are we using Directory Info?
  if (m_useDirectoryInfo)
  {
    index = view->includeIndex();
  }

  xml_node& viewsNode = m_internals->m_views.at(index);
  // Is the Views node properly prepped?
  if (!viewsNode)
  {
    m_internals->m_roots.at(index)
      .append_child(node_comment)
      .set_value("********** Workflow Views ***********");
    viewsNode = m_internals->m_roots.at(index).append_child("Views");
  }
  xml_node node;
  node = viewsNode.append_child("View");
  node.append_attribute("Type").set_value(view->type().c_str());
  node.append_attribute("Name").set_value(view->name().c_str());
  if (view->iconName() != "")
  {
    node.append_attribute("Icon").set_value(view->iconName().c_str());
  }
  this->processViewComponent(view->details(), node);
}

void XmlV2StringWriter::processViewComponent(smtk::view::View::Component& comp, xml_node& node)
{
  // Add the attributes of the component to the node
  std::map<std::string, std::string>::const_iterator iter;
  for (iter = comp.attributes().begin(); iter != comp.attributes().end(); iter++)
  {
    node.append_attribute(iter->first.c_str()).set_value(iter->second.c_str());
  }
  // if the comp has contents then save it in the node's text
  // else process the comp's children
  if (comp.contents() != "")
  {
    node.text().set(comp.contents().c_str());
  }
  else
  {
    xml_node child;
    std::size_t i, n = comp.numberOfChildren();
    for (i = 0; i < n; i++)
    {
      child = node.append_child(comp.child(i).name().c_str());
      this->processViewComponent(comp.child(i), child);
    }
  }
}

std::string XmlV2StringWriter::encodeColor(const double* c)
{
  std::stringstream oss;
  oss << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3];
  std::string result = oss.str();
  return result;
}

std::string XmlV2StringWriter::encodeModelEntityMask(smtk::model::BitFlags f)
{
  return smtk::model::Entity::flagToSpecifierString(f);
}

pugi::xml_node& XmlV2StringWriter::topDefinitionsNode() const
{
  return m_internals->m_defs[0];
}
pugi::xml_node& XmlV2StringWriter::topRootNode() const
{
  return m_internals->m_roots[0];
}
pugi::xml_node& XmlV2StringWriter::topAttributesNode() const
{
  return m_internals->m_atts[0];
}
pugi::xml_node& XmlV2StringWriter::topViewsNode() const
{
  return m_internals->m_views[0];
}

} // namespace io
} // namespace smtk
