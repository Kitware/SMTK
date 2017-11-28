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

#include "smtk/io/Logger.h"

#include "smtk/model/AssignColors_xml.h"

#include <cstddef> // for size_t

using smtk::attribute::StringItem;

namespace smtk
{
namespace model
{

AssignColors::Result AssignColors::operateInternal()
{
  std::vector<FloatList> colors;
  StringItem::Ptr colorSpec = this->parameters()->findString("colors");
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
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  auto associations = this->parameters()->associations();
  EntityRefArray entities(associations->begin(), associations->end());
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
      // TODO: Add support for remove color assigned by entity list. For now if
      // user assign color by entityListPhrase, there is no way to remove it
      // (unless manually set it to white).
      if (ent.isValid())
      {
        ent.setColor(0, 0, 0, -1);
        modified.push_back(ent);
      }
    }
  }

  Result result = this->createResult(smtk::operation::NewOp::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
  for (auto& m : modified)
  {
    modifiedItem->appendValue(m.component());
  }

  return result;
}

const char* AssignColors::xmlDescription() const
{
  return AssignColors_xml;
}

} //namespace model
} // namespace smtk
