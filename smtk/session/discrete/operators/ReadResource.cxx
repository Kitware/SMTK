//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/discrete/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/session/discrete/Resource.h"

#include "smtk/session/discrete/ReadResource_xml.h"
#include "smtk/session/discrete/operators/ReadOperation.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace session
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
  auto resource = smtk::session::discrete::Resource::create();
  smtk::model::SessionIOJSON::loadModelRecords(j, resource);

  // Access the model associated with this resource
  smtk::model::Models models =
    resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  for (auto& model : models)
  {
    const smtk::common::UUID& modelid = model.entity();

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
        smtk::session::discrete::ReadOperation::Ptr readOp =
          smtk::session::discrete::ReadOperation::create();

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

void ReadResource::markModifiedResources(ReadResource::Result& res)
{
  auto resourceItem = res->findResource("resource");
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }
}

smtk::resource::ResourcePtr readResource(const std::string& filename)
{
  ReadResource::Ptr read = ReadResource::create();
  read->parameters()->findFile("filename")->setValue(filename);
  ReadResource::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(ReadResource::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}
}
}
}
