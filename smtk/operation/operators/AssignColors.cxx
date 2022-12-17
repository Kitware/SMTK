//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/AssignColors.h"

#include "smtk/operation/Hints.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Properties.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/Color.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/operators/AssignColors_xml.h"

#include <cstddef> // for size_t

using smtk::attribute::DoubleItem;
using smtk::attribute::StringItem;
using smtk::resource::PersistentObjectPtr;
using FloatList = std::vector<double>;

namespace smtk
{
namespace operation
{

AssignColors::Result AssignColors::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto entities = associations->as<std::set<PersistentObjectPtr>>();

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  std::set<smtk::resource::PersistentObject::Ptr> modified;
  // auto modified = result->findComponent("result");

  // I. Set opacities
  DoubleItem::Ptr opacitySpec = this->parameters()->findDouble("opacity");
  double opacity = opacitySpec->isEnabled() ? opacitySpec->value() : -1.0;
  if (opacity >= 0.0)
  {
    for (const auto& entity : entities)
    {
      bool skip = false;
      // If we are setting the opacity on an entity with no
      // existing color, assign it to be white by default:
      FloatList color{ 1., 1., 1., opacity };
      if (entity->properties().contains<FloatList>("color"))
      {
        color = entity->properties().at<FloatList>("color");
        skip = (color[3] == opacity);
        color[3] = opacity;
      }
      if (!skip)
      {
        entity->properties().get<FloatList>()["color"] = color;
        modified.insert(entity);
      }
    }
  }

  // II. Set colors
  std::vector<FloatList> colors;
  StringItem::Ptr colorSpec = this->parameters()->findString("colors");
  size_t numColors = colorSpec->isEnabled() ? colorSpec->numberOfValues() : 0;

  if (numColors > 0)
  {
    colors.reserve(numColors);
    for (size_t cc = 0; cc < numColors; ++cc)
    {
      FloatList rgba;
      if (smtk::common::Color::stringToFloatRGBA(
            rgba, colorSpec->value(cc), opacity >= 0.0 ? opacity : 1.0))
      {
        colors.push_back(rgba);
      }
      else
      {
        smtkWarningMacro(
          this->log(),
          "Color " << cc << " (" << colorSpec->value(cc) << ") could not be parsed. Skipping.");
      }
    }
  }

  if (numColors > 0 && colors.empty())
  { // someone tried to specify colors, but failed.
    smtkErrorMacro(this->log(), "No valid colors to assign.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  if (numColors > 0)
  {
    size_t cc = 0;
    numColors = colors.size();
    for (const auto& entity : entities)
    {
      if (entity)
      {
        entity->properties().get<FloatList>()["color"] = colors[cc % numColors];
        modified.insert(entity);
        ++cc;
      }
    }
  }
  else if (colorSpec->isEnabled())
  { // remove
    for (const auto& entity : entities)
    {
      if (entity && entity->properties().contains<FloatList>("color"))
      {
        entity->properties().erase<FloatList>("color");
        modified.insert(entity);
      }
    }
  }

  auto modifiedItem = result->findComponent("modified");
  for (const auto& mm : modified)
  {
    if (auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(mm))
    {
      modifiedItem->appendValue(comp);
    }
    else if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(mm))
    {
      // TODO
    }
  }

  // Add a hint to reset the selection so the assigned colors get shown.
  // If run from the operation parameter-editor, the same components can
  // be recolored without the selection being affected.
  smtk::operation::addSelectionHint(
    result,
    std::set<smtk::resource::Component::Ptr>{},
    smtk::view::SelectionAction::UNFILTERED_REPLACE,
    1);

  return result;
}

const char* AssignColors::xmlDescription() const
{
  return AssignColors_xml;
}

} //namespace operation
} // namespace smtk
