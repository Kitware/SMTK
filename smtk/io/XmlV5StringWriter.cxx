//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV5StringWriter.h"

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

XmlV5StringWriter::XmlV5StringWriter(
  const attribute::ResourcePtr myResource,
  smtk::io::Logger& logger)
  : XmlV4StringWriter(myResource, logger)
{
}

XmlV5StringWriter::~XmlV5StringWriter() = default;

std::string XmlV5StringWriter::className() const
{
  return std::string("XmlV5StringWriter");
}

unsigned int XmlV5StringWriter::fileVersion() const
{
  return 5;
}

void XmlV5StringWriter::addHints()
{
  if (m_resource->properties().contains<bool>("smtk.attribute_panel.display_hint"))
  {
    auto& rootNode = this->topRootNode();
    rootNode.append_attribute("DisplayHint")
      .set_value(m_resource->properties().at<bool>("smtk.attribute_panel.display_hint"));
  }
}
} // namespace io
} // namespace smtk
