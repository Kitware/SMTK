//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/json/jsonResource.h"

#include "smtk/resource/json/jsonLinkBase.h"

#include "smtk/common/json/jsonLinks.h"
#include "smtk/common/json/jsonUUID.h"

// Define how resources are serialized.
namespace smtk
{
namespace resource
{
void to_json(json& j, const ResourcePtr& resource)
{
  j["id"] = resource->id().toString();
  j["version"] = "3.0";
  j["type"] = resource->typeName();
  j["links"] = resource->links().data();
  if (resource->isNameSet())
  {
    j["name"] = resource->name();
  }
}

void from_json(const json& j, ResourcePtr& resource)
{
  // For backwards compatibility, do not require "id" json item.
  if (j.find("id") != j.end())
  {
    resource->setId(j.at("id"));
  }

  // For backwards compatibility, do not require "links" json item.
  if (j.find("links") != j.end())
  {
    resource->links().data() = j.at("links");
  }

  if (j.find("name") != j.end())
  {
    resource->setName(j.at("name"));
  }
}
}
}
