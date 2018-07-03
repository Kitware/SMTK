//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ResourceSetWriter.h"

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/XmlStringWriter.h"

#include "smtk/attribute/Resource.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

using namespace smtk::common;
using namespace smtk::resource;

namespace smtk
{
namespace io
{

bool ResourceSetWriter::writeFile(
  std::string filename, const Set& resources, smtk::io::Logger& logger, LinkedFilesOption option)
{
  logger.reset();
  std::string content;
  this->writeString(content, resources, logger, option);
  if (!logger.hasErrors())
  {
    std::ofstream outfile;
    outfile.open(filename.c_str());
    outfile << content;
    outfile.close();
    std::cout << "Wrote " << filename << std::endl;
  }
  return logger.hasErrors();
}

bool ResourceSetWriter::writeString(
  std::string& content, const Set& resources, smtk::io::Logger& logger, LinkedFilesOption option)
{
  logger.reset();

  pugi::xml_document document;
  pugi::xml_node rootElement = document.append_child("cmb-resources");

  // Construct xml element for each resource
  std::vector<std::string> resourceIds = resources.resourceIds();
  Set::State state;
  Set::Role role;
  std::string link;
  bool ok = true;
  for (unsigned i = 0; i < resourceIds.size() && ok; ++i)
  {
    std::string id = resourceIds[i];
    ok = resources.resourceInfo(id, role, state, link);
    if (!ok)
    {
      std::cerr << "Error retrieving" << std::endl;
      continue;
    }

    // Use XmlStringWriter to generate xml for this attribute resource
    smtk::resource::ResourcePtr resource;
    ok = resources.get(id, resource);

    // Create valid tag from typeName, replacing "::" with "_",
    // since colon char not valid in xml tags
    std::regex re("::");
    std::string tag = std::regex_replace(resource->typeName(), re, "_");

    pugi::xml_node resourceElement = rootElement.append_child(tag.c_str());
    resourceElement.append_attribute("id").set_value(id.c_str());

    if (role != Set::NOT_DEFINED)
    {
      std::string rstring = Set::role2String(role);
      resourceElement.append_attribute("role").set_value(rstring.c_str());
    }

    if ((("" == link) || (EXPAND_LINKED_FILES == option)) && Set::LOADED == state)
    {
      smtk::attribute::ResourcePtr attResource =
        dynamic_pointer_cast<smtk::attribute::Resource>(resource);

      AttributeWriter attWriter;
      // Get the default string writer instance
      // Could consider allowing application to assign version number
      XmlStringWriter* xmlWriter = attWriter.newXmlStringWriter(attResource);
      xmlWriter->generateXml(resourceElement, logger);
      delete xmlWriter;
    }
    else if (link != "")
    {
      // If resource is linked, insert <include> element
      pugi::xml_node includeElement = resourceElement.append_child("include");
      includeElement.append_attribute("href").set_value(link.c_str());

      // Save linked resources if option selected
      if (option == WRITE_LINKED_FILES)
      {
        smtk::attribute::ResourcePtr attResource =
          dynamic_pointer_cast<smtk::attribute::Resource>(resource);
        AttributeWriter attWriter;
        bool hasErr = attWriter.write(attResource, link, logger);
        ok = !hasErr;
        if (ok)
        {
          std::cout << "Wrote linked file " << link << "\n";
        }
        else
        {
          std::cerr << "ERROR writing linked file " << link << "\n";
        }
      }
    }
  }

  if (!ok)
  {
    std::cerr << "ERROR writing resource file\n";
    return true;
  }

  // Serialize xml document
  std::stringstream oss;
  unsigned int flags = pugi::format_indent;
  document.save(oss, "  ", flags);
  content = oss.str();

  // Write for dev use
  //std::cout << "\n";
  //document.save(std::cout, "  ");
  //std::cout << "\n" << std::endl;

  return logger.hasErrors();
}

} // namespace io
} // namespace smtk
