//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/ExtractSkin.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/DimensionTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/mesh/operators/ExtractSkin_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::ExtractSkin::Result ExtractSkin::operateInternal()
{
  // Access the meshset
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>();
  smtk::mesh::MeshSet meshset = meshComponent->mesh();

  // Extract the skin
  smtk::mesh::MeshSet skin = meshset.extractShell();

  if (skin.is_empty())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  skin.setName(meshset.name() + " (skin)");

  smtk::mesh::Component::Ptr skinComponent = smtk::mesh::Component::create(skin);

  if (skinComponent == nullptr)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Create a new result
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with modified components
  result->findComponent("created")->appendValue(skinComponent);

  // Mark the created component as having a modified geometry so it will be
  // propertly rendered
  smtk::operation::MarkGeometry().markModified(skinComponent);

  // Return with success
  return result;
}

const char* ExtractSkin::xmlDescription() const
{
  return ExtractSkin_xml;
}
} // namespace mesh
} // namespace smtk
