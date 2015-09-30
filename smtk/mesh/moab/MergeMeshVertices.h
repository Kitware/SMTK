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

#include "moab/AdaptiveKDTree.hpp"
#include "smtk/mesh/Handle.h"

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

  //- perform the actual merge
  ::moab::ErrorCode perform_merge(::moab::Tag merged_to);

  ::moab::Interface *mbImpl;

  //- the tag pointing to the entity to which an entity will be merged
  ::moab::Tag mbMergeTag;

  //- entities which will go away after the merge
  ::moab::Range deadEnts;

  // vertices that were merged with other vertices, and were left in the database
  ::moab::Range mergedToVertices;

  double mergeTol, mergeTolSq;

};

    } // namespace moab
  } // namespace mesh
} // namespace smtk

#endif
