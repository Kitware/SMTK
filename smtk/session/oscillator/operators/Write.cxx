//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/oscillator/operators/Write.h"

#include "smtk/session/oscillator/Resource.h"
#include "smtk/session/oscillator/operators/Write_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/json/jsonResource.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace oscillator
{

bool Write::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  if (this->parameters()->associations()->numberOfValues() < 1)
  {
    return false;
  }

  return true;
}

Write::Result Write::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::session::oscillator::Resource::Ptr rsrc =
    std::dynamic_pointer_cast<smtk::session::oscillator::Resource>(resourceItem->value());

  // Serialize resource into a set of JSON records:
  smtk::model::SessionIOJSON::json j = rsrc;

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::string fileDirectory = smtk::common::Paths::directory(rsrc->location()) + "/";

  // Write JSON records to the specified URL:
  bool ok = smtk::model::SessionIOJSON::saveModelRecords(j, rsrc->location());

  return ok ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
            : this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

void Write::markModifiedResources(Write::Result& /*unused*/)
{
  auto resourceItem = this->parameters()->associations();
  for (auto rit = resourceItem->begin(); rit != resourceItem->end(); ++rit)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(*rit);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }
}

bool write(
  const smtk::resource::ResourcePtr& resource,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Write::Ptr write = Write::create();
  write->setManagers(managers);
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}

} // namespace oscillator
} // namespace session
} // namespace smtk
