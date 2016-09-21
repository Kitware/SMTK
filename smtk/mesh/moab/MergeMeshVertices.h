//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef __smtk_mesh_moab_MergeMeshVertices_h
#define __smtk_mesh_moab_MergeMeshVertices_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include "smtk/mesh/Handle.h"

#include <map>

namespace smtk {
  namespace mesh {
    namespace moab {

class MergeMeshVertices
{
public:
  MergeMeshVertices(::moab::Interface* iface);

  ~MergeMeshVertices();

  ::moab::ErrorCode merge_entities(const smtk::mesh::HandleRange&  meshsets,
                                   const double merge_tol=1.0e-6);
private:
  //- given a kdtree, set tag on vertices in leaf nodes with vertices
  //- to which they should be merged
  ::moab::ErrorCode find_merged_to(::moab::EntityHandle &tree_root,
                                   ::moab::AdaptiveKDTree &tree,
                                   ::moab::Tag merged_to);

  //- fill mappingFromDeadToAlive
  ::moab::ErrorCode map_dead_to_alive(::moab::Tag merged_to);

  //- delete the deadEnts
  ::moab::ErrorCode delete_dead_entities(::moab::Tag merged_to);

  //- correct any occurrences of vertices inside a mesh being deleted and
  // the replacement vertex not already being an entity of that mesh
  ::moab::ErrorCode correct_vertex_merge(const smtk::mesh::HandleRange&  meshsets);

  //Update the connectivity of the cells that used one or more of the
  //soon to be dead points
  ::moab::ErrorCode update_connectivity();

  //Identify higher dimension to be merged
  ::moab::ErrorCode merge_higher_dimensions(::moab::Range &elems);

  ::moab::Interface *mbImpl;

  //- the tag pointing to the entity to which an entity will be merged
  ::moab::Tag mbMergeTag;

  //- entities which will go away after the merge
  ::moab::Range deadEnts;

  // vertices that were merged with other vertices, and were left in the database
  ::moab::Range mergedToVertices;

  // mapping from deadEnts to vertices that we are keeping
  std::map< ::moab::EntityHandle, ::moab::EntityHandle> mappingFromDeadToAlive;

  double mergeTol, mergeTolSq;

};

    } // namespace moab
  } // namespace mesh
} // namespace smtk

#endif
