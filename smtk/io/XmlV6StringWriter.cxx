//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV6StringWriter.h"

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

XmlV6StringWriter::XmlV6StringWriter(
  const attribute::ResourcePtr myResource,
  smtk::io::Logger& logger)
  : XmlV5StringWriter(myResource, logger)
{
}

XmlV6StringWriter::~XmlV6StringWriter() = default;

std::string XmlV6StringWriter::className() const
{
  return std::string("XmlV6StringWriter");
}

unsigned int XmlV6StringWriter::fileVersion() const
{
  return 6;
}

} // namespace io
} // namespace smtk
