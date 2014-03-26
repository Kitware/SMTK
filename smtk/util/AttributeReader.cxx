/*=========================================================================
Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"
#include "smtk/util/XmlDocV1Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.cpp"
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
using namespace smtk::util;

// Returns the complete path to the file.  If the file does n
std::string Internal_getDirectory(const std::string &fname,
                                  const std::vector<std::string> &spaths)
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
//----------------------------------------------------------------------------
bool Internal_scanIncludes(pugi::xml_document& doc,
                           std::vector<std::string> &includeStack,
                           const std::set<std::string> &activeIncludes,
                           const std::vector<std::string> &spaths,
                           Logger &logger)
{
  // Lets get the include section
  pugi::xml_node root = doc.child("SMTK_AttributeManager");
  if (!root)
    {
    smtkErrorMacro(logger, "Can not find root node: SMTK_AttributeManager");
    return true;
    }

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
      smtkErrorMacro(logger, "Include Loop detected! - Encountered: " << fname << " a second time!");
      return true;
      }

    // Traverse this include file
    pugi::xml_document doc1;
    pugi::xml_parse_result presult = doc1.load_file(fname.c_str());
    if (presult.status != pugi::status_ok)
      {
      smtkErrorMacro(logger, "Problem loading in " << fname << " Error Description:\n"<< presult.description());
      return true;
      }
    std::set<std::string> newSet = activeIncludes;
    newSet.insert(fname);
    if (Internal_scanIncludes(doc1, includeStack, newSet, spaths, logger))
      {
      smtkErrorMacro(logger, "Problem processing " << fname );
      return true;
      }
    // If we are here then we were able to process all of the include files this one depends on!
    includeStack.push_back(fname);
    }
  return false; // everything is ok!
}
//----------------------------------------------------------------------------
void Internal_parseXmlDoc(smtk::attribute::Manager &manager,
                          pugi::xml_document& doc,
                          Logger &logger)
{
  // Lets get the version of the Attribute File Format
  pugi::xml_node root = doc.child("SMTK_AttributeManager");
  if (!root)
    {
    smtkErrorMacro(logger, "Can not find root node: SMTK_AttributeManager");
    return;
    }

  pugi::xml_attribute xatt = root.attribute("Version");
  if (!xatt)
    {
    smtkErrorMacro(logger, "Can not find XML Attribute Version in node: SMTK_AttributeManager");
    return;
    }

  int versionNum = xatt.as_int();
  if (versionNum != 1)
    {
    smtkErrorMacro(logger, "Unsupported Attribute Version: " << versionNum);
    return;
    }
  XmlDocV1Parser theReader(manager);
  theReader.process(doc);
  logger.append(theReader.messageLog());
}

//----------------------------------------------------------------------------
void Internal_readAttributes(smtk::attribute::Manager &manager,
                             const std::string &initialFileName,
                             pugi::xml_document& doc,
                             const std::vector<std::string> &spaths,
                             Logger &logger)
{
  // First lets process the include files
  std::vector<std::string> includeStack;
  std::set<std::string> activeIncludes;
  // If doc was read in from a file, seed the active include set to start with it
  if (initialFileName != "")
    {
    activeIncludes.insert(initialFileName);
    }
  if (Internal_scanIncludes(doc, includeStack, activeIncludes, spaths, logger))
    {
    smtkErrorMacro(logger, "Problem occured traversing includes!");
    return;
    }
  // now reverse the stack
  std::reverse(includeStack.begin(), includeStack.end());
  // Now lets process the includes starting with the back and working to the front
  while(includeStack.size())
    {
    pugi::xml_document doc1;
    pugi::xml_parse_result presult = doc1.load_file(includeStack.back().c_str());
    std::cout << "Processing Include File: " << includeStack.back().c_str() << "\n";
    Internal_parseXmlDoc(manager, doc1, logger);
    if (logger.hasErrors())
      {
      return;
      }
    includeStack.pop_back();
    }
  // Finally process the initial doc
    Internal_parseXmlDoc(manager, doc, logger);
}

//----------------------------------------------------------------------------
bool AttributeReader::read(smtk::attribute::Manager &manager,
                           const std::string &filename, bool includePath,
                           Logger &logger)
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
  if (includePath)
    {
    // Add the path to the file as a search path
    path p(filename);
    std::vector<std::string> newSPaths(1, p.parent_path().string());
    newSPaths.insert(newSPaths.end(), this->m_searchPaths.begin(),
                     this->m_searchPaths.end());
    Internal_readAttributes(manager, filename, doc, newSPaths, logger);
    }
  else
    {
    Internal_readAttributes(manager, filename, doc, this->m_searchPaths, logger);
    }
  return logger.hasErrors();
}

//----------------------------------------------------------------------------
bool AttributeReader::readContents(smtk::attribute::Manager &manager,
                                   const std::string &filecontents,
                                   Logger &logger)
{
  logger.reset();
  // First load in the xml document
  pugi::xml_document doc;
  pugi::xml_parse_result presult = doc.load(filecontents.c_str());
  if (presult.status != pugi::status_ok)
    {
    smtkErrorMacro(logger, presult.description());
    return true;
    }
  Internal_readAttributes(manager, "", doc, this->m_searchPaths, logger);
  return logger.hasErrors();
}

//----------------------------------------------------------------------------
