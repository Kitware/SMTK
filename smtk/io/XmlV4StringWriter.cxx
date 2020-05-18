//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV4StringWriter.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;

namespace smtk
{
namespace io
{

XmlV4StringWriter::XmlV4StringWriter(
  const attribute::ResourcePtr myResource,
  smtk::io::Logger& logger)
  : XmlV3StringWriter(myResource, logger)
{
}

XmlV4StringWriter::~XmlV4StringWriter() = default;

std::string XmlV4StringWriter::className() const
{
  return std::string("XmlV4StringWriter");
}

unsigned int XmlV4StringWriter::fileVersion() const
{
  return 4;
}

void XmlV4StringWriter::processDefinitionInternal(
  pugi::xml_node& definition,
  smtk::attribute::DefinitionPtr def)
{
  XmlV3StringWriter::processDefinitionInternal(definition, def);

  auto associationRuleForDef =
    m_resource->associationRules().associationRulesForDefinitions().find(def->type());
  if (
    associationRuleForDef != m_resource->associationRules().associationRulesForDefinitions().end())
  {
    definition.append_child("AssociationRule")
      .append_attribute("Name")
      .set_value(associationRuleForDef->second.c_str());
  }

  auto dissociationRuleForDef =
    m_resource->associationRules().dissociationRulesForDefinitions().find(def->type());
  if (
    dissociationRuleForDef !=
    m_resource->associationRules().dissociationRulesForDefinitions().end())
  {
    definition.append_child("DissociationRule")
      .append_attribute("Name")
      .set_value(dissociationRuleForDef->second.c_str());
  }
}

void XmlV4StringWriter::processItemDefinitionAttributes(
  pugi::xml_node& node,
  smtk::attribute::ItemDefinitionPtr idef)
{
  XmlV3StringWriter::processItemDefinitionAttributes(node, idef);
  // Add support for ItemDef Tags
  if (!idef->tags().empty())
  {
    xml_node tagsNode = node.append_child();
    tagsNode.set_name("Tags");

    std::string sep; // TODO: The writer could accept a user-provided separator.
    for (auto& tag : idef->tags())
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
}
} // namespace io
} // namespace smtk
