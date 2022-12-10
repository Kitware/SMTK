//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/UndoElevateMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/io/Logger.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/operators/UndoElevateMesh_xml.h"
#include "smtk/mesh/utility/ApplyToMesh.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/model/Model.h"

namespace smtk
{
namespace mesh
{

bool UndoElevateMesh::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  if (!meshItem || meshItem->numberOfValues() == 0)
  {
    return false;
  }

  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->valueAs<smtk::mesh::Component>(i)->mesh();
    smtk::mesh::PointField prior = mesh.pointField("_prior");
    if (!prior.isValid())
    {
      return false;
    }
  }

  return true;
}

UndoElevateMesh::Result UndoElevateMesh::operateInternal()
{
  // Access the mesh
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();

  // Access the attribute associated with the modified meshes
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with the modified model
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>(i);
    smtk::mesh::MeshSet mesh = meshComponent->mesh();

    bool success = smtk::mesh::utility::undoWarp(mesh);

    if (!success)
    {
      smtkErrorMacro(this->log(), "Undo elevate failed.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    modified->appendValue(meshComponent);
    smtk::operation::MarkGeometry().markModified(meshComponent);

    smtk::model::EntityRefArray entities;
    bool entitiesAreValid = mesh.modelEntities(entities);
    if (entitiesAreValid && !entities.empty())
    {
      smtk::model::Model model = entities[0].owningModel();
      modified->appendValue(model.component());
      smtk::operation::MarkGeometry().markModified(model.component());
    }
  }

  return result;
}

const char* UndoElevateMesh::xmlDescription() const
{
  return UndoElevateMesh_xml;
}
} // namespace mesh
} // namespace smtk
