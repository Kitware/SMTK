//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "Write.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/SessionIOJSON.h"

#include "smtk/session/polygon/operators/Write_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace polygon
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

  smtk::session::polygon::Resource::Ptr rsrc =
    std::dynamic_pointer_cast<smtk::session::polygon::Resource>(resourceItem->value());

  // Serialize resource into a set of JSON records:
  smtk::session::polygon::SessionIOJSON::json j =
    smtk::session::polygon::SessionIOJSON::saveJSON(rsrc);

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Write JSON records to the specified URL:
  bool ok = smtk::session::polygon::SessionIOJSON::saveModelRecords(j, rsrc->location());

  return ok ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
            : this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Write::xmlDescription() const
{
  return Write_xml;
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

} // namespace polygon
} // namespace session
} // namespace smtk
