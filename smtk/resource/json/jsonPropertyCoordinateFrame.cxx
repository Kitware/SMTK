//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/properties/CoordinateFrame.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"
#include "smtk/resource/json/jsonPropertyCoordinateFrame.h"

#include "smtk/io/Logger.h"

#include <iostream>

// Define how CoordinateFrame is serialized.
namespace smtk
{
namespace resource
{
namespace properties
{

SMTKCORE_EXPORT void to_json(json& j, const smtk::resource::properties::CoordinateFrame& frame)
{
  j["Origin"] = frame.origin;
  j["XAxis"] = frame.xAxis;
  j["YAxis"] = frame.yAxis;
  j["ZAxis"] = frame.zAxis;
  if (!frame.parent.isNull())
  {
    j["Parent"] = frame.parent;
  }
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::resource::properties::CoordinateFrame& frame)
{
  if (j.find("Origin") != j.end())
  {
    frame.origin = j.at("Origin");
  }
  if (j.find("XAxis") != j.end())
  {
    frame.xAxis = j.at("XAxis");
  }
  if (j.find("YAxis") != j.end())
  {
    frame.yAxis = j.at("YAxis");
  }
  if (j.find("ZAxis") != j.end())
  {
    frame.zAxis = j.at("ZAxis");
  }
  if (j.find("Parent") != j.end())
  {
    frame.parent = j.at("Parent");
    // smtk::resource::json::Helper::instance().finishFrame(&frame, uid);
    /*
    auto* resource = smtk::resource::json::Helper::instance().resource();
    if (!uid.isNull() && resource)
    {
      if (resource->id() == uid)
      {
        frame.parent = resource->shared_from_this();
      }
      else
      {
        auto comp = resource->find(uid);
        if (comp)
        {
          frame.parent = comp;
        }
      }
    }
    if (!frame.parent.lock())
    {
      // TODO: search in resource manager (not currently available) for the proper object.
      smtkErrorMacro(smtk::io::Logger::instance(),
        "Unable to locate coordinate-frame parent \"" <<
        j.at("Parent").get<std::string>() << "\".");
    }
    */
  }
}

} // namespace properties
} // namespace resource
} // namespace smtk
