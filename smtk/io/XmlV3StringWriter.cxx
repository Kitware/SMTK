//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV3StringWriter.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

#include "smtk/attribute/Attribute.h"
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
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/io/Logger.h"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;
using namespace smtk::common;

namespace smtk
{
namespace io
{

XmlV3StringWriter::XmlV3StringWriter(
  const attribute::ResourcePtr myResource,
  smtk::io::Logger& logger)
  : XmlV2StringWriter(myResource, logger)
{
}

XmlV3StringWriter::~XmlV3StringWriter() = default;

std::string XmlV3StringWriter::className() const
{
  return std::string("XmlV3StringWriter");
}

unsigned int XmlV3StringWriter::fileVersion() const
{
  return 3;
}

std::string XmlV3StringWriter::rootNodeName() const
{
  return std::string("SMTK_AttributeResource");
}

void XmlV3StringWriter::generateXml()
{
  XmlV2StringWriter::generateXml();
  auto root = this->topRootNode();

  // Are we saving out the unique role information?
  if (m_includeUniqueRoles)
  {
    // Do we have unique roles to save?
    const std::set<smtk::resource::Links::RoleType>& roles = m_resource->uniqueRoles();
    if (!roles.empty())
    {
      auto uNodes = root.append_child("UniqueRoles");
      for (const auto& role : m_resource->uniqueRoles())
      {
        auto rNode = uNodes.append_child("Role");
        rNode.append_attribute("ID").set_value(role);
      }
    }
  }
  if (m_includeResourceAssociations)
  {
    // Process Resource Association Info
    auto associations = m_resource->associations();
    // Find the XML to place this info
    auto node =
      (this->topDefinitionsNode()
         ? this->topDefinitionsNode()
         : ((this->topAttributesNode() ? this->topAttributesNode() : this->topViewsNode())));
    xml_node associationsNode;
    if (node)
    {
      associationsNode = root.insert_child_before("Associations", node);
    }
    else
    {
      associationsNode = root.append_child("Associations");
    }

    for (const auto& association : associations)
    {
      xml_node associationNode = associationsNode.append_child("Association");
      // Because resource links are not serialized into XML, we add enough data
      // to each reference item to recreate the link.
      associationNode.append_attribute("Index").set_value(
        static_cast<unsigned int>(association->index()));
      associationNode.append_attribute("TypeName").set_value(association->typeName().c_str());
      associationNode.append_attribute("Id").set_value(association->id().toString().c_str());
      associationNode.append_attribute("Location").set_value(association->location().c_str());
    }
  }
  // Lets process the constraints if we are saving definition info
  if (m_includeDefinitions)
  {
    // Lets see if we have any exclusions or prerequisites constraints
    xml_node exNode, preNode, child, n, node;
    // First lets get the definitions in sorted order
    std::vector<DefinitionPtr> defs;
    m_resource->definitions(defs, true);

    for (const auto& def : defs)
    {
      auto defType = def->type();
      auto excludedTypes = def->excludedTypeNames();

      // First lets process exclusions
      if (!excludedTypes.empty())
      {
        // First we need to see if we need to create the XML node
        if (!exNode)
        {
          // This should be right after the definitions section if there is
          // one else it should be before prerequisites, attributes and views
          node = this->topDefinitionsNode();
          if (node)
          {
            exNode = root.insert_child_after("Exclusions", node);
          }
          else
          {
            node = preNode
              ? preNode
              : ((this->topAttributesNode()) ? this->topAttributesNode() : this->topViewsNode());
            if (node)
            {
              exNode = root.insert_child_before("Exclusions", node);
            }
            else
            {
              exNode = root.append_child("Exclusions");
            }
          }
        }
        // We will now write out pair-wise rules starting with the
        // first definition in the list that comes after this definition
        // alphabetically - assuming all of the skipped rules have already been added
        for (const auto& etype : excludedTypes)
        {
          if (etype > defType)
          {
            child = exNode.append_child("Rule");
            n = child.append_child("Def");
            n.text().set(defType.c_str());
            n = child.append_child("Def");
            n.text().set(etype.c_str());
          }
        }
      }
      // Now lets process prerequisites
      auto prerequisitesTypes = def->prerequisiteTypeNames();

      if (!prerequisitesTypes.empty())
      {
        // First we need to see if we need to create the XML node
        if (!preNode)
        {
          // This should be right after the exclusion or definitions section if there is
          // one else it should be before attributes and views
          node = (exNode) ? exNode : this->topDefinitionsNode();
          if (node)
          {
            preNode = root.insert_child_after("Prerequisites", node);
          }
          else
          {
            node = (this->topAttributesNode()) ? this->topAttributesNode() : this->topViewsNode();
            if (node)
            {
              preNode = root.insert_child_before("Prerequisites", node);
            }
            else
            {
              preNode = root.append_child("Prerequisites");
            }
          }
        }
        child = preNode.append_child("Rule");
        // set the xml type attribute
        child.append_attribute("Type").set_value(defType.c_str());
        // Write out all prerequisitesTypes
        for (const auto& ptype : prerequisitesTypes)
        {
          n = child.append_child("Def");
          n.text().set(ptype.c_str());
        }
      }
    }
  }
}

void XmlV3StringWriter::processDefinitionInternal(xml_node& definition, DefinitionPtr def)
{
  if (!def->tags().empty())
  {
    xml_node tagsNode = definition.append_child();
    tagsNode.set_name("Tags");

    std::string sep; // TODO: The writer could accept a user-provided separator.
    for (const auto& tag : def->tags())
    {
      xml_node tagNode = tagsNode.append_child();
      tagNode.set_name("Tag");
      tagNode.append_attribute("Name").set_value(tag.name().c_str());
      if (!tag.values().empty())
      {
        tagNode.text().set(concatenate(tag.values(), sep, &m_logger).c_str());
      }
    }
  }

  auto& localExpression = def->localCategories();
  // Lets write out the category stuff
  xml_node catGroupNode, catExpNode = definition.append_child("CategoryExpression");
  catExpNode.append_attribute("InheritanceMode")
    .set_value(Categories::combinationModeAsString(def->categoryInheritanceMode()).c_str());
  if (localExpression.isSet())
  {
    if (!localExpression.expression().empty())
    {
      catExpNode.text().set(localExpression.expression().c_str());
    }
    else
    {
      if (localExpression.allPass())
      {
        catExpNode.append_attribute("PassMode").set_value("All");
      }
      else
      {
        catExpNode.append_attribute("PassMode").set_value("None");
      }
    }
  }

  XmlV2StringWriter::processDefinitionInternal(definition, def);
}

void XmlV3StringWriter::processItemDefinitionType(xml_node& node, ItemDefinitionPtr idef)
{
  switch (idef->type())
  {
    case Item::DateTimeType:
      this->processDateTimeDef(node, smtk::dynamic_pointer_cast<DateTimeItemDefinition>(idef));
      break;

    case Item::ReferenceType:
      this->processReferenceDef(node, std::dynamic_pointer_cast<ReferenceItemDefinition>(idef));
      break;

    case Item::ResourceType:
      this->processResourceDef(node, std::dynamic_pointer_cast<ResourceItemDefinition>(idef));
      break;

    case Item::ComponentType:
      this->processComponentDef(node, std::dynamic_pointer_cast<ComponentItemDefinition>(idef));
      break;

    default:
      XmlV2StringWriter::processItemDefinitionType(node, idef);
      break;
  }
}

void XmlV3StringWriter::processDateTimeDef(pugi::xml_node& node, DateTimeItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());

  std::string format = idef->displayFormat();
  if (!format.empty())
  {
    node.append_attribute("DisplayFormat").set_value(format.c_str());
  }
  node.append_attribute("ShowTimeZone") = idef->useTimeZone();
  node.append_attribute("ShowCalendarPopup") = idef->useCalendarPopup();

  if (idef->hasDefault())
  {
    xml_node defnode = node.append_child("DefaultValue");
    ::smtk::common::DateTimeZonePair dtz = idef->defaultValue();
    defnode.text().set(dtz.serialize().c_str());
  }

  // if (idef->hasRange())
  //   {
  //   xml_node rnode = node.append_child("RangeInfo");
  //   xml_node r;
  //   bool inclusive;
  //   if (idef->hasMinRange())
  //     {
  //     r = rnode.append_child("Min");
  //     inclusive = idef->minRangeInclusive();
  //     r.append_attribute("Inclusive").set_value(inclusive);
  //     r.text().set(getValueForXMLElement(idef->minRange()));
  //     }
  //   if (idef->hasMaxRange())
  //     {
  //     r = rnode.append_child("Max");
  //     inclusive = idef->maxRangeInclusive();
  //     r.append_attribute("Inclusive").set_value(inclusive);
  //     r.text().set(getValueForXMLElement(idef->maxRange()));
  //     }
  //   }
}

void XmlV3StringWriter::processItemType(xml_node& node, ItemPtr item)
{
  switch (item->type())
  {
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

    default:
      XmlV2StringWriter::processItemType(node, item);
      break;
  }
}

void XmlV3StringWriter::processDateTimeItem(pugi::xml_node& node, attribute::DateTimeItemPtr item)
{
  size_t numValues = item->numberOfValues();
  xml_node val;
  if (numValues == 0)
  {
    return;
  }

  // (else)
  if (numValues == 1)
  {
    if (item->isSet())
    {
      ::smtk::common::DateTimeZonePair dtz = item->value();
      node.text().set(dtz.serialize().c_str());
    }
    else
    {
      node.append_child("UnsetVal");
    }
    return;
  }

  // (else)
  xml_node values = node.append_child("Values");
  for (std::size_t i = 0; i < numValues; ++i)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      ::smtk::common::DateTimeZonePair dtz = item->value();
      val.text().set(dtz.serialize().c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV3StringWriter::processReferenceDef(pugi::xml_node& node, ReferenceItemDefinitionPtr idef)
{
  this->processReferenceDefCommon(node, idef, "ReferenceLabels");
}

void XmlV3StringWriter::processReferenceItem(pugi::xml_node& node, attribute::ReferenceItemPtr item)
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
    // Added children information if needed
    if (item->numberOfChildrenItems())
    {
      xml_node childNode, childNodes = node.append_child("ChildrenItems");
      std::map<std::string, ItemPtr>::const_iterator iter;
      const std::map<std::string, ItemPtr>& childrenItems = item->childrenItems();
      for (iter = childrenItems.begin(); iter != childrenItems.end(); iter++)
      {
        childNode = childNodes.append_child();
        childNode.set_name(Item::type2String(iter->second->type()).c_str());
        this->processItem(childNode, iter->second);
      }
      // Save the conditional that is currently being used if it is valid
      std::size_t conditional = item->currentConditional();
      if (conditional != ReferenceItemDefinition::s_invalidIndex)
      {
        node.append_attribute("Conditional").set_value(static_cast<unsigned int>(conditional));
      }
    }

    if (item->isSet())
    {
      val = node.append_child("Val");

      // Because resource links are not serialized into XML, we add enough data
      // to each reference item to recreate the link.
      auto objKey = item->objectKey(i);

      xml_node key = val.append_child("Key");
      auto key1 = key.append_child("_1_");
      key1.text().set(objKey.first.toString().c_str());
      auto key2 = key.append_child("_2_");
      key2.text().set(objKey.second.toString().c_str());

      const smtk::resource::Surrogate& surrogate =
        item->attribute()->resource()->links().data().value(objKey.first);

      xml_node rhs = val.append_child("RHS");
      auto rhs1 = rhs.append_child("_1_");
      rhs1.text().set(
        item->attribute()->resource()->links().data().at(objKey.first).right.toString().c_str());
      auto rhs2 = rhs.append_child("_2_");
      rhs2.text().set(item->attribute()
                        ->resource()
                        ->links()
                        .data()
                        .at(objKey.first)
                        .at(objKey.second)
                        .right.toString()
                        .c_str());

      val.append_attribute("Role").set_value(
        item->attribute()->resource()->links().data().at(objKey.first).at(objKey.second).role);

      xml_node surrogateNode = val.append_child("Surrogate");
      surrogateNode.append_attribute("Index").set_value(
        static_cast<unsigned int>(surrogate.index()));
      surrogateNode.append_attribute("TypeName").set_value(surrogate.typeName().c_str());
      surrogateNode.append_attribute("Id").set_value(surrogate.id().toString().c_str());
      surrogateNode.append_attribute("Location").set_value(surrogate.location().c_str());
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

      // Because resource links are not serialized into XML, we add enough dta
      // to each reference item to recreate the link.
      auto objKey = item->objectKey(i);

      xml_node key = val.append_child("Key");
      auto key1 = key.append_child("_1_");
      key1.text().set(objKey.first.toString().c_str());
      auto key2 = key.append_child("_2_");
      key2.text().set(objKey.second.toString().c_str());

      const smtk::resource::Surrogate& surrogate =
        item->attribute()->resource()->links().data().value(objKey.first);

      xml_node rhs = val.append_child("RHS");
      auto rhs1 = rhs.append_child("_1_");
      rhs1.text().set(
        item->attribute()->resource()->links().data().at(objKey.first).right.toString().c_str());
      auto rhs2 = rhs.append_child("_2_");
      rhs2.text().set(item->attribute()
                        ->resource()
                        ->links()
                        .data()
                        .at(objKey.first)
                        .at(objKey.second)
                        .right.toString()
                        .c_str());

      val.append_attribute("Role").set_value(
        item->attribute()->resource()->links().data().at(objKey.first).at(objKey.second).role);

      xml_node surrogateNode = val.append_child("Surrogate");
      surrogateNode.append_attribute("Index").set_value(
        static_cast<unsigned int>(surrogate.index()));
      surrogateNode.append_attribute("TypeName").set_value(surrogate.typeName().c_str());
      surrogateNode.append_attribute("Id").set_value(surrogate.id().toString().c_str());
      surrogateNode.append_attribute("Location").set_value(surrogate.location().c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV3StringWriter::processResourceDef(pugi::xml_node& node, ResourceItemDefinitionPtr idef)
{
  this->processReferenceDefCommon(node, idef, "ResourceLabels");
}

void XmlV3StringWriter::processResourceItem(pugi::xml_node& node, attribute::ResourceItemPtr item)
{
  this->processReferenceItem(node, item);
}

void XmlV3StringWriter::processComponentDef(pugi::xml_node& node, ComponentItemDefinitionPtr idef)
{
  this->processReferenceDefCommon(node, idef, "ComponentLabels");
}

void XmlV3StringWriter::processComponentItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  this->processReferenceItem(node, item);
}

void XmlV3StringWriter::processReferenceDefCommon(
  pugi::xml_node& node,
  ReferenceItemDefinitionPtr idef,
  const std::string& labelName)
{
  node.append_attribute("EnforceCategories") = idef->enforcesCategories();
  auto acceptableEntries = idef->acceptableEntries();
  xml_node accnode = node.append_child("Accepts");

  for (const auto& entry : acceptableEntries)
  {
    xml_node rsrcnode = accnode.append_child("Resource");
    rsrcnode.append_attribute("Name").set_value(entry.first.c_str());
    if (!entry.second.empty())
    {
      rsrcnode.append_attribute("Filter").set_value(entry.second.c_str());
    }
  }

  xml_node rejnode = node.append_child("Rejects");

  auto rejectedEntries = idef->rejectedEntries();
  for (const auto& entry : rejectedEntries)
  {
    xml_node rsrcnode = rejnode.append_child("Resource");
    rsrcnode.append_attribute("Name").set_value(entry.first.c_str());
    if (!entry.second.empty())
    {
      rsrcnode.append_attribute("Filter").set_value(entry.second.c_str());
    }
  }

  if (idef->onlyResources())
  {
    node.append_attribute("OnlyResources") = true;
  }

  if (idef->lockType() != smtk::resource::LockType::Write)
  {
    node.append_attribute("LockType")
      .set_value((idef->lockType() == smtk::resource::LockType::DoNotLock ? "DoNotLock" : "Read"));
  }

  if (idef->holdReference())
  {
    node.append_attribute("HoldReference") = true;
  }

  node.append_attribute("Role") = idef->role();

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
    lnode.set_name(labelName.c_str());
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

  // Lets process its children items
  if (!idef->numberOfChildrenItemDefinitions())
  {
    return;
  }
  xml_node itemDefNode, itemDefNodes = node.append_child("ChildrenDefinitions");
  std::map<std::string, ItemDefinitionPtr>::const_iterator iter;
  for (const auto& childDefInfo : idef->childrenItemDefinitions())
  {
    itemDefNode = itemDefNodes.append_child();
    itemDefNode.set_name(Item::type2String(childDefInfo.second->type()).c_str());
    this->processItemDefinition(itemDefNode, childDefInfo.second);
  }
  //Now process the conditionals
  if (!idef->numberOfConditionals())
  {
    return;
  }
  const std::vector<std::vector<std::string>>& conditionals = idef->conditionalInformation();
  const std::vector<std::string>& rqueries = idef->resourceQueries();
  const std::vector<std::string>& cqueries = idef->componentQueries();
  std::size_t n = conditionals.size();
  xml_node conditionNode, conditionalNodes = node.append_child("ConditionalInfo");
  for (std::size_t i = 0; i < n; i++)
  {
    conditionNode = conditionalNodes.append_child("Condition");
    if (!rqueries[i].empty())
    {
      conditionNode.append_attribute("Resource") = rqueries[i].c_str();
    }
    if (!cqueries[i].empty())
    {
      conditionNode.append_attribute("Component") = cqueries[i].c_str();
    }

    if (!conditionals[i].empty())
    {
      xml_node itemNameNode, itemNameNodes = conditionNode.append_child("Items");
      std::size_t numItems = conditionals[i].size();
      for (std::size_t j = 0; j < numItems; j++)
      {
        itemNameNode = itemNameNodes.append_child("Item");
        itemNameNode.text().set(conditionals[i][j].c_str());
      }
    }
  }
}

} // namespace io
} // namespace smtk
