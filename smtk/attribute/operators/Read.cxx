//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Read.h"

#include "smtk/attribute/Read_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/attribute/json/jsonResource.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

namespace smtk
{
namespace attribute
{

Read::Result Read::operateInternal()
{
  // Access the file name.
  std::string filename = this->parameters()->findFile("filename")->value();

  // Check the file's validity.
  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(log(), "Cannot read file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Read the file into nlohmann json.
  nlohmann::json j;
  try
  {
    j = nlohmann::json::parse(file);
  }
  catch (...)
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // is the a supported version?  Should be 3.0 or 4.0
  auto version = j.find("version");
  if ((version == j.end()) || !((*version == "3.0") || (*version == "4.0")))
  {
    if (version == j.end())
    {
      smtkErrorMacro(log(), "Cannot read attribute file \"" << filename << "\" - Missing Version.");
    }
    else
    {
      smtkErrorMacro(log(), "Cannot read attribute file \""
          << filename << "\" - Unsupported Version: " << *version << ".");
    }
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Create an attribute resource. If available, use the resource manager to
  // create the resource and add logic to handle custom items to it.
  smtk::attribute::Resource::Ptr resource;
  if (auto resourceManager = this->resourceManager())
  {
    resource = resourceManager->create<smtk::attribute::Resource>();
  }
  else
  {
    resource = smtk::attribute::Resource::create();
  }

  // Copy the contents of the json object into the attribute resource.
  smtk::attribute::from_json(j, resource);
  resource->setLocation(filename);

  // If the resource manager was used to create the resource, then the resource
  // will have already been added to the manager. Now that the resource's
  // contents are read into memory, manually resolve its surrogates.
  if (auto resourceManager = this->resourceManager())
  {
    for (auto& rsrc : resourceManager->resources())
    {
      resource->links().resolve(rsrc);
      rsrc->links().resolve(resource);
    }
  }

  // Create a result object.
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Populate the result object with the new attribute resource.
  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  // Return with success.
  return result;
}

const char* Read::xmlDescription() const
{
  return Read_xml;
}

void Read::markModifiedResources(Read::Result& res)
{
  auto resourceItem = res->findResource("resource");
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);
    if (resource != nullptr)
    {
      // Set the resource as unmodified from its persistent (i.e. on-disk) state
      resource->setClean(true);
    }
  }
}

smtk::resource::ResourcePtr read(const std::string& filename)
{
  Read::Ptr read = Read::create();
  read->parameters()->findFile("filename")->setValue(filename);
  Read::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Read::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}
}
}
