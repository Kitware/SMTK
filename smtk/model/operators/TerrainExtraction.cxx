//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/TerrainExtraction.h"

#include "smtk/model/EntityRef.h"

#include "smtk/common/Color.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/io/Logger.h"

#include "smtk/model/operators/TerrainExtraction_xml.h"

namespace smtk
{
namespace model
{

TerrainExtraction::Result TerrainExtraction::operateInternal()
{
  auto associations = this->parameters()->associations();

  if (!associations || associations->numberOfValues() == 0)
  {
    smtkErrorMacro(this->log(), "No parent specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Hide the visibilty of input aux_geom
  EntityRef parent(associations->valueAs<smtk::model::Entity>());
  parent.setVisible(false);
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
  modifiedItem->appendValue(parent.component());

  return result;
}

const char* TerrainExtraction::xmlDescription() const
{
  return TerrainExtraction_xml;
}

} //namespace model
} // namespace smtk
