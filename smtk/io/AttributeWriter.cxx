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

#define DEFAULT_FILE_VERSION 3
#define MAX_FILE_VERSION 3
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace smtk
{
namespace io
{

AttributeWriter::AttributeWriter()
  : m_fileVersion(DEFAULT_FILE_VERSION)
  , m_includeDefinitions(true)
  , m_includeInstances(true)
  , m_includeViews(true)
  , m_useDirectoryInfo(false)
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
  const smtk::attribute::ResourcePtr resource, const std::string& filename, Logger& logger)
{
  // Lets first clear the logger's error state
  logger.clearErrors();
  XmlStringWriter* theWriter = this->newXmlStringWriter(resource, logger);
  theWriter->includeDefinitions(m_includeDefinitions);
  theWriter->includeInstances(m_includeInstances);
  theWriter->includeViews(m_includeViews);
  theWriter->useDirectoryInfo(m_useDirectoryInfo);

  std::string result = theWriter->convertToString();
  if (m_useDirectoryInfo && (!logger.hasErrors()))
  {
    path p(filename);
    path searchPath = p.parent_path();
    create_directories(searchPath);
    std::size_t i, num = resource->directoryInfo().size();
    for (i = 0; i < num; i++)
    {
      // The first entry refers to the new filename specified in the write
      // the rest should be the required include files that use relative paths
      path fpath((i == 0) ? filename : resource->directoryInfo().at(i).filename());
      if (i && fpath.is_relative())
      {
        path newFPath = searchPath / fpath;
        fpath = newFPath;
        create_directories(fpath.parent_path());
      }
      else if (i != 0)
      {
        smtkErrorMacro(logger,
          "Error - Will not over-write include file using absolute path: " << fpath.string());
        continue;
      }
      std::ofstream outfile;
      outfile.open(fpath.string().c_str(), std::ofstream::out | std::ofstream::trunc);
      if (!outfile)
      {
        smtkErrorMacro(logger, "Error opening file for writing: " << fpath.string());
      }
      else
      {
        outfile << theWriter->getString(i);
      }
      outfile.close();
    }
  }
  else if (!logger.hasErrors())
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

bool AttributeWriter::writeContents(const smtk::attribute::ResourcePtr resource,
  std::string& filecontents, Logger& logger, bool no_declaration)
{
  logger.clearErrors();
  XmlStringWriter* theWriter = this->newXmlStringWriter(resource, logger);
  theWriter->includeDefinitions(m_includeDefinitions);
  theWriter->includeInstances(m_includeInstances);
  theWriter->includeViews(m_includeViews);
  theWriter->useDirectoryInfo(false);
  filecontents = theWriter->convertToString(no_declaration);
  delete theWriter;
  return logger.hasErrors();
}

XmlStringWriter* AttributeWriter::newXmlStringWriter(
  const smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger) const
{
  XmlStringWriter* writer = NULL;
  switch (m_fileVersion)
  {
    case 2:
      writer = new XmlV2StringWriter(resource, logger);
      break;

    case 3:
      writer = new XmlV3StringWriter(resource, logger);
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
