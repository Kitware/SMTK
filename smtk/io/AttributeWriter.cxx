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
#include "smtk/io/XmlV6StringWriter.h"
#include <cassert>
#include <fstream>

#define DEFAULT_FILE_VERSION 6
#define MAX_FILE_VERSION 6
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
{
}

bool AttributeWriter::setFileVersion(unsigned int version)
{
  // Validate input
  if ((version >= 3) && (version <= MAX_FILE_VERSION))
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
  const smtk::attribute::ResourcePtr resource,
  const std::string& filename,
  Logger& logger)
{
  // Lets first clear the logger's error state
  logger.clearErrors();
  XmlStringWriter* theWriter = this->newXmlStringWriter(resource, logger);
  theWriter->includeAnalyses(m_includeAnalyses);
  theWriter->includeAdvanceLevels(m_includeAdvanceLevels);
  theWriter->includeAttributeAssociations(m_includeAttributeAssociations);
  theWriter->includeDefinitions(m_includeDefinitions);
  theWriter->includeInstances(m_includeInstances);
  theWriter->includeResourceAssociations(m_includeResourceAssociations);
  theWriter->includeResourceID(m_includeResourceID);
  theWriter->includeUniqueRoles(m_includeUniqueRoles);
  theWriter->includeViews(m_includeViews);
  theWriter->useDirectoryInfo(m_useDirectoryInfo);
  theWriter->setIncludedDefinitions(m_includedDefs);
  theWriter->setExcludedDefinitions(m_excludedDefs);

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
        smtkErrorMacro(
          logger,
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

bool AttributeWriter::writeContents(
  const smtk::attribute::ResourcePtr resource,
  std::string& filecontents,
  Logger& logger,
  bool no_declaration)
{
  logger.clearErrors();
  XmlStringWriter* theWriter = this->newXmlStringWriter(resource, logger);
  theWriter->includeAnalyses(m_includeAnalyses);
  theWriter->includeAdvanceLevels(m_includeAdvanceLevels);
  theWriter->includeAttributeAssociations(m_includeAttributeAssociations);
  theWriter->includeDefinitions(m_includeDefinitions);
  theWriter->includeInstances(m_includeInstances);
  theWriter->includeUniqueRoles(m_includeUniqueRoles);
  theWriter->includeResourceAssociations(m_includeResourceAssociations);
  theWriter->includeResourceID(m_includeResourceID);
  theWriter->includeViews(m_includeViews);
  theWriter->setIncludedDefinitions(m_includedDefs);
  theWriter->setExcludedDefinitions(m_excludedDefs);
  theWriter->useDirectoryInfo(false);
  filecontents = theWriter->convertToString(no_declaration);
  delete theWriter;
  return logger.hasErrors();
}

XmlStringWriter* AttributeWriter::newXmlStringWriter(
  const smtk::attribute::ResourcePtr resource,
  smtk::io::Logger& logger) const
{
  XmlStringWriter* writer = nullptr;
  switch (m_fileVersion)
  {
    case 3:
    case 4:
    case 5:
    case 6:
      writer = new XmlV6StringWriter(resource, logger);
      break;

    default:
      assert("Invalid file version");
      break;
  }
  return writer;
}

void AttributeWriter::setIncludedDefinitions(
  const std::vector<smtk::attribute::DefinitionPtr>& includedDefs)
{
  // Lets make sure there are no redundant definitions - these would cause attribute
  // instances to be written out multiple times
  std::size_t i, j, n = includedDefs.size();
  std::vector<bool> isRedundant;
  isRedundant.resize(n, false);
  for (i = 0; i < n; i++)
  {
    // Skip the i th def if it has been marked redundant
    if (isRedundant[i])
    {
      continue;
    }
    // Mark and skip all nullptr Defs
    if (includedDefs[i] == nullptr)
    {
      isRedundant[i] = true;
      continue;
    }

    for (j = i + 1; j < includedDefs.size(); j++)
    {
      // Skip testing j if its redundant
      if (isRedundant[j])
      {
        continue;
      }
      // If the i th definition is derived from the j th definition we can just skip it
      if (includedDefs[i]->isA(includedDefs[j]))
      {
        isRedundant[i] = true;
        break;
      }
      // If j th definition is derived from the i th then mark it redundant
      if (includedDefs[j]->isA(includedDefs[i]))
      {
        isRedundant[j] = true;
      }
    }
  }
  // Now lets save all non-redundant definitions
  m_includedDefs.clear();
  for (i = 0; i < n; i++)
  {
    if (!isRedundant[i])
    {
      m_includedDefs.push_back(includedDefs[i]);
    }
  }
}

void AttributeWriter::setExcludedDefinitions(
  const std::set<smtk::attribute::DefinitionPtr>& excludedDefs)
{
  // We do not need to prune exclusions as you can only die once, Mr. Bond.
  m_excludedDefs = excludedDefs;
}

void AttributeWriter::treatAsLibrary(
  const std::vector<smtk::attribute::DefinitionPtr>& includedDefs)
{
  // Lets turn off the appropriate sections
  m_includeAnalyses = false;
  m_includeAdvanceLevels = false;
  m_includeAttributeAssociations = false;
  m_includeDefinitions = false;
  m_useDirectoryInfo = false;
  m_includeResourceAssociations = false;
  m_includeResourceID = false;
  m_includeUniqueRoles = false;
  m_includeViews = false;
  this->setIncludedDefinitions(includedDefs);
}

} // namespace io
} // namespace smtk

#undef DEFAULT_FILE_VERSION
#undef MAX_FILE_VERSION
