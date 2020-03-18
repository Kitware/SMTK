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
  auto resourceItem = this->parameters()->findResource("use-resource");
  if ((resourceItem != nullptr) && resourceItem->isEnabled())
  {
    resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceItem->value(0));
    if (resource == nullptr)
    {
      smtkErrorMacro(log(), "Failed to find specified attribute resource");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  else
  {
    // Construct an attribute resource to populate.
    resource = smtk::attribute::Resource::create();
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
