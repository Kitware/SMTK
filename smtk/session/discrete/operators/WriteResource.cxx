//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/discrete/operators/WriteResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/session/discrete/Resource.h"
#include "smtk/session/discrete/WriteResource_xml.h"
#include "smtk/session/discrete/operators/WriteOperation.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace discrete
{

bool WriteResource::ableToOperate()
{
  if (!this->smtk::operation::XMLOperation::ableToOperate())
  {
    return false;
  }

  if (this->parameters()->associations()->numberOfValues() < 1)
  {
    return false;
  }

  return true;
}

WriteResource::Result WriteResource::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::session::discrete::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::discrete::Resource>(resourceItem->objectValue());

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
        smtk::session::discrete::WriteOperation::Ptr writeOp =
          smtk::session::discrete::WriteOperation::create();

        writeOp->parameters()->findFile("filename")->setValue(nativemodelfile);

        // Set the entity association
        writeOp->parameters()->associateEntity(model);

        // Execute the operation
        smtk::operation::Operation::Result writeOpResult = writeOp->operate(Key());

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

void WriteResource::markModifiedResources(WriteResource::Result& /*unused*/)
{
  auto resourceItem = this->parameters()->associations();
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }
}

bool writeResource(const smtk::resource::ResourcePtr& resource)
{
  WriteResource::Ptr write = WriteResource::create();
  write->parameters()->associate(resource);
  WriteResource::Result result = write->operate();
  return (
    result->findInt("outcome")->value() == static_cast<int>(WriteResource::Outcome::SUCCEEDED));
}

} // namespace discrete
} // namespace session
} // namespace smtk
