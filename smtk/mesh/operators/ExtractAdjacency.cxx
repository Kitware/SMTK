//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/ExtractAdjacency.h"

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

#include "smtk/mesh/operators/ExtractAdjacency_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::ExtractAdjacency::Result ExtractAdjacency::operateInternal()
{
  // Access the meshset
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>();
  smtk::mesh::MeshSet meshset = meshComponent->mesh();

  // Access the dimension of the requested adjacency mesh
  smtk::attribute::StringItem::Ptr nameItem = this->parameters()->findString("dimension");
  smtk::mesh::DimensionType dimension = smtk::mesh::DimensionType_MAX;
  if (nameItem->value() == "0")
  {
    dimension = smtk::mesh::Dims0;
  }
  else if (nameItem->value() == "1")
  {
    dimension = smtk::mesh::Dims1;
  }
  else if (nameItem->value() == "2")
  {
    dimension = smtk::mesh::Dims2;
  }
  else if (nameItem->value() == "3")
  {
    dimension = smtk::mesh::Dims3;
  }

  // Extract the adjacency mesh
  smtk::mesh::MeshSet adjacencies = meshset.extractAdjacenciesOfDimension(dimension);

  if (adjacencies.is_empty())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  adjacencies.setName(meshset.name() + " (adjacency)");

  smtk::mesh::Component::Ptr adjacencyComponent = smtk::mesh::Component::create(adjacencies);

  if (adjacencyComponent == nullptr)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Create a new result
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with modified components
  result->findComponent("created")->appendValue(adjacencyComponent);

  // Mark the created component as having a modified geometry so it will be
  // propertly rendered
  smtk::operation::MarkGeometry().markModified(adjacencyComponent);

  // Return with success
  return result;
}

const char* ExtractAdjacency::xmlDescription() const
{
  return ExtractAdjacency_xml;
}
} // namespace mesh
} // namespace smtk
