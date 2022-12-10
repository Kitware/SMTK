//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/MergeCoincidentPoints.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/operators/MergeCoincidentPoints_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::MergeCoincidentPoints::Result MergeCoincidentPoints::operateInternal()
{
  // Access the tolerance.
  double tolerance = this->parameters()->findDouble("tolerance")->value();

  // Access the meshset
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>();
  smtk::mesh::MeshSet meshset = meshComponent->mesh();

  // Set the name of the meshset
  bool success = meshset.mergeCoincidentContactPoints(tolerance);

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

const char* MergeCoincidentPoints::xmlDescription() const
{
  return MergeCoincidentPoints_xml;
}

} //namespace mesh
} // namespace smtk
