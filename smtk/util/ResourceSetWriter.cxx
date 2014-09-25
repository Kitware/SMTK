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

#include "ResourceSetWriter.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"
#include "smtk/attribute/Manager.h"
#include "smtk/util/AttributeWriter.h"
#include "smtk/util/XmlV2StringWriter.h"

#include <fstream>
#include <iostream>
#include <sstream>


using namespace smtk::util;


//----------------------------------------------------------------------------
bool
ResourceSetWriter::
writeFile(std::string filename,
          const ResourceSet& resources,
          smtk::util::Logger& logger,
          bool writeLinkedFiles)
{
  logger.reset();
  std::string content;
  this->writeString(content, resources, logger, writeLinkedFiles);
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

//----------------------------------------------------------------------------
bool
ResourceSetWriter::
writeString(std::string& content,
            const ResourceSet& resources,
            smtk::util::Logger& logger,
            bool writeLinkedFiles)
{
  logger.reset();

  pugi::xml_document document;
  pugi::xml_node rootElement = document.append_child("cmb-resources");

  // Construct xml element for each resource
  std::vector<std::string> resourceIds = resources.resourceIds();
  smtk::util::Resource::Type rtype;
  std::string rtypeString;
  ResourceSet::ResourceState state;
  ResourceSet::ResourceRole role;
  std::string link;
  bool ok = true;
  for (unsigned i=0; i<resourceIds.size() && ok; ++i)
    {
    std::string id = resourceIds[i];
    ok = resources.resourceInfo(id, rtype, role, state, link);
    if (!ok)
      {
      std::cerr << "Error retrieving" << std::endl;
      continue;
      }

    rtypeString = smtk::util::Resource::type2String(rtype);
    pugi::xml_node resourceElement =
      rootElement.append_child(rtypeString.c_str());
    resourceElement.append_attribute("id").set_value(id.c_str());

    if (role != ResourceSet::NOT_DEFINED)
      {
      std::string rstring = ResourceSet::role2String(role);
      resourceElement.append_attribute("role").set_value(rstring.c_str());
      }

    if ("" == link && ResourceSet::LOADED == state)
      {
      // Use XmlV2StringWriter to generate xml for this attribute manager
      smtk::util::ResourcePtr resource;
      ok = resources.get(id, resource);
      smtk::attribute::Manager *manager =
        dynamic_cast<smtk::attribute::Manager *>(resource.get());
      XmlV2StringWriter xmlWriter(*manager);
      xmlWriter.generateXml(resourceElement, logger);
      }
    else if (link != "")
      {
      // If resource is linked, insert <include> element
      pugi::xml_node includeElement = resourceElement.append_child("include");
      includeElement.append_attribute("href").set_value(link.c_str());

      // Save linked resources if option selected
      if (writeLinkedFiles)
        {
        smtk::util::ResourcePtr resource;
        ok = resources.get(id, resource);
        smtk::attribute::Manager *manager =
          dynamic_cast<smtk::attribute::Manager *>(resource.get());
        AttributeWriter attWriter;
        bool hasErr = attWriter.write(*manager, link, logger);
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

//----------------------------------------------------------------------------
