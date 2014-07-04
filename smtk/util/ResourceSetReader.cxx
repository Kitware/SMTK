/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
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
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "ResourceSetReader.h"
#include "pugixml-1.2/src/pugixml.hpp"
#include "smtk/attribute/Manager.h"
#include "smtk/util/AttributeReader.h"
#include "smtk/util/XmlDocV1Parser.h"

#include "boost/filesystem.hpp"
#include <fstream>
#include <sstream>

using namespace smtk::util;


//----------------------------------------------------------------------------
bool
ResourceSetReader::
readFile(std::string filename,
         ResourceSet& resources,
         smtk::util::Logger& logger,
         bool loadLinkedFiles)
{
  // Load filename into string and call readString()
  std::ifstream in(filename.c_str(), std::ios::in);
  if (!in)
    {
    std::stringstream ss;
    ss << "Unabled to open file " << filename;
    smtkErrorMacro(logger, ss.str());
    return true;
    }

  // Allocate string
  std::string content;
  in.seekg(0, std::ios::end);
  content.resize(in.tellg());

  // Read file
  in.seekg(0, std::ios::beg);
  in.read(&content[0], content.size());
  in.close();

  // Set linkStartPath for any linked resources
  boost::filesystem::path path(filename);
  path.remove_filename();
  std::cout << "path \"" << path.native() << "\"" << std::endl;
  resources.setLinkStartPath(path.native());

  // Parse string
  return this->readString(content, resources, logger, loadLinkedFiles);
}

//----------------------------------------------------------------------------
bool
ResourceSetReader::
readString(const std::string& content,
           ResourceSet& resources,
           smtk::util::Logger& logger,
           bool loadLinkedFiles,
           ResourceMapType *resourceMap)
{
  std::stringstream ss;  // for log messages

  // Load content into xml document
  pugi::xml_document document;
  pugi::xml_parse_result presult =
    document.load_buffer(content.c_str(), content.size());
  if (presult.status != pugi::status_ok)
    {
    smtkErrorMacro(logger, presult.description());
    return true;
    }

  // Retrieve <cmb-resources> element
  pugi::xml_node rootElement = document.child("cmb-resources");
  if (!rootElement)
    {
    smtkErrorMacro(logger, "Can not find root node: cmb-resources");
    return true;
    }

  // Traverse all first-level nodes
  pugi::xml_node resourceElement;

  for (resourceElement = rootElement.first_child();
       resourceElement;
       resourceElement = resourceElement.next_sibling())
    {
    std::string tagName = resourceElement.name();
    if (tagName != "attribute")
      {
      ss.str("");
      ss << "Unrecognized tag " << tagName;
      smtkErrorMacro(logger, ss.str());
      continue;
      }

    std::string id = resourceElement.attribute("id").value();
    if (id == "")
      {
      ss.str("");
      ss << tagName << " tag with no id specified";
      smtkErrorMacro(logger, ss.str());
      continue;
      }
    //std::cout << "Element " << tagName << " id " << id << "\n";

    ResourceSet::ResourceRole role = ResourceSet::NOT_DEFINED;
    std::string srole = resourceElement.attribute("role").value();
    if (srole.length() > 0)
      {
      role = ResourceSet::string2Role(srole);
      }

    if ((tagName == "attribute") &&
        (role == ResourceSet::NOT_DEFINED))
      {
      ss.str("");
      ss << "No role defined for attribute " << id;
      smtkErrorMacro(logger, ss.str());
      continue;
      }

    // Initialize resource
    smtk::util::ResourcePtr r;
    if (resourceMap)
      {
      // Check for pre-loaded resource
      ResourceMapType::const_iterator iter = resourceMap->find(id);
      if (iter != resourceMap->end())
        {
        r = iter->second;
        }
      }

    // Check child element for embedded vs included attribute
    // (Note: code is verbose for readability)
    pugi::xml_node childElement = resourceElement.first_child();
    std::string childTagName = childElement.name();
    if (childTagName == "include")
      {
      // Get the link from the element
      std::string link = childElement.attribute("href").value();
      std::cout << "Included as link \"" << link << "\"\n";
      if (link == "")
        {
        smtkErrorMacro(logger, "<include> element missing href link");
        return false;
        }

      if (loadLinkedFiles)
        {
        std::string path = this->buildIncludePath(resources, link);
        std::cout << "Load include path \"" << path << "\"" << std::endl;
        if (readIncludedManager(childElement, r, path, logger))
          {
          resources.addResource(r, id, link, role);
          }
        else
          {
          smtk::util::Resource::Type type = smtk::util::Resource::string2Type(tagName);
          resources.addResourceInfo(id, type, role, ResourceSet::LOAD_ERROR, link);
          break;
          }
        }
      else
        {
        smtk::util::Resource::Type type = smtk::util::Resource::string2Type(tagName);
        resources.addResourceInfo(id, type, role, ResourceSet::NOT_LOADED, link);
        }
      }
    else if (childTagName == "SMTK_AttributeManager")
      {
      //std::cout << "  Embedded\n";
      std::string path = resources.linkStartPath();
      if (readEmbeddedManager(childElement, r, path, logger))
        {
        resources.addResource(r, id, "", role);
        }
      else
        {
        smtk::util::Resource::Type type = smtk::util::Resource::string2Type(tagName);
        resources.addResourceInfo(id, type, role, ResourceSet::LOAD_ERROR);
        break;
        }
      }
    else
      {
      ss.str("");
      ss << "Unrecognized child element " << childTagName;
      smtkErrorMacro(logger, ss.str());
      continue;
      }

    }

  return logger.hasErrors();
}

//----------------------------------------------------------------------------
bool
ResourceSetReader::
readEmbeddedManager(pugi::xml_node& element,
                    smtk::util::ResourcePtr& resource,
                    std::string& linkStartPath,
                    smtk::util::Logger& logger)
{
  // Initialize attribute manager
  smtk::attribute::Manager *manager = NULL;
  // If input resource is empty, create attribute manager
  if (resource)
    {
    manager = dynamic_cast<smtk::attribute::Manager *>(resource.get());
    }
  else
    {
    manager = new smtk::attribute::Manager();
    resource = smtk::util::ResourcePtr(manager);
    }

  if (!manager)
    {
    smtkErrorMacro(logger, "Failed to initialize attribute manager");
    return false;
    }

  // Instantiate AttributeReader and load contents
  AttributeReader reader;
  if (linkStartPath != "")
    {
    std::vector<std::string> searchPaths;
    searchPaths.push_back(linkStartPath);
    reader.setSearchPaths(searchPaths);
    }
  reader.readContents(*manager, element, logger);
  return !logger.hasErrors();
}


//----------------------------------------------------------------------------
bool
ResourceSetReader::
readIncludedManager(const pugi::xml_node& element,
                    smtk::util::ResourcePtr& resource,
                    std::string& path,
                    smtk::util::Logger& logger)
{
  // // Make sure path exists
  if (!boost::filesystem::exists(path))
    {
    std::stringstream ss;
    ss << "No file at include path \"" << path << "\"";
    smtkErrorMacro(logger, ss.str());
    return false;
    }

  // Initialize attribute manager
  smtk::attribute::Manager *manager = NULL;
  // If input resource is empty, create attribute manager
  if (resource)
    {
    manager = dynamic_cast<smtk::attribute::Manager *>(resource.get());
    }
  else
    {
    manager = new smtk::attribute::Manager();
    resource = smtk::util::ResourcePtr(manager);
    }

  if (!manager)
    {
    smtkErrorMacro(logger, "Failed to initialize attribute manager");
    return false;
    }

  smtk::util::AttributeReader reader;
  bool hasErr = reader.read(*manager, path, true, logger);
  if (hasErr)
    {
    return false;
    }

  return true;
}


//----------------------------------------------------------------------------
std::string
ResourceSetReader::
buildIncludePath(const ResourceSet& resources,
                 const std::string link) const
{
  std::string path = link;  // return value

  // Add linkStartPath if link not absolute
  boost::filesystem::path inputPath(link);
  if (!inputPath.has_root_directory() &&
      resources.linkStartPath() != "")
    {
    boost::filesystem::path outputPath(resources.linkStartPath());
    outputPath /= inputPath;
    path = outputPath.c_str();
    }

  return path;
}
