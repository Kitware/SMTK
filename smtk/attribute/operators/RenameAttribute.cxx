//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/RenameAttribute.h"

#include "smtk/attribute/operators/RenameAttribute_xml.h"

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

RenameAttribute::Result RenameAttribute::operateInternal()
{
  auto params = this->parameters();
  auto attrItem = params->associations();
  auto nameItem = params->findString("name");
  if (attrItem->numberOfValues() != nameItem->numberOfValues())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::size_t nn = nameItem->numberOfValues();
  std::size_t numFailed = 0;
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto attribute = attrItem->valueAs<smtk::attribute::Attribute>(ii);
    if (!attribute)
    {
      continue;
    }
    if (!attribute->attributeResource()->rename(attribute, nameItem->value(ii)))
    {
      smtkErrorMacro(
        this->log(),
        "Failed to rename \"" << attribute->name() << "\" to \"" << nameItem->value(ii) << "\".");
      ++numFailed;
    }
    else
    {
      modified->appendValue(attribute);
    }
  }

  if (numFailed > 0)
  {
    smtk::operation::setOutcome(result, smtk::operation::Operation::Outcome::FAILED);
  }
  return result;
}

void RenameAttribute::generateSummary(Operation::Result& /*unused*/) {}

const char* RenameAttribute::xmlDescription() const
{
  return RenameAttribute_xml;
}
} // namespace attribute
} // namespace smtk
