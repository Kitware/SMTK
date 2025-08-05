//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/CreateAttribute.h"

#include "smtk/attribute/operators/CreateAttribute_xml.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/geometry/Geometry.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace attribute
{

CreateAttribute::Result CreateAttribute::operateInternal()
{
  auto params = this->parameters();
  auto resource = params->associations()->valueAs<smtk::attribute::Resource>();
  if (!resource)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  auto attribute = resource->createAttribute(params->findString("definition")->value());
  if (!attribute)
  {
    smtkErrorMacro(
      this->log(),
      "Could not create an attribute of type \"" << params->findString("definition")->value()
                                                 << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto created = result->findComponent("created");
  created->appendValue(attribute);

  // Force any geometry on the created attribute to be updated.
  smtk::operation::MarkGeometry marker;
  // Only empty the cache entry if there is a geometry backend
  // for the resource.
  auto& geom = resource->geometry();
  if (geom)
  {
    marker.markModified(attribute);
  }

  return result;
}

void CreateAttribute::generateSummary(Operation::Result& /*unused*/) {}

const char* CreateAttribute::xmlDescription() const
{
  return CreateAttribute_xml;
}
} // namespace attribute
} // namespace smtk
