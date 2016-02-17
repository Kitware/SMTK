//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Reclassify_h
#define __smtk_mesh_Reclassify_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/MeshSet.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Edge.h"

namespace smtk {
  namespace mesh {

#ifndef SHIBOKEN_SKIP
  //This operation is the mesh mirror for the split operator of a model
  //It needs to occur after the model operation has been run.
  //The first parameter is the original Edge that has been split,
  //the second parameter being the new edge which was
  SMTKCORE_EXPORT
  bool split(smtk::mesh::CollectionPtr,
             smtk::model::Edge orignalEdge,
             smtk::model::Edge newEdge,
             smtk::model::Vertex promotedVertex);

  //Merge together two edges and a model vertex into a single new edge mesh
  //representation. The model vertex mesh will be removed from the system
  SMTKCORE_EXPORT
  bool merge(smtk::mesh::CollectionPtr,
             smtk::model::Vertex toRemoveVert,
             smtk::model::Edge toRemoveEdge,
             smtk::model::Edge toAddTo);




  //----------------------------------------------------------------------------
  //Lower level routines that merge/split/etc are based on but could be
  //useful for other people to use

  //Given two meshsets we will make sure that all cells that exists both in A
  //and B will be removed from B.
  SMTKCORE_EXPORT
  bool make_disjoint(smtk::mesh::CollectionPtr,
                    const smtk::mesh::MeshSet& a,
                    smtk::mesh::MeshSet& b,
                    const smtk::model::EntityRef& modelAssoc);


  //Fuse two meshset together to create a new single meshset with a
  //model association to the given modelRef.
  //When successful it will update 'toAddTo' to point to the new merged
  //meshset
  SMTKCORE_EXPORT
  bool fuse(smtk::mesh::CollectionPtr,
             smtk::mesh::MeshSet& toRemove,
             smtk::mesh::MeshSet& toAddTo,
             const smtk::model::EntityRef& assoc);

#endif
  }
}

#endif
