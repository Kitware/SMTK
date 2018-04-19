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

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/SessionIOJSON.h"

#include "smtk/bridge/polygon/Write_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
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

  smtk::bridge::polygon::Resource::Ptr rsrc =
    std::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(resourceItem->objectValue());

  // Serialize resource into a set of JSON records:
  smtk::bridge::polygon::SessionIOJSON::json j =
    smtk::bridge::polygon::SessionIOJSON::saveJSON(rsrc);

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Write JSON records to the specified URL:
  bool ok = smtk::bridge::polygon::SessionIOJSON::saveModelRecords(j, rsrc->location());

  return ok ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
            : this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

bool write(const smtk::resource::ResourcePtr& resource)
{
  Write::Ptr write = Write::create();
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}

} // namespace polygon
} // namespace bridge
} // namespace smtk
