//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/AssignColors.h"

#include "smtk/model/EntityRef.h"

#include "smtk/common/Color.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include <cstddef> // for size_t

using smtk::attribute::StringItem;

namespace smtk
{
namespace model
{

smtk::model::OperatorResult AssignColors::operateInternal()
{
  std::vector<FloatList> colors;
  StringItem::Ptr colorSpec = this->findString("colors");
  size_t numColors = colorSpec->numberOfValues();

  if (numColors > 0)
  {
    colors.reserve(numColors);
    for (size_t cc = 0; cc < numColors; ++cc)
    {
      FloatList rgba;
      if (smtk::common::Color::stringToFloatRGBA(rgba, colorSpec->value(cc)))
      {
        colors.push_back(rgba);
      }
      else
      {
        smtkWarningMacro(this->log(), "Color " << cc << " (" << colorSpec->value(cc)
                                               << ") could not be parsed. Skipping.");
      }
    }
  }

  if (numColors > 0 && colors.empty())
  { // someone tried to specify colors, but failed.
    smtkErrorMacro(this->log(), "No valid colors to assign.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  EntityRefArray modified;
  if (numColors > 0)
  {
    size_t cc = 0;
    numColors = colors.size();
    for (auto ent : entities)
    {
      if (ent.isValid())
      {
        ent.setColor(colors[cc % numColors]);
        // TODO: Only mark modified if color changed; pre-existing color may have been identical.
        modified.push_back(ent);
        ++cc;
      }
    }
  }
  else
  { // remove (or at least invalidate) colors instead
    for (auto ent : entities)
    {
      // TODO: It would be nice to call ent.RemoveFloatProperty("color")
      //       here, but doing so does not remove the colors on the client.
      if (ent.isValid())
      {
        ent.setColor(0, 0, 0, -1);
        modified.push_back(ent);
      }
    }
  }

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  this->addEntitiesToResult(result, modified, MODIFIED);
  return result;
}

} //namespace model
} // namespace smtk

#include "smtk/model/AssignColors_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::AssignColors, assign_colors,
  "assign colors", AssignColors_xml, smtk::model::Session);
