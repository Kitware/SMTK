//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV7StringWriter.h"

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

XmlV7StringWriter::XmlV7StringWriter(
  const attribute::ResourcePtr myResource,
  smtk::io::Logger& logger)
  : XmlV6StringWriter(myResource, logger)
{
}

XmlV7StringWriter::~XmlV7StringWriter() = default;

std::string XmlV7StringWriter::className() const
{
  return std::string("XmlV7StringWriter");
}

unsigned int XmlV7StringWriter::fileVersion() const
{
  return 7;
}

} // namespace io
} // namespace smtk
