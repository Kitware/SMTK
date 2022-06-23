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
#include "smtk/io/XmlDocV4Parser.h"
#include "smtk/io/XmlDocV5Parser.h"
#include "smtk/io/XmlDocV6Parser.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryInfo.h"
#include "smtk/view/Configuration.h"
#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
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

namespace smtk
{
namespace io
{

// Hides Pugi related stuff
class AttributeReaderInternals
{
public:
  // Returns the attribute resource root node in a pugi doc
  pugi::xml_node getRootNode(pugi::xml_document& doc);

  // Returns the complete path to the file.  If the file does not exist it will return the
  // original filename
  std::string getDirectory(const std::string& fname, const std::vector<std::string>& spaths);

  bool scanIncludes(
    pugi::xml_node& root,
    smtk::attribute::FileInfo& finfo,
    std::vector<std::string>& includeStack,
    const std::set<std::string>& activeIncludes,
    const std::vector<std::string>& spaths,
    Logger& logger);

  void parseXml(
    smtk::attribute::ResourcePtr resource,
    pugi::xml_node& root,
    bool reportAsError,
    Logger& logger);

  void readAttributes(
    smtk::attribute::ResourcePtr resource,
    const std::string& initialFileName,
    pugi::xml_node& root,
    const std::vector<std::string>& spaths,
    bool reportAsError,
    Logger& logger);

  void print();
  void print(smtk::attribute::ResourcePtr resource);
  smtk::attribute::DirectoryInfo m_dirInfo;
  std::size_t m_currentFileIndex;
};
}; // namespace io
}; // namespace smtk

void AttributeReaderInternals::print()
{
  std::cerr << "Attribute Directory Info:\n";
  for (auto dinfo : m_dirInfo)
  {
    dinfo.print("  ");
  }
}
void AttributeReaderInternals::print(smtk::attribute::ResourcePtr resource)
{
  this->print();
  std::vector<smtk::attribute::DefinitionPtr> result;
  std::vector<smtk::attribute::AttributePtr> attResult;
  resource->definitions(result);
  resource->attributes(attResult);
  auto viewResult = resource->views();
  std::cerr << "Definitions:\n";
  for (const auto& def : result)
  {
    std::cerr << "  Def: " << def->type() << " Include Id: " << def->includeIndex() << std::endl;
  }
  std::cerr << "Attributes:\n";
  for (const auto& att : attResult)
  {
    std::cerr << "  Att: " << att->name() << " Include Id: " << att->includeIndex() << std::endl;
  }
  std::cerr << "Views:\n";
  for (auto it = viewResult.begin(); it != viewResult.end(); ++it)
  {
    std::cerr << "  View: " << it->second->type() << " Include Id: " << it->second->includeIndex()
              << std::endl;
  }
}

// Returns the attribute resource root node in a pugi doc
pugi::xml_node AttributeReaderInternals::getRootNode(pugi::xml_document& doc)
{
  if (XmlDocV6Parser::canParse(doc))
  {
    return XmlDocV6Parser::getRootNode(doc);
  }

  if (XmlDocV5Parser::canParse(doc))
  {
    return XmlDocV5Parser::getRootNode(doc);
  }

  if (XmlDocV4Parser::canParse(doc))
  {
    return XmlDocV4Parser::getRootNode(doc);
  }

  if (XmlDocV3Parser::canParse(doc))
  {
    return XmlDocV3Parser::getRootNode(doc);
  }

  if (XmlDocV2Parser::canParse(doc))
  {
    return XmlDocV2Parser::getRootNode(doc);
  }

  if (XmlDocV1Parser::canParse(doc))
  {
    return XmlDocV1Parser::getRootNode(doc);
  }

  pugi::xml_node temp; // no node found
  return temp;
}

// Returns the complete path to the file.  If the file does not exist it will
// return the original filename
std::string AttributeReaderInternals::getDirectory(
  const std::string& fname,
  const std::vector<std::string>& spaths)
{
  // Are we dealing with an absolute path
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

bool AttributeReaderInternals::scanIncludes(
  pugi::xml_node& root,
  smtk::attribute::FileInfo& finfo,
  std::vector<std::string>& includeStack,
  const std::set<std::string>& activeIncludes,
  const std::vector<std::string>& spaths,
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
    std::string fname = this->getDirectory(fnameInitial, spaths);

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
      smtkErrorMacro(
        logger,
        "Problem loading in " << fname << " Error Description:\n"
                              << presult.description());
      return true;
    }
    std::set<std::string> newSet = activeIncludes;
    newSet.insert(fname);

    // See if any of the parsers can get the root node
    pugi::xml_node root1 = this->getRootNode(doc1);
    if (!root1)
    {
      smtkErrorMacro(logger, "Cannot find attribute resource root node in file " << fname);
      return true;
    }

    // Get ready for the next file to be processed
    smtk::attribute::FileInfo nextFileInfo(fnameInitial);
    // Get the Category Info for this file so we can save it - NOTE that all current readers
    // use the same format for this info!
    std::set<std::string> catagories;
    std::string defCat;
    XmlDocV1Parser::getCategories(root1, catagories, defCat);
    nextFileInfo.setCatagories(catagories);
    nextFileInfo.setDefaultCatagory(defCat);
    if (this->scanIncludes(root1, nextFileInfo, includeStack, newSet, spaths, logger))
    {
      smtkErrorMacro(logger, "Problem processing " << fname);
      return true;
    }
    // Ok add this file to the list of includes associated with the
    // node being processed
    finfo.appendInclude(fnameInitial);
    // Get the nodes
    m_dirInfo.push_back(nextFileInfo);
    // If we are here then we were able to process all of the include files this one depends on!
    includeStack.push_back(fname);
  }
  return false; // everything is ok!
}

void AttributeReaderInternals::parseXml(
  smtk::attribute::ResourcePtr resource,
  pugi::xml_node& root,
  bool reportAsError,
  Logger& logger)
{
  if (!root)
  {
    smtkErrorMacro(logger, "Attribute resource root node is missing");
    return;
  }

  // Lets see if any of the parsers can process the node
  if (XmlDocV6Parser::canParse(root))
  {
    XmlDocV6Parser theReader(resource, logger);
    theReader.setIncludeFileIndex(m_currentFileIndex);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
  }
  else if (XmlDocV5Parser::canParse(root))
  {
    XmlDocV5Parser theReader(resource, logger);
    theReader.setIncludeFileIndex(m_currentFileIndex);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
  }
  else if (XmlDocV4Parser::canParse(root))
  {
    XmlDocV4Parser theReader(resource, logger);
    theReader.setIncludeFileIndex(m_currentFileIndex);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
  }
  else if (XmlDocV3Parser::canParse(root))
  {
    XmlDocV3Parser theReader(resource, logger);
    theReader.setIncludeFileIndex(m_currentFileIndex);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
  }
  else if (XmlDocV2Parser::canParse(root))
  {
    XmlDocV2Parser theReader(resource, logger);
    theReader.setIncludeFileIndex(m_currentFileIndex);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
  }
  else if (XmlDocV1Parser::canParse(root))
  {
    XmlDocV1Parser theReader(resource, logger);
    theReader.setIncludeFileIndex(m_currentFileIndex);
    theReader.setReportDuplicateDefinitionsAsErrors(reportAsError);
    theReader.process(root);
  }
  else
  {
    smtkErrorMacro(logger, "Unsupported Attribute Resource Format");
  }
}

void AttributeReaderInternals::readAttributes(
  smtk::attribute::ResourcePtr resource,
  const std::string& initialFileName,
  pugi::xml_node& root,
  const std::vector<std::string>& spaths,
  bool reportAsError,
  Logger& logger)
{
  if (!root)
  {
    smtkErrorMacro(logger, "Root node is missing");
    return;
  }

  // First lets process the include files
  std::vector<std::string> includeStack;
  std::set<std::string> activeIncludes;
  smtk::attribute::FileInfo myFileInfo(initialFileName);
  // Get the Category Info for this file so we can save it - NOTE that all current readers
  // use the same format for this info!
  std::set<std::string> catagories;
  std::string defaultCat;
  XmlDocV1Parser::getCategories(root, catagories, defaultCat);
  myFileInfo.setCatagories(catagories);
  myFileInfo.setDefaultCatagory(defaultCat);
  // If doc was read in from a file, seed the active include set to start with it
  if (!initialFileName.empty())
  {
    activeIncludes.insert(initialFileName);
  }
  if (this->scanIncludes(root, myFileInfo, includeStack, activeIncludes, spaths, logger))
  {
    smtkErrorMacro(logger, "Problem occured traversing includes!");
    return;
  }
  // We want the toplevel file to be at the start of the vector.  This way all  information not
  // contained in an include file or created after the information is read  will have an include
  // index of 0.
  m_dirInfo.insert(m_dirInfo.begin(), myFileInfo);
  // now reverse the stack
  std::reverse(includeStack.begin(), includeStack.end());
  // Now lets process the includes starting with the back and working to the front
  // First set the file index to be the file we are going to be processing
  m_currentFileIndex = 1;

  while (!includeStack.empty())
  {
    pugi::xml_document doc1;
    doc1.load_file(includeStack.back().c_str());
    // Lets get the root attribute resource node
    pugi::xml_node root1 = this->getRootNode(doc1);
    if (!root)
    {
      smtkErrorMacro(
        logger, "Root attribute resource node is missing from " << includeStack.back());
      return;
    }

    this->parseXml(resource, root1, reportAsError, logger);
    if (logger.hasErrors())
    {
      return;
    }
    includeStack.pop_back();
    ++m_currentFileIndex;
  }
  // Finally process the initial doc
  //Sanity Check - the current file index should be 0 when we get here!
  if (m_currentFileIndex != m_dirInfo.size())
  {
    smtkErrorMacro(
      logger, "Problem processing include files - incorrect count!" << includeStack.back());
  }
  // Ok lets process the original file
  m_currentFileIndex = 0;
  this->parseXml(resource, root, reportAsError, logger);
  resource->setDirectoryInfo(m_dirInfo);
}

AttributeReader::AttributeReader()
{
  m_internals = new AttributeReaderInternals();
}

AttributeReader::~AttributeReader()
{
  delete m_internals;
}

bool AttributeReader::read(
  smtk::attribute::ResourcePtr resource,
  const std::string& filename,
  bool includePath,
  Logger& logger)
{
  logger.reset();
  m_internals->m_dirInfo.clear();
  // First load in the xml document
  pugi::xml_document doc;
  pugi::xml_parse_result presult = doc.load_file(filename.c_str());
  if (presult.status != pugi::status_ok)
  {
    smtkErrorMacro(logger, presult.description());
    return true;
  }

  // Get root element
  pugi::xml_node root = m_internals->getRootNode(doc);
  if (!root)
  {
    smtkErrorMacro(logger, "Cannot find root attribute resource node in file " << filename);
    return true;
  }

  if (includePath)
  {
    // Add the path to the file as a search path
    path p(filename);
    std::vector<std::string> newSPaths(1, p.parent_path().string());
    newSPaths.insert(newSPaths.end(), m_searchPaths.begin(), m_searchPaths.end());
    m_internals->readAttributes(resource, filename, root, newSPaths, m_reportAsError, logger);
  }
  else
  {
    m_internals->readAttributes(resource, filename, root, m_searchPaths, m_reportAsError, logger);
  }
  return logger.hasErrors();
}

bool AttributeReader::readContents(
  smtk::attribute::ResourcePtr resource,
  const std::string& filecontents,
  Logger& logger)
{
  return this->readContents(resource, filecontents.c_str(), filecontents.size(), logger);
}

bool AttributeReader::readContents(
  smtk::attribute::ResourcePtr resource,
  const char* content,
  std::size_t length,
  Logger& logger)
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
  pugi::xml_node root = m_internals->getRootNode(doc);
  return this->readContents(resource, root, logger);
}

bool AttributeReader::readContents(
  smtk::attribute::ResourcePtr resource,
  pugi::xml_node& root,
  Logger& logger)
{
  logger.reset();
  m_internals->m_dirInfo.clear();
  if (root)
  {
    m_internals->readAttributes(resource, "", root, m_searchPaths, m_reportAsError, logger);
  }
  else
  {
    smtkErrorMacro(logger, "Can not find attribute resource root node");
  }
  return logger.hasErrors();
}
