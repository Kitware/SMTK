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
#include "smtk/mesh/moab/MergeMeshVertices.h"

namespace smtk {
  namespace mesh {
    namespace moab {

//----------------------------------------------------------------------------
MergeMeshVertices::MergeMeshVertices(::moab::Interface* iface):
  mbImpl(iface),
  mbMergeTag(),
  deadEnts(),
  mergedToVertices()
{
}

//----------------------------------------------------------------------------
MergeMeshVertices::~MergeMeshVertices()
{
  if (mbMergeTag)
    {
    mbImpl->tag_delete(mbMergeTag);
    }
}

//----------------------------------------------------------------------------
::moab::ErrorCode MergeMeshVertices::merge_entities(const smtk::mesh::HandleRange&  meshsets,
                                                    const double merge_tol)
{
  using ::moab::AdaptiveKDTree;
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_TAG_DENSE;
  using ::moab::MB_TAG_EXCL;
  using ::moab::MB_TYPE_HANDLE;
  using ::moab::Range;
  using ::moab::MBENTITYSET;
  using ::moab::MBVERTEX;
  using ::moab::MB_SUCCESS;

  ErrorCode rval;

  EntityHandle def_val = 0;
  rval = mbImpl->tag_get_handle("__merge_tag", 1, MB_TYPE_HANDLE,
        mbMergeTag, MB_TAG_DENSE | MB_TAG_EXCL, &def_val);
  if (MB_SUCCESS != rval)
    {
    return rval;
    }

  // get all entities;
  // get all vertices connected
  // build a kdtree
  // find merged to
  mergeTol = merge_tol;
  mergeTolSq = merge_tol * merge_tol;

  // get all vertices for the meshsets
  Range entities;
  for(smtk::mesh::HandleRange::const_iterator i = meshsets.begin();
      i != meshsets.end();
      ++i)
    {
    Range tmp;
    rval = mbImpl->get_entities_by_handle(*i, entities, /*recursive*/ true);
    if (MB_SUCCESS != rval)
      {
      return rval;
      }
    entities.insert(tmp.begin(), tmp.end());
    }

  Range sets= entities.subset_by_type(MBENTITYSET);
  entities= ::moab::subtract(entities, sets);
  Range verts;
  rval = mbImpl->get_connectivity(entities, verts);;
  if (MB_SUCCESS != rval)
    {
    return rval;
    }

  // build a kd tree with the vertices
  AdaptiveKDTree kd(mbImpl);
  EntityHandle tree_root;
  rval = kd.build_tree(verts, &tree_root);
  if (MB_SUCCESS != rval)
    {
    return rval;
    }

  // find matching vertices, mark them
  rval = find_merged_to(tree_root, kd, mbMergeTag);;
  if (MB_SUCCESS != rval)
    {
    return rval;
    }

  rval = perform_merge(mbMergeTag);;
  if (MB_SUCCESS != rval)
    {
    return rval;
    }

  //currently skipping higher dimensions

  return MB_SUCCESS;
}

//----------------------------------------------------------------------------
::moab::ErrorCode MergeMeshVertices::find_merged_to(::moab::EntityHandle &tree_root,
                                            ::moab::AdaptiveKDTree &tree,
                                            ::moab::Tag merged_to)
{

  using ::moab::AdaptiveKDTree;
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_TAG_DENSE;
  using ::moab::MB_TAG_EXCL;
  using ::moab::MB_TYPE_HANDLE;
  using ::moab::Range;
  using ::moab::MBENTITYSET;
  using ::moab::MBVERTEX;
  using ::moab::MB_SUCCESS;
  using ::moab::AdaptiveKDTreeIter;
  using ::moab::MB_ENTITY_NOT_FOUND;
  using ::moab::CartVect;

  AdaptiveKDTreeIter iter;

  // evaluate vertices in this leaf
  Range leaf_range, leaf_range2;
  std::vector<EntityHandle> sorted_leaves;
  std::vector<double> coords;
  std::vector<EntityHandle> merge_tag_val, leaves_out;

  ErrorCode result = tree.get_tree_iterator(tree_root, iter);
  if (MB_SUCCESS != result)
    return result;
  while (result == MB_SUCCESS)
  {
    sorted_leaves.push_back(iter.handle());
    result = iter.step();
  }
  if (result != MB_ENTITY_NOT_FOUND)
    return result;
  std::sort(sorted_leaves.begin(), sorted_leaves.end());

  std::vector<EntityHandle>::iterator it;
  for (it = sorted_leaves.begin(); it != sorted_leaves.end(); ++it)
  {

    leaf_range.clear();
    result = mbImpl->get_entities_by_handle(*it, leaf_range);
    if (MB_SUCCESS != result)
      return result;
    coords.resize(3 * leaf_range.size());
    merge_tag_val.resize(leaf_range.size());
    result = mbImpl->get_coords(leaf_range, &coords[0]);
    if (MB_SUCCESS != result)
      return result;
    result = mbImpl->tag_get_data(merged_to, leaf_range, &merge_tag_val[0]);
    if (MB_SUCCESS != result)
      return result;
    Range::iterator rit;
    unsigned int i;
    bool inleaf_merged, outleaf_merged = false;
    unsigned int lr_size = leaf_range.size();

    for (i = 0, rit = leaf_range.begin(); i != lr_size; rit++, i++)
    {
      if (0 != merge_tag_val[i])
        continue;
      CartVect from(&coords[3 * i]);
      inleaf_merged = false;

      // check close-by leaves too
      leaves_out.clear();
      result = tree.distance_search(from.array(), mergeTol, leaves_out,
          mergeTol, 1.0e-6, NULL, NULL, &tree_root);
      leaf_range2.clear();
      for (std::vector<EntityHandle>::iterator vit = leaves_out.begin();
          vit != leaves_out.end(); vit++)
      {
        if (*vit > *it)
        { // if we haven't visited this leaf yet in the outer loop
          result = mbImpl->get_entities_by_handle(*vit,
                                                  leaf_range2,
                                                  ::moab::Interface::UNION);
          if (MB_SUCCESS != result)
            return result;
        }
      }
      if (!leaf_range2.empty())
      {
        coords.resize(3 * (lr_size + leaf_range2.size()));
        merge_tag_val.resize(lr_size + leaf_range2.size());
        result = mbImpl->get_coords(leaf_range2, &coords[3 * lr_size]);
        if (MB_SUCCESS != result)
          return result;
        result = mbImpl->tag_get_data(merged_to, leaf_range2,
            &merge_tag_val[lr_size]);
        if (MB_SUCCESS != result)
          return result;
        outleaf_merged = false;
      }

      // check other verts in this leaf
      for (unsigned int j = i + 1; j < merge_tag_val.size(); j++)
      {
        EntityHandle to_ent =
            j >= lr_size ? leaf_range2[j - lr_size] : leaf_range[j];

        if (*rit == to_ent)
          continue;

        //needs to be less than/equal too so that points are resolved
        //even when the tolerance is zero. Otherwise exact matches are
        //considered to be not the same
        const double distSq = (from - CartVect(&coords[3 * j])).length_squared();
        if (distSq <= mergeTolSq)
        {
          merge_tag_val[j] = *rit;
          if (j < lr_size)
          {
            inleaf_merged = true;
          }
          else
          {
            outleaf_merged = true;
          }
          deadEnts.insert(to_ent);
        }

      }
      if (outleaf_merged)
      {
        result = mbImpl->tag_set_data(merged_to, leaf_range2,
            &merge_tag_val[leaf_range.size()]);
        if (MB_SUCCESS != result)
          return result;
        outleaf_merged = false;
      }
      if (inleaf_merged)
      {
        result = mbImpl->tag_set_data(merged_to, leaf_range, &merge_tag_val[0]);
        if (MB_SUCCESS != result)
          return result;
      }

    }
  }
  return MB_SUCCESS;
}


//----------------------------------------------------------------------------
::moab::ErrorCode MergeMeshVertices::perform_merge(::moab::Tag merged_to)
{
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::Range;
  using ::moab::MBVERTEX;
  using ::moab::MB_SUCCESS;
  using ::moab::MB_FAILURE;

  // we start with an empty range of vertices that are "merged to"
  // they are used (eventually) for higher dim entities
  mergedToVertices.clear();
  ErrorCode result;
  if (deadEnts.size() == 0)
  {
    std::cout << "deadEnts size is zero" << std::endl;
    return MB_SUCCESS; //nothing to merge carry on with the program
  }
  if (mbImpl->type_from_handle(*deadEnts.rbegin()) != MBVERTEX)
    return MB_FAILURE;
  std::vector<EntityHandle> merge_tag_val(deadEnts.size());
  result = mbImpl->tag_get_data(merged_to, deadEnts, &merge_tag_val[0]);
  if (MB_SUCCESS != result)
    return result;

  Range::iterator rit;
  unsigned int i;
  for (rit = deadEnts.begin(), i = 0; rit != deadEnts.end(); rit++, i++)
  {
    assert(merge_tag_val[i]);
    if (MBVERTEX==mbImpl->type_from_handle(merge_tag_val[i]) )
      mergedToVertices.insert(merge_tag_val[i]);
    result = mbImpl->merge_entities(merge_tag_val[i], *rit, false, false);
    if (MB_SUCCESS != result)
    {
      return result;
    }
  }
  result = mbImpl->delete_entities(deadEnts);
  return result;
}


    } // namespace moab
  } // namespace mesh
} // namespace smtk
