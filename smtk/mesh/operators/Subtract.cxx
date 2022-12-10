//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/Subtract.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/mesh/operators/Subtract_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::Subtract::Result Subtract::operateInternal()
{
  // Access the minuend and its resource
  smtk::mesh::MeshSet minuend =
    this->parameters()->associations()->valueAs<smtk::mesh::Component>()->mesh();
  const smtk::mesh::Resource::Ptr& resource = minuend.resource();

  // Access the subtrahends
  smtk::attribute::ComponentItemPtr subtrahends = this->parameters()->findComponent("subtrahend");

  // For each subtrahend, subtract it from the minuend
  smtk::mesh::HandleRange differenceRange = minuend.cells().range();
  for (std::size_t i = 0; i < subtrahends->numberOfValues(); ++i)
  {
    differenceRange -= subtrahends->valueAs<smtk::mesh::Component>(i)->mesh().cells().range();
  }

  // Create a new result
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::mesh::MeshSet difference =
    resource->createMesh(smtk::mesh::CellSet(resource, differenceRange));
  difference.setName("difference");

  smtk::mesh::Component::Ptr differenceComponent = smtk::mesh::Component::create(difference);

  // Access the attribute associated with modified components
  result->findComponent("created")->appendValue(differenceComponent);

  // Mark the minuend and difference as having a modified geometry.
  smtk::operation::MarkGeometry().markModified(
    this->parameters()->associations()->valueAs<smtk::mesh::Component>());
  smtk::operation::MarkGeometry().markModified(differenceComponent);

  // Return with success
  return result;
}

const char* Subtract::xmlDescription() const
{
  return Subtract_xml;
}

} //namespace mesh
} // namespace smtk
