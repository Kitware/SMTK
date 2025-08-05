//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/DeleteAttribute.h"

#include "smtk/attribute/operators/DeleteAttribute_xml.h"

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

DeleteAttribute::Result DeleteAttribute::operateInternal()
{
  auto params = this->parameters();
  int numFailed = 0;
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto expunged = result->findComponent("expunged");
  smtk::operation::MarkGeometry marker;
  for (const auto& obj : *params->associations())
  {
    if (auto attribute = std::dynamic_pointer_cast<smtk::attribute::Attribute>(obj))
    {
      auto rsrc = attribute->attributeResource();
      auto& geom = rsrc->geometry();
      if (!rsrc->removeAttribute(attribute))
      {
        smtkErrorMacro(
          this->log(),
          "Could not delete attribute " << attribute->name() << " of type \"" << attribute->type()
                                        << "\".");
        ++numFailed;
      }
      else
      {
        expunged->appendValue(attribute);
        // Force any geometry on the created attribute to be updated.
        // Only empty the cache entry if there is a geometry backend
        // for the resource.
        if (geom)
        {
          marker.markModified(attribute);
        }
      }
    }
  }

  if (numFailed > 0)
  {
    setOutcome(result, smtk::operation::Operation::Outcome::FAILED);
  }

  return result;
}

void DeleteAttribute::generateSummary(Operation::Result& /*unused*/) {}

const char* DeleteAttribute::xmlDescription() const
{
  return DeleteAttribute_xml;
}
} // namespace attribute
} // namespace smtk
