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

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/core/MeshSet.h"

#include "smtk/model/Session.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/mesh/DeleteMesh_xml.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::DeleteMesh::Result DeleteMesh::operateInternal()
{
  // ableToOperate should have verified that mesh(s) are set
  smtk::attribute::MeshItem::Ptr meshItem = this->parameters()->findMesh("mesh");

  smtk::mesh::MeshSets expunged;
  bool success = true;
  for (attribute::MeshItem::const_mesh_it mit = meshItem->begin(); mit != meshItem->end(); ++mit)
  {
    // all mesh items are guaranteed to have a valid associated collection by ableToOperate
    smtk::mesh::CollectionPtr collec = mit->collection();
    smtk::mesh::ManagerPtr meshMgr = collec->manager();

    if (meshMgr)
    {
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

  Result result = this->createResult(success ? smtk::operation::Operation::Outcome::SUCCEEDED
                                             : smtk::operation::Operation::Outcome::FAILED);

  if (success)
  {
    result->findMesh("mesh_expunged")->appendValues(expunged);
  }
  return result;
}

const char* DeleteMesh::xmlDescription() const
{
  return DeleteMesh_xml;
}

} //namespace mesh
} // namespace smtk
