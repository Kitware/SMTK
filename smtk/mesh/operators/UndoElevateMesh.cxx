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
#include "smtk/attribute/MeshItem.h"

#include "smtk/io/Logger.h"

#include "smtk/mesh/UndoElevateMesh_xml.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/utility/ApplyToMesh.h"

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

  smtk::attribute::MeshItem::Ptr meshItem = this->parameters()->findMesh("mesh");
  if (!meshItem || meshItem->numberOfValues() == 0)
  {
    return false;
  }

  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);
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
  smtk::attribute::MeshItem::Ptr meshItem = this->parameters()->findMesh("mesh");

  // Access the attribute associated with the modified meshes
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::NewOp::Outcome::SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the modified model
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);

    bool success = smtk::mesh::utility::undoWarp(mesh);

    if (!success)
    {
      smtkErrorMacro(this->log(), "Undo elevate failed.");
      return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
    }

    modifiedMeshes->appendValue(mesh);

    smtk::model::EntityRefArray entities;
    bool entitiesAreValid = mesh.modelEntities(entities);
    if (entitiesAreValid && !entities.empty())
    {
      smtk::model::Model model = entities[0].owningModel();
      modified->appendValue(model.component());
      modifiedEntities->appendValue(model);
    }
  }

  return result;
}

const char* UndoElevateMesh::xmlDescription() const
{
  return UndoElevateMesh_xml;
}
}
}
