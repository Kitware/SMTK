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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshItem.h"

namespace smtk
{
namespace mesh
{

bool DeleteMesh::ableToOperate()
{
  if (!this->ensureSpecification())
    return false;
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
  return meshItem && meshItem->numberOfValues() > 0;
}

smtk::model::OperatorResult DeleteMesh::operateInternal()
{
  // ableToOperate should have verified that mesh(s) are set
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  smtk::mesh::MeshSets expunged;
  bool success = true;
  smtk::mesh::ManagerPtr meshMgr = this->manager()->meshes();
  if (meshMgr)
  {
    for (attribute::MeshItem::const_mesh_it mit = meshItem->begin(); mit != meshItem->end(); ++mit)
    {
      smtk::mesh::CollectionPtr collec = mit->collection();
      if (collec)
      { //can have an invalid collection when trying to delete
        //an NULL/Invalid MeshSet.
        const bool wasRemoved = meshMgr->removeCollection(collec);
        if (wasRemoved)
        {
          expunged.insert(*mit);
        }
      }
    }
  }

  smtk::model::OperatorResult result =
    this->createResult(success ? smtk::model::OPERATION_SUCCEEDED : smtk::model::OPERATION_FAILED);

  if (success)
  {
    result->findMesh("mesh_expunged")->appendValues(expunged);
  }
  return result;
}

} //namespace mesh
} // namespace smtk

#include "smtk/mesh/DeleteMesh_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::mesh::DeleteMesh, delete_mesh, "delete mesh",
  DeleteMesh_xml, smtk::model::Session);
