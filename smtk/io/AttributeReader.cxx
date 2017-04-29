//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/XmlDocV1Parser.h"
#include "smtk/io/XmlDocV2Parser.h"
#include "smtk/io/XmlDocV3Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
using namespace smtk::io;

namespace
{

// Returns the attribute system root node in a pugi doc
pugi::xml_node Internal_getRootNode(pugi::xml_document& doc)
{
  if (XmlDocV1Parser::canParse(doc))
  {
    return XmlDocV1Parser::getRootNode(doc);
  }

  if (XmlDocV2Parser::canParse(doc))
  {
    return XmlDocV2Parser::getRootNode(doc);
  }

  if (XmlDocV3Parser::canParse(doc))
  {
    return XmlDocV3Parser::getRootNode(doc);
  }

  pugi::xml_node temp; // no node found
  return temp;
}

// Returns the complete path to the file.  If the file does n
std::string Internal_getDirectory(const std::string& fname, const std::vector<std::string>& spaths)
{
  // Are we dealing with an absoulte path
  path p(fname);
  if (!p.root_path().empty())
  {
    return fname;
  }
  std::size_t i, n = spaths.size();
  for (i = 0; i < n; i++)
  {
    path sp(spaths[i]);
    // Add the file name to the possible path
    sp /= fname;
    if (exists(sp) && is_regular_file(sp))
    {
      return sp.string();
    }
  }
  return fname;
}

bool Internal_scanIncludes(pugi::xml_node& root, std::vector<std::string>& includeStack,
  const std::set<std::string>& activeIncludes, const std::vector<std::string>& spaths,
  Logger& logger)
{
  if (!root)
  {
    smtkErrorMacro(logger, "Root node is missing");
    return true;
  }

  // Lets get the include section
  pugi::xml_node includesNode = root.child("Includes");
  if (!includesNode)
  {
    return false; // There are no includes
  }

  for (pugi::xml_node n = includesNode.child("File"); n; n = n.next_sibling("File"))
  {
    std::string fnameInitial = n.text().get();
    // Based on the search paths find the correct filename
    std::string fname = Internal_getDirectory(fnameInitial, spaths);

    // Have we already processed this fname?
    if (std::find(includeStack.begin(), includeStack.end(), fname) != includeStack.end())
    {
      // this has already been processed so we can skip it
      continue;
    }
    // Are we already in the process of checking this file (aka is this a loop of includes?
    if (activeIncludes.find(fname) != activeIncludes.end())
    {
      smtkErrorMacro(
        logger, "Include Loop detected! - Encountered: " << fname << " a second time!");
      return true;
    }

    // Traverse this include file
    pugi::xml_document doc1;
    pugi::xml_parse_result presult = doc1.load_file(fname.c_str());
    if (presult.status != pugi::status_ok)
    {
      smtkErrorMacro(logger, "Problem loading in " << fname << " Error Description:\n"
                                                   << presult.description());
      return true;
    }
    std::set<std::string> newSet = activeIncludes;
    newSet.insert(fname);

    // See if any of the parsers can get the root node
    pugi::xml_node root1 = Internal_getRootNode(doc1);
    if (!root1)
    {
      smtkErrorMacro(logger, "Cannot find attribute system root node in file " << fname);
      return true;
    }
    if (Internal_scanIncludes(root1, includeStack, newSet, spaths, logger))
    {
      smtkErrorMacro(logger, "Problem processing " << fname);
      return true;
    }
    // If we are here then we were able to process all of the include files this one depends on!
    includeStack.push_back(fname);
  }
  return false; // everything is ok!
}

void Internal_parseXml(
  smtk::attribute::SystemPtr system, pugi::xml_node& root, bool reportAsError, Logger& logger)
{
  if (!root)
  {
    smtkErrorMacro(logger, "Attribute system root node is missing");
    return;
  }

  // Lets see if any of the parsers can process the node
  if (XmlDocV1Parser::canParse(root))
  {
    XmlDocV1Parser theReader(system);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
    logger.append(theReader.messageLog());
  }
  else if (XmlDocV2Parser::canParse(root))
  {
    XmlDocV2Parser theReader(system);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
    logger.append(theReader.messageLog());
  }
  else if (XmlDocV3Parser::canParse(root))
  {
    XmlDocV3Parser theReader(system);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
    logger.append(theReader.messageLog());
  }
  else
  {
    smtkErrorMacro(logger, "Unsupported Attribute System Format");
  }
}

void Internal_readAttributes(smtk::attribute::SystemPtr system, const std::string& initialFileName,
  pugi::xml_node& root, const std::vector<std::string>& spaths, bool reportAsError, Logger& logger)
{
  if (!root)
  {
    smtkErrorMacro(logger, "Root node is missing");
    return;
  }

  // First lets process the include files
  std::vector<std::string> includeStack;
  std::set<std::string> activeIncludes;
  // If doc was read in from a file, seed the active include set to start with it
  if (initialFileName != "")
  {
    activeIncludes.insert(initialFileName);
  }
  if (Internal_scanIncludes(root, includeStack, activeIncludes, spaths, logger))
  {
    smtkErrorMacro(logger, "Problem occured traversing includes!");
    return;
  }
  // now reverse the stack
  std::reverse(includeStack.begin(), includeStack.end());
  // Now lets process the includes starting with the back and working to the front
  while (includeStack.size())
  {
    pugi::xml_document doc1;
    doc1.load_file(includeStack.back().c_str());
    std::cout << "Processing Include File: " << includeStack.back().c_str() << "\n";
    // Lets get the root attribute system node
    pugi::xml_node root1 = Internal_getRootNode(doc1);
    if (!root)
    {
      smtkErrorMacro(logger, "Root attribute system node is missing from " << includeStack.back());
      return;
    }

    Internal_parseXml(system, root1, reportAsError, logger);
    if (logger.hasErrors())
    {
      return;
    }
    includeStack.pop_back();
  }
  // Finally process the initial doc
  Internal_parseXml(system, root, reportAsError, logger);
}
} // namespace

bool AttributeReader::read(
  smtk::attribute::SystemPtr system, const std::string& filename, bool includePath, Logger& logger)
{
  logger.reset();
  // First load in the xml document
  pugi::xml_document doc;
  pugi::xml_parse_result presult = doc.load_file(filename.c_str());
  if (presult.status != pugi::status_ok)
  {
    smtkErrorMacro(logger, presult.description());
    return true;
  }

  // Get root element
  pugi::xml_node root = Internal_getRootNode(doc);
  if (!root)
  {
    smtkErrorMacro(logger, "Cannot find root attribute system node in file " << filename);
    return true;
  }

  if (includePath)
  {
    // Add the path to the file as a search path
    path p(filename);
    std::vector<std::string> newSPaths(1, p.parent_path().string());
    newSPaths.insert(newSPaths.end(), this->m_searchPaths.begin(), this->m_searchPaths.end());
    Internal_readAttributes(system, filename, root, newSPaths, this->m_reportAsError, logger);
  }
  else
  {
    Internal_readAttributes(
      system, filename, root, this->m_searchPaths, this->m_reportAsError, logger);
  }
  return logger.hasErrors();
}

bool AttributeReader::readContents(
  smtk::attribute::SystemPtr system, const std::string& filecontents, Logger& logger)
{
  return this->readContents(system, filecontents.c_str(), filecontents.size(), logger);
}

bool AttributeReader::readContents(
  smtk::attribute::SystemPtr system, const char* content, std::size_t length, Logger& logger)
{
  logger.reset();
  // First load in the xml document
  pugi::xml_document doc;
  pugi::xml_parse_result presult = doc.load_buffer(content, length);
  if (presult.status != pugi::status_ok)
  {
    smtkErrorMacro(logger, presult.description());
    return true;
  }

  // Get root element
  pugi::xml_node root = Internal_getRootNode(doc);
  return this->readContents(system, root, logger);
}

bool AttributeReader::readContents(
  smtk::attribute::SystemPtr system, pugi::xml_node& root, Logger& logger)
{
  logger.reset();
  if (root)
  {
    Internal_readAttributes(system, "", root, this->m_searchPaths, this->m_reportAsError, logger);
  }
  else
  {
    smtkErrorMacro(logger, "Can not find attribute system root node");
  }
  return logger.hasErrors();
}
