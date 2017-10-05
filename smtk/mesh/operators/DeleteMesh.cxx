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
      // all mesh items are guaranteed to have a valid associated collection by ableToOperate
      smtk::mesh::CollectionPtr collec = mit->collection();

      //remove the mesh from its collection
      success = collec->removeMeshes(smtk::mesh::MeshSet(*mit));
      if (success)
      {
        expunged.insert(*mit);
      }

      //if the collection no longer has any meshes, remove
      //the collection from the manager as well
      if (collec->numberOfMeshes() == 0)
      {
        meshMgr->removeCollection(collec);
      }
    }
  }

  smtk::model::OperatorResult result =
    this->createResult(success ? smtk::operation::Operator::OPERATION_SUCCEEDED
                               : smtk::operation::Operator::OPERATION_FAILED);

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
