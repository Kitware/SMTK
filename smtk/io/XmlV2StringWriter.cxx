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

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/common/View.h"
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

#include "smtk/model/EntityRef.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/StringData.h"
#include "smtk/mesh/Collection.h"

#include <sstream>
#include "cJSON.h"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;

// Some helper functions
namespace {

  int getValueForXMLElement(int v)
  {
    return v;
  }

//----------------------------------------------------------------------------
  double getValueForXMLElement(double v)
  {
    return v;
  }

//----------------------------------------------------------------------------
  const char *getValueForXMLElement(const std::string& v)
  {
    return v.c_str();
  }

//----------------------------------------------------------------------------
  template<typename T>
  std::string getValueForXMLElement(const T& v, std::string& sep)
  {
    std::ostringstream token;
    if (v.empty())
      return token.str();

    typename T::const_iterator it;
    std::set<char> notSep;
    if (sep.empty())
      {
      // Find all the characters that cannot serve as a separator
      // Note that we need to do this even when v.size() == 1
      // because some locales might use our preferred separator
      // in a non-separator-role (e.g., "3.14" is written "3,14"
      // in some locales), so we must ensure that the single value
      // in v does not contain the default separator.
      for (it = v.begin(); it != v.end(); ++it)
        {
        token.precision(17);
        token << *it;
        std::string::const_iterator sit;
        std::string entry = token.str();
        for (sit = entry.begin(); sit != entry.end(); ++sit)
          notSep.insert(*sit);
        token.str(std::string());
        token.clear();
        }
      // Try some preferred separators in order of preference.
      static const char preferredSeps[] = ",;|:#@!.-=_`?+/\\";
      int preferredSepsLen = sizeof(preferredSeps) / sizeof(preferredSeps[0]);
      char finalSep = '\0';
      for (int i = 0; i < preferredSepsLen; ++i)
        if (notSep.find(preferredSeps[i]) == notSep.end())
          {
          finalSep = preferredSeps[i];
          break;
          }
      // OK, desperately try any character at all.
      if (!finalSep)
        for (int i = 1; i < 255; ++i)
          if (notSep.find(preferredSeps[i]) == notSep.end())
            {
            finalSep = preferredSeps[i];
            break;
            }
      if (!finalSep)
        {
        std::cerr << "Tokens use every single possible character; no separator found. Using comma.\n";
        finalSep = ',';
        }
      // Return whatever separator we came up with to the caller:
      sep = finalSep;
      }
    // Now accumulate values into an output string.
    token.clear();
    token.precision(17);
    it = v.begin();
    token << *it;
    ++it;
    for (; it != v.end(); ++it)
      {
      token.precision(17);
      token << sep << *it;
      }

    return token.str();
  }

//----------------------------------------------------------------------------
  template<typename ItemDefType>
  void processDerivedValueDef(pugi::xml_node &node,  ItemDefType idef)
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
//----------------------------------------------------------------------------
  template<typename ItemType>
  void processDerivedValue(pugi::xml_node &node,  ItemType item)
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
    for(i = 0; i < n; i++)
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

namespace smtk {
  namespace io {

struct XmlV2StringWriter::PugiPrivate
{
  xml_node& root;
  PugiPrivate(xml_node& parent_node): root(parent_node) {}
};

//----------------------------------------------------------------------------
XmlV2StringWriter::XmlV2StringWriter(const attribute::System &mySystem):
m_system(mySystem), m_includeDefinitions(true), m_includeInstances(true),
m_includeModelInformation(true), m_includeViews(true), m_pugi(0)
{
}

//----------------------------------------------------------------------------
XmlV2StringWriter::~XmlV2StringWriter()
{
  delete m_pugi;
}
//----------------------------------------------------------------------------
std::string XmlV2StringWriter::convertToString(Logger &logger,
                                               bool no_declaration)
{
  // Initialize the xml document
  xml_document doc;
  doc.append_child(node_comment).set_value("Created by XmlV2StringWriter");
  xml_node root = doc.append_child("SMTK_AttributeSystem");
  root.append_attribute("Version").set_value(2);

  // Generate the element tree
  this->generateXml(root, logger, false);

  // Serialize the result
  std::stringstream oss;
  unsigned int flags = pugi::format_indent;
  if (no_declaration)
    {
    flags |= pugi::format_no_declaration;
    }
  doc.save(oss, "  ", flags);
  std::string result = oss.str();
  return result;
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::generateXml(pugi::xml_node& parent_node,
                                    Logger& logger,
                                    bool createRoot)
{
  // Reset the message log
  this->m_logger.reset();

  xml_node root;
  if (createRoot)
    {
    // This option is used to insert an attribute system
    // into an existing xml document (for writing resource files).
    root = parent_node.append_child("SMTK_AttributeSystem");
    root.append_attribute("Version").set_value(2);
    m_pugi = new PugiPrivate(root);
    }
  else
    {
    // This option is used when writing a single attribute system,
    // and the root node has already been created by the caller.
    m_pugi = new PugiPrivate(parent_node);
    }

  this->m_pugi->root.append_child(node_comment)
    .set_value("**********  Category and Analysis Information ***********");

  // Write out the category and analysis information
  if (this->m_system.numberOfCategories())
    {
    xml_node cnode, catNodes = this->m_pugi->root.append_child("Categories");
    std::set<std::string>::const_iterator it;
    const std::set<std::string> &cats = this->m_system.categories();
    for (it = cats.begin(); it != cats.end(); it++)
      {
      catNodes.append_child("Cat").text().set(it->c_str());
      }
    }

  if (this->m_system.numberOfAnalyses())
    {
    xml_node cnode, catNodes = this->m_pugi->root.append_child("Analyses");
    std::map<std::string, std::set<std::string> >::const_iterator it;
    const std::map<std::string, std::set<std::string> > &analyses =
      this->m_system.analyses();
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

  // Write out the advance levels information
  if (this->m_system.numberOfAdvanceLevels())
    {
    xml_node cnode, catNodes = this->m_pugi->root.append_child("AdvanceLevels");
    std::map<int, std::string>::const_iterator it;
    const std::map<int, std::string> &levels = this->m_system.advanceLevels();
    for (it = levels.begin(); it != levels.end(); it++)
      {
      xml_node anode = catNodes.append_child("Level");
      anode.append_attribute("Label").set_value(it->second.c_str());
      if(this->m_system.advanceLevelColor(it->first))
        {
        anode.append_attribute("Color").set_value(
          this->encodeColor(this->m_system.advanceLevelColor(it->first)).c_str());
        }
      anode.text().set(getValueForXMLElement(it->first));
      }
    }

  if (this->m_includeDefinitions || this->m_includeInstances)
    {
    this->processAttributeInformation();
    }
  if (this->m_includeViews)
    {
    this->processViews();
    }
  if (this->m_includeModelInformation)
    {
    this->processModelInfo();
    }
  logger = this->m_logger;
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processAttributeInformation()
{
  std::vector<smtk::attribute::DefinitionPtr> baseDefs;
  this->m_system.findBaseDefinitions(baseDefs);
  std::size_t i, n = baseDefs.size();
  xml_node definitions, attributes;

  if (this->m_includeDefinitions)
    {
    this->m_pugi->root.append_child(node_comment).set_value("**********  Attribute Definitions ***********");
    definitions = this->m_pugi->root.append_child("Definitions");
    }
  if (this->m_includeInstances)
    {
    this->m_pugi->root.append_child(node_comment).set_value("**********  Attribute Instances ***********");
    attributes = this->m_pugi->root.append_child("Attributes");
    }
  for (i = 0; i < n; i++)
    {
    this->processDefinition(definitions, attributes, baseDefs[i]);
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processDefinition(xml_node &definitions,
                                          xml_node &attributes,
                                          smtk::attribute::DefinitionPtr def)
{
  std::size_t i, n;

  if (this->m_includeDefinitions)
    {
    xml_node itemDefNode, itemDefNodes,
      child, node = definitions.append_child();

    node.set_name("AttDef");
    node.append_attribute("Type").set_value(def->type().c_str());
    if (def->label() != "")
      {
      node.append_attribute("Label").set_value(def->label().c_str());
      }
    if (def->baseDefinition())
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
      { // true is the default
      node.append_attribute("Unique").set_value("true");
      }
    else
      {
      node.append_attribute("Unique").set_value("false");
      }
    if (def->rootName() != def->type())
      {
      node.append_attribute("RootName").set_value(def->rootName().c_str());
      }
    if (def->isNodal())
      {
      node.append_attribute("Nodal").set_value("true");
      }
    // Save Color Information
    std::string s;
    if (def->isNotApplicableColorSet())
      {
      s = this->encodeColor(def->notApplicableColor());
      node.append_child("NotApplicableColor").text().set(s.c_str());
      }
    if (def->isDefaultColorSet())
      {
      s = this->encodeColor(def->defaultColor());
      node.append_child("DefaultColor").text().set(s.c_str());
      }

    if (def->associationMask())
      {
      // Create association element if we need to.
      xml_node assocDefNode = node.append_child("AssociationsDef");
      ModelEntityItemDefinitionPtr assocRule = def->associationRule();
      this->processItemDefinition(assocDefNode, assocRule);
      }

    if (def->briefDescription() != "")
      {
      node.append_child("BriefDescription").text().set(def->briefDescription().c_str());
      }
    if (def->detailedDescription() != "")
      {
      node.append_child("DetailedDescription").text().set(def->detailedDescription().c_str());
      }
    // Now lets process its items
    n = def->numberOfItemDefinitions();
    // Does this definition have items not derived from its base def?
    if (n != def->itemOffset())
      {
      itemDefNodes = node.append_child("ItemDefinitions");
      for (i = def->itemOffset(); i < n; i++)
        {
        itemDefNode = itemDefNodes.append_child();
        itemDefNode.set_name(Item::type2String(def->itemDefinition(static_cast<int>(i))->type()).c_str());
        this->processItemDefinition(itemDefNode,
                                    def->itemDefinition(static_cast<int>(i)));
        }
      }
    }
  if (this->m_includeInstances)
    {
    // Process all attributes based on this class
    std::vector<smtk::attribute::AttributePtr> atts;
    this->m_system.findDefinitionAttributes(def->type(), atts);
    n = atts.size();
    for (i = 0; i < n; i++)
      {
      this->processAttribute(attributes, atts[i]);
      }
    }
  // Now process all of its derived classes
  std::vector<smtk::attribute::DefinitionPtr> defs;
  this->m_system.derivedDefinitions(def, defs);
  n = defs.size();
  for (i = 0; i < n; i++)
    {
    this->processDefinition(definitions, attributes, defs[i]);
    }
}
//----------------------------------------------------------------------------
void
XmlV2StringWriter::processItemDefinition(xml_node &node,
                                         smtk::attribute::ItemDefinitionPtr idef)
{
  xml_node child;
  node.append_attribute("Name").set_value(idef->name().c_str());
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
      this->processRefDef(node, smtk::dynamic_pointer_cast<RefItemDefinition>(idef));
      break;
    case Item::DOUBLE:
      this->processDoubleDef(node, smtk::dynamic_pointer_cast<DoubleItemDefinition>(idef));
      break;
    case Item::DIRECTORY:
      this->processDirectoryDef(node, smtk::dynamic_pointer_cast<DirectoryItemDefinition>(idef));
      break;
    case Item::FILE:
      this->processFileDef(node, smtk::dynamic_pointer_cast<FileItemDefinition>(idef));
      break;
    case Item::GROUP:
      this->processGroupDef(node, smtk::dynamic_pointer_cast<GroupItemDefinition>(idef));
      break;
    case Item::INT:
      this->processIntDef(node, smtk::dynamic_pointer_cast<IntItemDefinition>(idef));
      break;
    case Item::STRING:
      this->processStringDef(node, smtk::dynamic_pointer_cast<StringItemDefinition>(idef));
      break;
    case Item::MODEL_ENTITY:
      this->processModelEntityDef(node, smtk::dynamic_pointer_cast<ModelEntityItemDefinition>(idef));
      break;
    case Item::MESH_SELECTION:
      this->processMeshSelectionItemDef(node, smtk::dynamic_pointer_cast<MeshSelectionItemDefinition>(idef));
      break;
    case Item::MESH_ENTITY:
      this->processMeshEntityDef(node, smtk::dynamic_pointer_cast<MeshItemDefinition>(idef));
      break;
    case Item::VOID:
      // Nothing to do!
      break;
      break;
    default:
      smtkErrorMacro(this->m_logger,
                     "Unsupported Type: " << Item::type2String(idef->type())
                     << " for Item Definition: " << idef->name());
    }
}

//----------------------------------------------------------------------------
void XmlV2StringWriter::processDoubleDef(pugi::xml_node &node,
                                         attribute::DoubleItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node,
                        dynamic_pointer_cast<ValueItemDefinition>(idef));
  processDerivedValueDef<attribute::DoubleItemDefinitionPtr>(node, idef);
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processIntDef(pugi::xml_node &node,
                                      attribute::IntItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node,
                        smtk::dynamic_pointer_cast<ValueItemDefinition>(idef));
  processDerivedValueDef<attribute::IntItemDefinitionPtr>(node, idef);
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processStringDef(pugi::xml_node &node,
                                         attribute::StringItemDefinitionPtr idef)
{
  // First process the common value item def stuff
  this->processValueDef(node,
                        smtk::dynamic_pointer_cast<ValueItemDefinition>(idef));
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
//----------------------------------------------------------------------------
void XmlV2StringWriter::processModelEntityDef(pugi::xml_node& node,
                                         attribute::ModelEntityItemDefinitionPtr idef)
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

//----------------------------------------------------------------------------
void XmlV2StringWriter::processMeshSelectionItemDef(pugi::xml_node &node,
                      smtk::attribute::MeshSelectionItemDefinitionPtr idef)
{
  // this->processItemDefinition(node, idef);
  node.append_attribute("ModelEntityRef").set_value(
    idef->refModelEntityName().c_str());

  smtk::model::BitFlags membershipMask = idef->membershipMask();
  std::string membershipMaskStr = this->encodeModelEntityMask(membershipMask);
  xml_node menode;
  menode = node.append_child("MembershipMask");
  menode.text().set(membershipMaskStr.c_str());

}

//----------------------------------------------------------------------------
void XmlV2StringWriter::processMeshEntityDef(pugi::xml_node& node,
                                         smtk::attribute::MeshItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
    {
    node.append_attribute("Extensible") = true;
    if (idef->maxNumberOfValues())
      {
      node.append_attribute("MaxNumberOfValues") =
        static_cast<unsigned int>(idef->maxNumberOfValues());
      }
    }
}

//----------------------------------------------------------------------------
void XmlV2StringWriter::processValueDef(pugi::xml_node &node,
                                        smtk::attribute::ValueItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
    {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfValues())
      {
      node.append_attribute("MaxNumberOfValues") = static_cast<unsigned int>(idef->maxNumberOfValues());
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
    attribute::DefinitionPtr  exp = idef->expressionDefinition();
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
       iter != idef->childrenItemDefinitions().end();
       ++iter)
    {
    itemDefNode = itemDefNodes.append_child();
    itemDefNode.set_name(Item::type2String(iter->second->type()).c_str());
    this->processItemDefinition(itemDefNode,
                                iter->second);
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processRefDef(pugi::xml_node &node,
                                      attribute::RefItemDefinitionPtr idef)
{
  attribute::DefinitionPtr  adp = idef->attributeDefinition();
  if (adp)
    {
    xml_node anode;
    anode = node.append_child("AttDef");
    anode.text().set(adp->type().c_str());
    }
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
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
//----------------------------------------------------------------------------
void XmlV2StringWriter::processDirectoryDef(pugi::xml_node &node,
                                            attribute::DirectoryItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") = static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
    {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfValues())
      {
      node.append_attribute("MaxNumberOfValues") = static_cast<unsigned int>(idef->maxNumberOfValues());
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
//----------------------------------------------------------------------------
void XmlV2StringWriter::processFileDef(pugi::xml_node &node,
                                       attribute::FileItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") = static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
    {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfValues())
      {
      node.append_attribute("MaxNumberOfValues") = static_cast<unsigned int>(idef->maxNumberOfValues());
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
  std::string fileFilters = idef->getFileFilters();
  if (fileFilters != "")
    {
    node.append_attribute("FileFilters") = fileFilters.c_str();
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processGroupDef(pugi::xml_node &node,
                                        attribute::GroupItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredGroups") = static_cast<unsigned int>(idef->numberOfRequiredGroups());
  if (idef->isExtensible())
    {
    node.append_attribute("Extensible").set_value("true");
    if (idef->maxNumberOfGroups())
      {
      node.append_attribute("MaxNumberOfGroups") = static_cast<unsigned int>(idef->maxNumberOfGroups());
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
      this->processItemDefinition(itemDefNode,
                                  idef->itemDefinition(i));
      }
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processAttribute(xml_node &attributes,
                                         attribute::AttributePtr att)
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
  ModelEntityItemPtr assoc = att->associations();
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
//----------------------------------------------------------------------------
void XmlV2StringWriter::processItem(xml_node &node,
                                    smtk::attribute::ItemPtr item)
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

  switch (item->type())
    {
    case Item::ATTRIBUTE_REF:
      this->processRefItem(node, smtk::dynamic_pointer_cast<RefItem>(item));
      break;
    case Item::DOUBLE:
      this->processDoubleItem(node, smtk::dynamic_pointer_cast<DoubleItem>(item));
      break;
    case Item::DIRECTORY:
      this->processDirectoryItem(node, smtk::dynamic_pointer_cast<DirectoryItem>(item));
      break;
    case Item::FILE:
      this->processFileItem(node, smtk::dynamic_pointer_cast<FileItem>(item));
      break;
    case Item::GROUP:
      this->processGroupItem(node, smtk::dynamic_pointer_cast<GroupItem>(item));
      break;
    case Item::INT:
      this->processIntItem(node, smtk::dynamic_pointer_cast<IntItem>(item));
      break;
    case Item::STRING:
      this->processStringItem(node, smtk::dynamic_pointer_cast<StringItem>(item));
      break;
    case Item::MODEL_ENTITY:
      this->processModelEntityItem(node, smtk::dynamic_pointer_cast<ModelEntityItem>(item));
      break;
    case Item::VOID:
      // Nothing to do!
      break;
    case Item::MESH_SELECTION:
      this->processMeshSelectionItem(node, smtk::dynamic_pointer_cast<MeshSelectionItem>(item));
      break;
    case Item::MESH_ENTITY:
      this->processMeshEntityItem(node, smtk::dynamic_pointer_cast<MeshItem>(item));
      break;
    default:
      smtkErrorMacro(this->m_logger,
                     "Unsupported Type: " << Item::type2String(item->type())
                     << " for Item: " << item->name());
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processValueItem(pugi::xml_node &node,
                                         attribute::ValueItemPtr item)
{
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  size_t i, n = item->numberOfValues();

  // If the item can have variable number of values then store how many
  // values it has
  if (item->isExtensible())
    {
    node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
    }

  if (!n)
    {
    return;  // nothing else to be done
    }

  if (!item->isDiscrete())
    {
    return; // there is nothing else to be done
    }

  if (item->numberOfChildrenItems())
    {
    xml_node childNode, childNodes = node.append_child("ChildrenItems");
    std::map<std::string, smtk::attribute::ItemPtr>::const_iterator iter;
    const std::map<std::string, smtk::attribute::ItemPtr> &childrenItems = item->childrenItems();
    for (iter = childrenItems.begin();
         iter != childrenItems.end();
         iter++)
      {
      childNode = childNodes.append_child();
      childNode.set_name(Item::type2String(iter->second->type()).c_str());
      this->processItem(childNode, iter->second);
      }
    }

  xml_node val, values;
  if ((numRequiredVals == 1)  && !item->isExtensible())// Special Common Case
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
//----------------------------------------------------------------------------
void XmlV2StringWriter::processDoubleItem(pugi::xml_node &node,
                                          attribute::DoubleItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::DoubleItemPtr>(node, item);
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processIntItem(pugi::xml_node &node,
                                       attribute::IntItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::IntItemPtr>(node, item);
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processStringItem(pugi::xml_node &node,
                                          attribute::StringItemPtr item)
{
  this->processValueItem(node,
                         dynamic_pointer_cast<ValueItem>(item));
  processDerivedValue<attribute::StringItemPtr>(node, item);
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processModelEntityItem(pugi::xml_node &node,
                                          attribute::ModelEntityItemPtr item)
{
  size_t i=0, n = item->numberOfValues();
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  // we should always have "NumberOfValues" set
  node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));

  xml_node val;
  if (!n)
    {
    return;
    }

  if (numRequiredVals == 1)
    {
    if (item->isSet())
      {
      val = node.append_child("Val");
      val.text().set(item->value(i).entity().toString().c_str());
      }
    return;
    }
  xml_node values = node.append_child("Values");
  for(i = 0; i < n; i++)
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

//----------------------------------------------------------------------------
void XmlV2StringWriter::processMeshEntityItem(pugi::xml_node &node,
                                          attribute::MeshItemPtr item)
{
  size_t i=0, n = item->numberOfValues();
  // we should always have "NumberOfValues" set
  node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));

  if (!n)
    {
    return;
    }

  xml_node values = node.append_child("Values");
  smtk::attribute::MeshItem::const_mesh_it it;
  xml_node val;
  for(it = item->begin(); it != item->end(); ++it, ++i)
    {
    if(item->isSet(i))
      {
      val = values.append_child("Val");
      val.append_attribute("collectionid").set_value(
        it->collection()->entity().toString().c_str());
      cJSON* jrange = smtk::mesh::to_json(it->range());
      char* json = cJSON_Print(jrange);
      cJSON_Delete(jrange);
      val.text().set(json);
      free(json);
      }
    }
}

//----------------------------------------------------------------------------
void XmlV2StringWriter::processMeshSelectionItem(pugi::xml_node &node,
                          smtk::attribute::MeshSelectionItemPtr item)
{
  size_t n = item->numberOfValues();
  node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
  xml_node val;
  val = node.append_child("CtrlKey");
  val.text().set(item->isCtrlKeyDown() ? 1 : 0);

  val = node.append_child("MeshModifyMode");
  val.text().set(MeshSelectionItem::modifyMode2String(
                 item->modifyMode()).c_str());
  if (!n)
    {
    return;
    }

  xml_node values, selValues = node.append_child("SelectionValues");
  smtk::attribute::MeshSelectionItem::const_sel_map_it it;
  for(it = item->begin(); it != item->end(); ++it)
    {
    values = selValues.append_child("Values");
    values.append_attribute("EntityUUID").set_value(it->first.toString().c_str());
    std::set<int>::const_iterator vit;
    for(vit = it->second.begin(); vit !=  it->second.end(); ++vit)
      {
      val = values.append_child("Val");
      val.text().set(*vit);
      }
    }
}

//----------------------------------------------------------------------------
void XmlV2StringWriter::processRefItem(pugi::xml_node &node,
                                       attribute::RefItemPtr item)
{
  size_t i=0, n = item->numberOfValues();
  std::size_t  numRequiredVals = item->numberOfRequiredValues();

  xml_node val;
  if (!n)
    {
    return;
    }

  if (!numRequiredVals)
    {
    node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
    }

  if (numRequiredVals == 1)
    {
    if (item->isSet())
      {
      val = node.append_child("Val");
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
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      val.text().set(item->value(i)->name().c_str());
      }
    else
      {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      }
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processDirectoryItem(pugi::xml_node &node,
                                             attribute::DirectoryItemPtr item)
{
  size_t i, n = item->numberOfValues();
  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  if (!n)
    {
    return;
    }

  // If the item can have variable number of values then store how many
  // values it has
  if (!numRequiredVals)
    {
    node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
    }

  if (numRequiredVals == 1)
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

//----------------------------------------------------------------------------
void XmlV2StringWriter::processFileItem(pugi::xml_node &node,
                                        attribute::FileItemPtr item)
{
  // always write out all recentValues
  if (item->recentValues().size() > 0)
    {
    xml_node recval, recvalues = node.append_child("RecentValues");
    std::vector<std::string>::const_iterator it;
    for(it = item->recentValues().begin(); it!= item->recentValues().end(); ++it)
      {
      recval = recvalues.append_child("Val");
      recval.text().set((*it).c_str());
      }
    }

  std::size_t  numRequiredVals = item->numberOfRequiredValues();
  size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }

  // If the item can have variable number of values then store how many
  // values it has
  if (!numRequiredVals)
    {
    node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));
    }

  if (numRequiredVals == 1) // Special Common Case
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
//----------------------------------------------------------------------------
void XmlV2StringWriter::processGroupItem(pugi::xml_node &node,
                                         attribute::GroupItemPtr item)
{
  size_t i, j, m, n;
  std::size_t  numRequiredGroups = item->numberOfRequiredGroups();
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
  for(i = 0; i < n; i++)
    {
    cluster = clusters.append_child("Cluster");
    cluster.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    for (j = 0; j < m; j++)
      {
      itemNode = cluster.append_child();
      itemNode.set_name(Item::type2String(item->item(i,j)->type()).c_str());
      this->processItem(itemNode, item->item(i,j));
      }
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processViews()
{
  this->m_pugi->root.append_child(node_comment).set_value("********** Workflow Views ***********");

  // First write toplevel views and then write out the non-toplevel - note that the
  // attribute or view system do care about this - the assumption is that the designer would
  // probably like all the toplevel views clustered together
  
  xml_node views = this->m_pugi->root.append_child("Views");
  std::map<std::string, smtk::common::ViewPtr>::const_iterator iter;
  bool isTop;
  for (iter = this->m_system.views().begin(); iter != this->m_system.views().end(); iter++)
    {
    if (!(iter->second->details().attributeAsBool("TopLevel", isTop) && isTop))
      {
      continue;
      }
    xml_node node;
    node = views.append_child("View");
            
    node.append_attribute("Type").set_value(iter->second->type().c_str());
    node.append_attribute("Title").set_value(iter->second->title().c_str());
    if (iter->second->iconName() != "")
      {
      node.append_attribute("Icon").set_value(iter->second->iconName().c_str());
      }
    this->processViewComponent(iter->second->details(), node);
    }
  for (iter = this->m_system.views().begin(); iter != this->m_system.views().end(); iter++)
    {
    if (iter->second->details().attributeAsBool("TopLevel", isTop) && isTop)
      {
      continue;
      }
    xml_node node;
    node = views.append_child("View");
            
    node.append_attribute("Type").set_value(iter->second->type().c_str());
    node.append_attribute("Title").set_value(iter->second->title().c_str());
    if (iter->second->iconName() != "")
      {
      node.append_attribute("Icon").set_value(iter->second->iconName().c_str());
      }
    this->processViewComponent(iter->second->details(), node);
    }
}
//----------------------------------------------------------------------------
void XmlV2StringWriter::processViewComponent(smtk::common::View::Component &comp,
                                             xml_node &node)
{
  // Add the attributes of the component to the node
  std::map<std::string, std::string>::const_iterator iter;
  for (iter = comp.attributes().begin(); iter != comp.attributes().end(); iter++)
    {
    node.append_attribute(iter->first.c_str()).
      set_value(iter->second.c_str());
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

//----------------------------------------------------------------------------
void XmlV2StringWriter::processModelInfo()
{
  /** This seems to be outdated with ModelEntityItem already being processed
  **/
}

//----------------------------------------------------------------------------
std::string XmlV2StringWriter::encodeColor(const double *c)
{
  std::stringstream oss;
  oss << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3];
  std::string result = oss.str();
  return result;
}
//----------------------------------------------------------------------------
std::string XmlV2StringWriter::encodeModelEntityMask(smtk::model::BitFlags f)
{
  return smtk::model::Entity::flagToSpecifierString(f);
}
//----------------------------------------------------------------------------

  } // namespace io
} // namespace smtk
