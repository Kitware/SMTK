//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Import.h"

#include "smtk/attribute/Import_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/Paths.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace attribute
{

Import::Result Import::operateInternal()
{
  // Access the file name.
  std::string filename = this->parameters()->findFile("filename")->value();

  // Check whether or not to use directory info
  smtk::attribute::VoidItem::Ptr directoryInfoItem =
    this->parameters()->findVoid("UseDirectoryInfo");
  bool useDirectoryInfo = directoryInfoItem->isEnabled();

  smtk::attribute::ResourcePtr resource;
  // Check if attribute resource is specified
  smtk::attribute::ReferenceItem::Ptr existingResourceItem = this->parameters()->associations();
  if (existingResourceItem->numberOfValues() > 0)
  {
    resource = std::static_pointer_cast<smtk::attribute::Resource>(existingResourceItem->value());
  }
  else
  {
    // Create an attribute resource. If available, use the resource manager to
    // create the resource and add logic to handle custom items to it.
    if (auto resourceManager = this->resourceManager())
    {
      resource = resourceManager->create<smtk::attribute::Resource>();
    }
    else
    {
      resource = smtk::attribute::Resource::create();
    }
    auto name = smtk::common::Paths::stem(filename);
    resource->setName(name);
  }

  // Populate the attribute resource with the contents of the file.
  smtk::io::AttributeReader reader;
  if (reader.read(resource, filename, useDirectoryInfo, log()))
  {
    smtkErrorMacro(log(), "Encountered errors while reading \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

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

const char* Import::xmlDescription() const
{
  return Import_xml;
}
}
}
