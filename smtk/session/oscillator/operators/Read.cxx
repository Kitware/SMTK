//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/oscillator/operators/Read.h"

#include "smtk/session/oscillator/Resource.h"
#include "smtk/session/oscillator/operators/Read_xml.h"

#include "smtk/model/Model.h"
#include "smtk/model/Volume.h"

#include "smtk/model/json/jsonResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

#include <fstream>

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace oscillator
{

Read::Result Read::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(log(), "Cannot read file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Load file and parse it:
  smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
  if (j.is_null())
  {
    smtkErrorMacro(log(), "Cannot parse file \"" << filename << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::string resourceIdStr = j.at("id");
  smtk::common::UUID resourceId(resourceIdStr);

  // Create a new resource for the import
  auto resource = smtk::session::oscillator::Resource::create();
  resource->setId(resourceId);
  resource->setLocation(filename);

  // Transcribe model data onto the resource
  auto modelResource = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::from_json(j, modelResource);

  auto volumes = resource->entitiesMatchingFlagsAs<smtk::model::Volumes>(
    smtk::model::VOLUME, /*exactMatch*/ true);
  for (auto volume : volumes)
  {
    resource->resetDomainTessellation(volume);
  }

  resource->setLocation(filename);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resource);
  }

  return result;
}

const char* Read::xmlDescription() const
{
  return Read_xml;
}

void Read::markModifiedResources(Read::Result& res)
{
  auto resourceItem = res->findResource("resourcesCreated");
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }
}

smtk::resource::ResourcePtr read(
  const std::string& filename,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Read::Ptr read = Read::create();
  read->setManagers(managers);
  read->parameters()->findFile("filename")->setValue(filename);
  Read::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Read::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resourcesCreated")->value();
}

} // namespace oscillator
} // namespace session
} // namespace smtk
