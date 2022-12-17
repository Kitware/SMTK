//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/SetMeshName.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/operators/SetMeshName_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::SetMeshName::Result SetMeshName::operateInternal()
{
  // Access the name to use when renaming the meshset
  smtk::attribute::StringItem::Ptr nameItem = this->parameters()->findString("name");

  // Access the meshset to rename
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>();
  smtk::mesh::MeshSet meshset = meshComponent->mesh();

  // Set the name of the meshset
  bool success = meshset.setName(nameItem->value());

  if (!success)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Create a new result
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with modified components
  result->findComponent("modified")->appendValue(meshComponent);

  // Return with success
  return result;
}

const char* SetMeshName::xmlDescription() const
{
  return SetMeshName_xml;
}

} //namespace mesh
} // namespace smtk
