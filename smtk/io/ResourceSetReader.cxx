//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ResourceSetReader.h"

#include "smtk/io/AttributeReader.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/attribute/Collection.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/model/Manager.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>
#include <sstream>

using namespace smtk::common;
using namespace smtk::resource;

namespace smtk
{
namespace io
{

ResourceSetReader::ResourceSetReader()
{
}

ResourceSetReader::ResourceSetReader(smtk::model::ManagerPtr mgr)
  : m_modelManager(mgr)
{
}

bool ResourceSetReader::readFile(
  std::string filename, Set& resources, smtk::io::Logger& logger, bool loadLinkedFiles)
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
  std::cout << "path \"" << path.string() << "\"" << std::endl;
  resources.setLinkStartPath(path.string());

  // Parse string
  return this->readString(content, resources, logger, loadLinkedFiles);
}

bool ResourceSetReader::readString(const std::string& content, Set& resources,
  smtk::io::Logger& logger, bool loadLinkedFiles, ResourceMapType* resourceMap)
{
  std::stringstream ss; // for log messages

  // Load content into xml document
  pugi::xml_document document;
  pugi::xml_parse_result presult = document.load_buffer(content.c_str(), content.size());
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

  for (resourceElement = rootElement.first_child(); resourceElement;
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

    Set::Role role = Set::NOT_DEFINED;
    std::string srole = resourceElement.attribute("role").value();
    if (srole.length() > 0)
    {
      role = Set::string2Role(srole);
    }

    if ((tagName == "attribute") && (role == Set::NOT_DEFINED))
    {
      ss.str("");
      ss << "No role defined for attribute " << id;
      smtkErrorMacro(logger, ss.str());
      continue;
    }

    // Initialize resource
    ResourcePtr r;
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
          resources.add(r, id, link, role);
        }
        else
        {
          resources.addInfo(id, role, Set::LOAD_ERROR, link);
          break;
        }
      }
      else
      {
        resources.addInfo(id, role, Set::NOT_LOADED, link);
      }
    }
    else if (childTagName == "SMTK_AttributeManager" || childTagName == "SMTK_AttributeSystem")
    {
      //std::cout << "  Embedded\n";
      std::string path = resources.linkStartPath();
      if (readEmbeddedAttCollection(childElement, r, path, logger))
      {
        resources.add(r, id, "", role);
      }
      else
      {
        resources.addInfo(id, role, Set::LOAD_ERROR);
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

bool ResourceSetReader::readEmbeddedAttCollection(pugi::xml_node& element,
  smtk::resource::ResourcePtr& resource, std::string& linkStartPath, smtk::io::Logger& logger)
{
  // Initialize attribute collection
  smtk::attribute::CollectionPtr collection;
  // If input resource is empty, create attribute manager
  if (resource)
  {
    collection = smtk::dynamic_pointer_cast<smtk::attribute::Collection>(resource);
  }
  else
  {
    collection = smtk::attribute::Collection::create();
    resource = smtk::resource::ResourcePtr(collection);
  }

  if (!collection)
  {
    smtkErrorMacro(logger, "Failed to initialize attribute collection");
    return false;
  }

  if (m_modelManager && !collection->refModelManager())
  {
    collection->setRefModelManager(m_modelManager);
  }

  // Instantiate AttributeReader and load contents
  AttributeReader reader;
  if (linkStartPath != "")
  {
    std::vector<std::string> searchPaths;
    searchPaths.push_back(linkStartPath);
    reader.setSearchPaths(searchPaths);
  }
  reader.readContents(collection, element, logger);
  return !logger.hasErrors();
}

bool ResourceSetReader::readIncludedManager(const pugi::xml_node& element,
  smtk::resource::ResourcePtr& resource, std::string& path, smtk::io::Logger& logger)
{
  (void)element;
  // // Make sure path exists
  if (!boost::filesystem::exists(path))
  {
    std::stringstream ss;
    ss << "No file at include path \"" << path << "\"";
    smtkErrorMacro(logger, ss.str());
    return false;
  }

  // Initialize attribute collection
  smtk::attribute::CollectionPtr collection;
  // If input resource is empty, create attribute manager
  if (resource)
  {
    collection = smtk::dynamic_pointer_cast<smtk::attribute::Collection>(resource);
  }
  else
  {
    collection = smtk::attribute::Collection::create();
    resource = smtk::resource::ResourcePtr(collection);
  }

  if (!collection)
  {
    smtkErrorMacro(logger, "Failed to initialize attribute collection");
    return false;
  }

  smtk::io::AttributeReader reader;
  bool hasErr = reader.read(collection, path, true, logger);
  if (hasErr)
  {
    return false;
  }

  return true;
}

std::string ResourceSetReader::buildIncludePath(const Set& resources, const std::string link) const
{
  std::string path = link; // return value

  // Add linkStartPath if link not absolute
  boost::filesystem::path inputPath(link);
  if (!inputPath.has_root_directory() && resources.linkStartPath() != "")
  {
    boost::filesystem::path outputPath(resources.linkStartPath());
    outputPath /= inputPath; // (concatenate)
    path = outputPath.string();
  }

  return path;
}

} // namespace io
} // namespace smtk
