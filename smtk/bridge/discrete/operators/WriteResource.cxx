//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/operators/WriteResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/WriteResource_xml.h"
#include "smtk/bridge/discrete/operators/WriteOperation.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

WriteResource::Result WriteResource::operateInternal()
{
  smtk::attribute::ResourceItem::Ptr resourceItem = this->parameters()->findResource("resource");

  smtk::bridge::discrete::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::bridge::discrete::Resource>(resourceItem->value());

  // Serialize resource into a set of JSON records:
  smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::saveJSON(resource);
  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  // Write JSON records to the specified URL:
  bool ok = smtk::model::SessionIOJSON::saveModelRecords(j, resource->location());

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
        smtk::bridge::discrete::WriteOperation::Ptr writeOp =
          smtk::bridge::discrete::WriteOperation::create();

        writeOp->parameters()->findFile("filename")->setValue(nativemodelfile);

        // Set the entity association
        writeOp->parameters()->associateEntity(model);

        // Execute the operation
        smtk::operation::Operation::Result writeOpResult = writeOp->operate();

        // Test for success
        ok &= (writeOpResult->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      }
    }
  }

  // Test for success
  return (ok) ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
              : this->createResult(smtk::operation::Operation::Outcome::FAILED);
}

const char* WriteResource::xmlDescription() const
{
  return WriteResource_xml;
}
} // namespace discrete
} // namespace bridge
} // namespace smtk
