//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Write.h"

#include "smtk/attribute/operators/Write_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/attribute/json/jsonResource.h"

#include "smtk/common/Paths.h"

#include "smtk/io/Logger.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

namespace smtk
{
namespace attribute
{

Write::Result Write::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::attribute::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceItem->value());

  // Serialize resource into a set of JSON records:
  nlohmann::json j;
  smtk::attribute::to_json(j, resource);

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  {
    // Ensure parent directory exists. It may not if we are part of a project
    // that has organized us into an unrealized subdirectory.
    auto location = resource->absoluteLocation(/*create directories*/ true);
    std::ofstream file(location);
    if (!file.good())
    {
      smtkErrorMacro(log(), "Unable to open \"" << resource->location() << "\" for writing.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    file << j.dump(2);
    file.close();
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
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

void Write::generateSummary(Result& res)
{
  if (smtk::operation::outcome(res) != Outcome::SUCCEEDED)
  {
    this->Superclass::generateSummary(res);
  }
  auto resourceItem = this->parameters()->associations();
  auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(resourceItem->value());
  smtkInfoMacro(this->log(), "Wrote \"" << resource->location() << "\".");
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
} // namespace attribute
} // namespace smtk
