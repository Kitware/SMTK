//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/io/XmlStringWriter.h"
#include "smtk/io/XmlV2StringWriter.h"
#include "smtk/io/XmlV3StringWriter.h"
#include <cassert>
#include <fstream>

#define DEFAULT_FILE_VERSION 2
#define MAX_FILE_VERSION 3

namespace smtk
{
namespace io
{

AttributeWriter::AttributeWriter()
  : m_fileVersion(DEFAULT_FILE_VERSION)
  , m_includeDefinitions(true)
  , m_includeInstances(true)
  , m_includeModelInformation(true)
  , m_includeViews(true)
{
}

bool AttributeWriter::setFileVersion(unsigned int version)
{
  // Validate input
  if ((version >= 2) && (version <= MAX_FILE_VERSION))
  {
    m_fileVersion = version;
    return true;
  }

  // (else)
  return false;
}

void AttributeWriter::setMaxFileVersion()
{
  m_fileVersion = MAX_FILE_VERSION;
}

unsigned int AttributeWriter::fileVersion() const
{
  return m_fileVersion;
}

bool AttributeWriter::write(
  const smtk::attribute::CollectionPtr collection, const std::string& filename, Logger& logger)
{
  logger.reset();
  XmlStringWriter* theWriter = this->newXmlStringWriter(collection);
  theWriter->includeDefinitions(m_includeDefinitions);
  theWriter->includeInstances(m_includeInstances);
  theWriter->includeModelInformation(m_includeModelInformation);
  theWriter->includeViews(m_includeViews);

  std::string result = theWriter->convertToString(logger);
  if (!logger.hasErrors())
  {
    std::ofstream outfile;
    outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (!outfile)
    {
      smtkErrorMacro(logger, "Error opening file for writing: " << filename);
    }
    else
    {
      outfile << result;
    }
    outfile.close();
  }
  delete theWriter;
  return logger.hasErrors();
}

bool AttributeWriter::writeContents(const smtk::attribute::CollectionPtr collection,
  std::string& filecontents, Logger& logger, bool no_declaration)
{
  logger.reset();
  XmlStringWriter* theWriter = this->newXmlStringWriter(collection);
  theWriter->includeDefinitions(m_includeDefinitions);
  theWriter->includeInstances(m_includeInstances);
  theWriter->includeModelInformation(m_includeModelInformation);
  theWriter->includeViews(m_includeViews);
  filecontents = theWriter->convertToString(logger, no_declaration);
  delete theWriter;
  return logger.hasErrors();
}

XmlStringWriter* AttributeWriter::newXmlStringWriter(
  const smtk::attribute::CollectionPtr collection) const
{
  XmlStringWriter* writer = NULL;
  switch (m_fileVersion)
  {
    case 2:
      writer = new XmlV2StringWriter(collection);
      break;

    case 3:
      writer = new XmlV3StringWriter(collection);
      break;

    default:
      assert("Invalid file version");
      break;
  }
  return writer;
}

} // namespace io
} // namespace smtk

#undef DEFAULT_FILE_VERSION
#undef MAX_FILE_VERSION
