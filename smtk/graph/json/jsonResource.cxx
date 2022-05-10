//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/json/jsonResource.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/graph/json/ArcDeserializer.h"
#include "smtk/graph/json/ArcSerializer.h"

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
  j["location"] = resource->location();
  j["links"] = resource->links().data();
  j["properties"] = resource->properties().data();
  if (resource->isNameSet())
  {
    j["name"] = resource->name();
  }

  // Only include arcs if 1 or more arc containers request it.
  json arcData;
  resource->evaluateArcs<ArcSerializer>(arcData);
  if (!arcData.empty())
  {
    j["arcs"] = arcData;
  }

  // TODO: Only include nodes if requested by the node container.
}

void from_json(const json& j, ResourcePtr& resource)
{
  // For backwards compatibility, do not require "id" json item.
  if (j.find("id") != j.end())
  {
    resource->setId(j.at("id"));
  }

  // For backwards compatibility, do not require "location" json item.
  if (j.find("location") != j.end())
  {
    resource->setLocation(j.at("location"));
  }

  // For backwards compatibility, do not require "links" json item.
  if (j.find("links") != j.end())
  {
    resource->links().data() = j.at("links");
  }

  // For backwards compatibility, do not require "properties" json item.
  if (j.find("properties") != j.end())
  {
    // Call the serialization method directly so the custom constructor for
    // m_data in smtk::resource::Properties::Properties() is used.
    smtk::common::from_json(j.at("properties"), resource->properties().data());
  }

  if (j.find("name") != j.end())
  {
    resource->setName(j.at("name"));
  }

#if 0
  // If any arcs have been serialized, deserialize them.
  if (j.find("arcs") != j.end())
  {
    resource->evaluateArcs<ArcDeserializer>(j.at("arcs"));
  }
#endif
}
} // namespace resource
} // namespace smtk
