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

#include "smtk/attribute/System.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace smtk::common;

namespace smtk
{
namespace io
{

bool ResourceSetWriter::writeFile(std::string filename, const ResourceSet& resources,
  smtk::io::Logger& logger, LinkedFilesOption option)
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

bool ResourceSetWriter::writeString(std::string& content, const ResourceSet& resources,
  smtk::io::Logger& logger, LinkedFilesOption option)
{
  logger.reset();

  pugi::xml_document document;
  pugi::xml_node rootElement = document.append_child("cmb-resources");

  // Construct xml element for each resource
  std::vector<std::string> resourceIds = resources.resourceIds();
  smtk::common::Resource::Type rtype;
  std::string rtypeString;
  ResourceSet::ResourceState state;
  ResourceSet::ResourceRole role;
  std::string link;
  bool ok = true;
  for (unsigned i = 0; i < resourceIds.size() && ok; ++i)
  {
    std::string id = resourceIds[i];
    ok = resources.resourceInfo(id, rtype, role, state, link);
    if (!ok)
    {
      std::cerr << "Error retrieving" << std::endl;
      continue;
    }

    rtypeString = smtk::common::Resource::type2String(rtype);
    pugi::xml_node resourceElement = rootElement.append_child(rtypeString.c_str());
    resourceElement.append_attribute("id").set_value(id.c_str());

    if (role != ResourceSet::NOT_DEFINED)
    {
      std::string rstring = ResourceSet::role2String(role);
      resourceElement.append_attribute("role").set_value(rstring.c_str());
    }

    if ((("" == link) || (EXPAND_LINKED_FILES == option)) && ResourceSet::LOADED == state)
    {
      // Use XmlStringWriter to generate xml for this attribute system
      smtk::common::ResourcePtr resource;
      ok = resources.get(id, resource);
      smtk::attribute::System* system = dynamic_cast<smtk::attribute::System*>(resource.get());

      AttributeWriter attWriter;
      // Get the default string writer instance
      // Could consider allowing application to assign version number
      XmlStringWriter* xmlWriter = attWriter.newXmlStringWriter(*system);
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
        smtk::common::ResourcePtr resource;
        ok = resources.get(id, resource);
        smtk::attribute::System* system = dynamic_cast<smtk::attribute::System*>(resource.get());
        AttributeWriter attWriter;
        bool hasErr = attWriter.write(*system, link, logger);
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
