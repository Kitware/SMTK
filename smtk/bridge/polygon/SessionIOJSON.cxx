//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/SessionIOJSON.h"

#include "smtk/bridge/polygon/json/jsonResource.h"

using json = nlohmann::json;

namespace smtk
{
namespace bridge
{
namespace polygon
{
json SessionIOJSON::saveJSON(const smtk::bridge::polygon::Resource::Ptr& rsrc)
{
  json result = rsrc;
  return result;
}

json SessionIOJSON::loadJSON(const std::string& filename)
{
  return smtk::model::SessionIOJSON::loadJSON(filename);
}

bool SessionIOJSON::loadModelRecords(const json& j, smtk::bridge::polygon::Resource::Ptr& rsrc)
{
  if (j.is_null())
  {
    return false;
  }

  smtk::bridge::polygon::from_json(j, rsrc);

  rsrc->setClean();
  return true;
}

} // namespace polygon
} // namespace bridge
} // namespace smtk
