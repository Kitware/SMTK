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

#include "smtk/bridge/vtk/Resource.h"
#include "smtk/bridge/vtk/Write_xml.h"
#include "smtk/bridge/vtk/json/jsonResource.h"

#include "smtk/model/SessionIOJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace vtk
{

Write::Result Write::operateInternal()
{
  smtk::attribute::ResourceItem::Ptr resourceItem = this->parameters()->findResource("resource");

  smtk::bridge::vtk::Resource::Ptr rsrc =
    std::dynamic_pointer_cast<smtk::bridge::vtk::Resource>(resourceItem->value());

  // Serialize resource into a set of JSON records:
  smtk::model::SessionIOJSON::json j = rsrc;

  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Write JSON records to the specified URL:
  bool ok = smtk::model::SessionIOJSON::saveModelRecords(j, rsrc->location());

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
  write->parameters()->findResource("resource")->setValue(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}

} // namespace vtk
} // namespace bridge
} // namespace smtk
