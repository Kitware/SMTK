//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/bridge/discrete/Resource.h"

#include "smtk/bridge/discrete/ReadResource_xml.h"
#include "smtk/bridge/discrete/operators/ReadOperation.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

ReadResource::Result ReadResource::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  // Load file and parse it:
  smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Deserialize parsed JSON into a model resource:
  auto resource = smtk::bridge::discrete::Resource::create();
  smtk::model::SessionIOJSON::loadModelRecords(j, resource);

  // Access the model associated with this resource
  smtk::model::Models models =
    resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  for (auto& model : models)
  {
    smtk::common::UUID modelid = model.entity();

    std::string nativemodelfile;
    std::string nativefilekey = resource->hasStringProperty(modelid, "url") ? "url" : "";
    if (!nativefilekey.empty())
    {
      smtk::model::StringList const& nprop(resource->stringProperty(modelid, nativefilekey));
      if (!nprop.empty())
      {
        nativemodelfile = nprop[0];
      }

      if (!nativemodelfile.empty())
      {
        smtk::bridge::discrete::ReadOperation::Ptr readOp =
          smtk::bridge::discrete::ReadOperation::create();

        readOp->parameters()->findFile("filename")->setValue(nativemodelfile);

        // Set the entity association
        readOp->parameters()->associateEntity(model);

        // Execute the operation
        smtk::operation::Operation::Result readOpResult = readOp->operate();
      }
    }
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  return result;
}

const char* ReadResource::xmlDescription() const
{
  return ReadResource_xml;
}
}
}
}
