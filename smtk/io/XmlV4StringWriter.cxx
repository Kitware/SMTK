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
  const attribute::ResourcePtr myResource, smtk::io::Logger& logger)
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

} // namespace io
} // namespace smtk
