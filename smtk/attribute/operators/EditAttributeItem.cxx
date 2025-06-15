//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/EditAttributeItem.h"

#include "smtk/attribute/operators/EditAttributeItem_xml.h"

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

EditAttributeItem::Result EditAttributeItem::operateInternal()
{
  bool didModify = false;
  auto params = this->parameters();
  auto attrib = params->associations()->valueAs<smtk::attribute::Attribute>();
  if (!attrib)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  auto item = attrib->itemAtPath(params->findString("item path")->value());
  if (!item)
  {
    smtkErrorMacro(
      this->log(), "No item at path \"" << params->findString("item path")->value() << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  auto enableItem = params->findInt("enable");
  int enable = enableItem->isEnabled() ? enableItem->value() : -1;
  if (enable >= 0)
  {
    if (!item->isOptional())
    {
      smtkErrorMacro(this->log(), "Cannot enable/disable a non-optional item.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    int current = item->isEnabled() ? 1 : 0;
    if (current != enable)
    {
      item->setIsEnabled(enable);
      didModify = true;
    }
    // else: It is not an error to request a non-change. Silently ignore.
  }

  int extend = params->findInt("extend")->value();
  if (extend >= 0)
  {
    auto vitem = std::dynamic_pointer_cast<ValueItem>(item);
    if (!vitem)
    {
      // TODO: Handle ReferenceItem.
      smtkErrorMacro(this->log(), "Cannot extend an item of type \"" << item->typeName() << "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    if (!vitem->setNumberOfValues(extend))
    {
      smtkErrorMacro(this->log(), "Cannot extend item to length " << extend << ".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    didModify = true;
  }

  auto valueItem = params->findString("value");
  if (valueItem->numberOfValues() > 0)
  {
    auto vitem = std::dynamic_pointer_cast<ValueItem>(item);
    if (!vitem)
    {
      // TODO: Handle ReferenceItem.
      smtkErrorMacro(this->log(), "Cannot extend an item of type \"" << item->typeName() << "\".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    if (vitem->numberOfValues() != valueItem->numberOfValues())
    {
      smtkErrorMacro(
        this->log(),
        "Cannot set values with mismatching lengths " << vitem->numberOfValues() << " vs "
                                                      << valueItem->numberOfValues() << ".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    std::size_t numVals = vitem->numberOfValues();
    for (std::size_t ii = 0; ii < numVals; ++ii)
    {
      if (vitem->valueAsString(ii) != valueItem->value(ii))
      {
        if (!vitem->setValueFromString(ii, valueItem->value(ii)))
        {
          smtkErrorMacro(
            this->log(), "Cannot set value " << ii << " to " << valueItem->value(ii) << ".");
          return this->createResult(smtk::operation::Operation::Outcome::FAILED);
        }
        didModify = true;
      }
    }
  }
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modOut = result->findComponent("modified");
  if (didModify)
  {
    modOut->appendValue(attrib);

    // Force any geometry on the modified attribute to be updated.
    smtk::operation::MarkGeometry marker;
    auto* rsrc = dynamic_cast<smtk::geometry::Resource*>(attrib->parentResource());
    if (rsrc)
    {
      // Only empty the cache entry if there is a geometry backend
      // for the resource.
      auto& geom = rsrc->geometry();
      if (geom)
      {
        marker.markModified(attrib);
      }
    }
  }

  return result;
}

void EditAttributeItem::generateSummary(Operation::Result& /*unused*/) {}

const char* EditAttributeItem::xmlDescription() const
{
  return EditAttributeItem_xml;
}
} // namespace attribute
} // namespace smtk
