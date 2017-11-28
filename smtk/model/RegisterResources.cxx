//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/model/RegisterResources.h"

#include "smtk/model/operators/LoadSMTKModel.h"
#include "smtk/model/operators/SaveSMTKModel.h"

#include "smtk/model/Manager.h"
#include "smtk/model/SessionIOJSON.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace smtk
{
namespace model
{

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::model::Manager>(
    // ### Load a model file ###
    [](const std::string& filename) -> smtk::resource::ResourcePtr {
      smtk::resource::ResourcePtr result;
      // Load file and parse it:
      json j = smtk::model::SessionIOJSON::loadJSON(filename);
      if (j.is_null())
      {
        return result;
      }

      // Deserialized parsed JSON into a model resource:
      auto rsrc = smtk::model::Manager::create();
      result = rsrc;
      smtk::model::SessionIOJSON::loadModelRecords(j, rsrc);
      return result;
    },

    // ### Save a model file ###
    [](const smtk::resource::ResourcePtr& rsrc) -> bool {
      // Serialize rsrc into a set of JSON records:
      json j =
        smtk::model::SessionIOJSON::saveJSON(std::dynamic_pointer_cast<smtk::model::Manager>(rsrc));
      if (j.is_null())
      {
        return false;
      }
      // Write JSON records to the specified URL:
      bool ok = smtk::model::SessionIOJSON::saveModelRecords(j, rsrc->location());
      return ok;
    });
}
}
}
