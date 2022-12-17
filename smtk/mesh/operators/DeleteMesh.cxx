//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/DeleteMesh.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/mesh/operators/DeleteMesh_xml.h"

namespace smtk
{
namespace mesh
{

DeleteMesh::DeleteMesh() = default;

smtk::mesh::DeleteMesh::Result DeleteMesh::operateInternal()
{
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  bool allRemoved = true;
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>(i);
    smtk::mesh::MeshSet meshset = meshComponent->mesh();
    bool removed = meshset.resource()->removeMeshes(meshset);

    if (removed)
    {
      result->findComponent("expunged")->appendValue(meshComponent);
      smtk::operation::MarkGeometry().erase(meshComponent);
    }

    allRemoved &= removed;
  }

  if (!allRemoved)
  {
    result->findInt("outcome")->setValue(
      0, static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
  }
  return result;
}

const char* DeleteMesh::xmlDescription() const
{
  return DeleteMesh_xml;
}

void DeleteMesh::generateSummary(smtk::operation::Operation::Result& result)
{
  if (!m_suppressOutput)
  {
    smtk::operation::Operation::generateSummary(result);
  }
}

} //namespace mesh
} // namespace smtk
