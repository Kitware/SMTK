//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV8StringWriter.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;

namespace smtk
{
namespace io
{

XmlV8StringWriter::XmlV8StringWriter(
  const attribute::ResourcePtr myResource,
  smtk::io::Logger& logger)
  : XmlV7StringWriter(myResource, logger)
{
}

XmlV8StringWriter::~XmlV8StringWriter() = default;

std::string XmlV8StringWriter::className() const
{
  return std::string("XmlV8StringWriter");
}

unsigned int XmlV8StringWriter::fileVersion() const
{
  return 8;
}

void XmlV8StringWriter::processDefinitionInternal(
  pugi::xml_node& definition,
  smtk::attribute::DefinitionPtr def)
{
  XmlV7StringWriter::processDefinitionInternal(definition, def);

  if (!def->localUnits().empty())
  {
    definition.append_attribute("Units").set_value(def->localUnits().c_str());
  }
  // Add its ID
  definition.append_attribute("ID").set_value(def->id().toString().c_str());
}

} // namespace io
} // namespace smtk
