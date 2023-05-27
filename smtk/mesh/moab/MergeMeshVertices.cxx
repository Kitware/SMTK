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

namespace smtk
{
namespace mesh
{
namespace moab
{

MergeMeshVertices::MergeMeshVertices(::moab::Interface* iface)
  : mbImpl(iface)
{
}

MergeMeshVertices::~MergeMeshVertices()
{
  if (mbMergeTag)
  {
    mbImpl->tag_delete(mbMergeTag);
  }
}

::moab::ErrorCode MergeMeshVertices::merge_entities(
  const ::moab::Range& meshsets,
  const double merge_tol)
{
  using ::moab::AdaptiveKDTree;
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_SUCCESS;
  using ::moab::MB_TAG_DENSE;
  using ::moab::MB_TAG_EXCL;
  using ::moab::MB_TYPE_HANDLE;
  using ::moab::MBENTITYSET;
  using ::moab::MBVERTEX;
  using ::moab::Range;

  ErrorCode rval;

  EntityHandle def_val = 0;
  rval = mbImpl->tag_get_handle(
    "__merge_tag", 1, MB_TYPE_HANDLE, mbMergeTag, MB_TAG_DENSE | MB_TAG_EXCL, &def_val);
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
  for (Range::const_iterator i = meshsets.begin(); i != meshsets.end(); ++i)
  {
    Range tmp;
    rval = mbImpl->get_entities_by_handle(*i, tmp, /*recursive*/ true);
    if (MB_SUCCESS != rval)
    {
      return rval;
    }
    entities.insert(tmp.begin(), tmp.end());
  }

  Range verts;
  rval = mbImpl->get_connectivity(entities, verts);

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
  rval = find_merged_to(tree_root, kd, mbMergeTag);
  if (MB_SUCCESS != rval)
  {
    return rval;
  }

  if (!deadEnts.empty())
  {
    rval = map_dead_to_alive(mbMergeTag);
    if (MB_SUCCESS != rval)
    {
      return rval;
    }

    //before we delete any elements, we need to update the connectivity of elements
    //that use the dead vertices
    this->update_connectivity();

    //Any meshset that explicitly has a deleted vertex needs to have the
    //the replacement vertex added back to it.
    rval = correct_vertex_merge(meshsets);
    if (MB_SUCCESS != rval)
    {
      return rval;
    }
  }

  rval = merge_higher_dimensions(entities);
  if (MB_SUCCESS != rval)
  {
    return rval;
  }

  return MB_SUCCESS;
}

::moab::ErrorCode MergeMeshVertices::find_merged_to(
  ::moab::EntityHandle& tree_root,
  ::moab::AdaptiveKDTree& tree,
  ::moab::Tag merged_to)
{

  using ::moab::AdaptiveKDTree;
  using ::moab::AdaptiveKDTreeIter;
  using ::moab::CartVect;
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_ENTITY_NOT_FOUND;
  using ::moab::MB_SUCCESS;
  using ::moab::MB_TAG_DENSE;
  using ::moab::MB_TAG_EXCL;
  using ::moab::MB_TYPE_HANDLE;
  using ::moab::MBENTITYSET;
  using ::moab::MBVERTEX;
  using ::moab::Range;

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
    result = mbImpl->get_coords(leaf_range, coords.data());
    if (MB_SUCCESS != result)
      return result;
    result = mbImpl->tag_get_data(merged_to, leaf_range, merge_tag_val.data());
    if (MB_SUCCESS != result)
      return result;
    Range::iterator rit;
    std::size_t i;
    bool inleaf_merged, outleaf_merged = false;
    std::size_t lr_size = leaf_range.size();

    for (i = 0, rit = leaf_range.begin(); i != lr_size; rit++, i++)
    {
      if (0 != merge_tag_val[i])
        continue;
      CartVect from(&coords[3 * i]);
      inleaf_merged = false;

      // check close-by leaves too
      leaves_out.clear();
      result = tree.distance_search(
        from.array(), mergeTol, leaves_out, mergeTol, 1.0e-6, nullptr, nullptr, &tree_root);
      leaf_range2.clear();
      for (std::vector<EntityHandle>::iterator vit = leaves_out.begin(); vit != leaves_out.end();
           vit++)
      {
        if (*vit > *it)
        { // if we haven't visited this leaf yet in the outer loop
          result = mbImpl->get_entities_by_handle(*vit, leaf_range2, ::moab::Interface::UNION);
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
        result = mbImpl->tag_get_data(merged_to, leaf_range2, &merge_tag_val[lr_size]);
        if (MB_SUCCESS != result)
          return result;
        outleaf_merged = false;
      }

      // check other verts in this leaf
      for (std::size_t j = i + 1; j < merge_tag_val.size(); j++)
      {
        EntityHandle to_ent = j >= lr_size ? leaf_range2[j - lr_size] : leaf_range[j];

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
        result = mbImpl->tag_set_data(merged_to, leaf_range2, &merge_tag_val[leaf_range.size()]);
        if (MB_SUCCESS != result)
          return result;
        outleaf_merged = false;
      }
      if (inleaf_merged)
      {
        result = mbImpl->tag_set_data(merged_to, leaf_range, merge_tag_val.data());
        if (MB_SUCCESS != result)
          return result;
      }
    }
  }
  return MB_SUCCESS;
}

::moab::ErrorCode MergeMeshVertices::map_dead_to_alive(::moab::Tag merged_to)
{
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_FAILURE;
  using ::moab::MB_SUCCESS;
  using ::moab::MBVERTEX;
  using ::moab::Range;

  // we start with an empty range of vertices that are "merged to"
  // they are used (eventually) for higher dim entities
  mergedToVertices.clear();
  ErrorCode result;
  if (deadEnts.empty())
  {
    return MB_SUCCESS; //nothing to merge carry on with the program
  }

  if (mbImpl->type_from_handle(*deadEnts.rbegin()) != MBVERTEX)
    return MB_FAILURE;
  std::vector<EntityHandle> merge_tag_val(deadEnts.size());
  result = mbImpl->tag_get_data(merged_to, deadEnts, merge_tag_val.data());
  if (MB_SUCCESS != result)
    return result;

  //first build up the mapping from dead to new vertices
  Range::iterator rit;
  std::size_t i;
  for (rit = deadEnts.begin(), i = 0; rit != deadEnts.end(); rit++, i++)
  {
    assert(merge_tag_val[i]);
    if (MBVERTEX == mbImpl->type_from_handle(merge_tag_val[i]))
    {
      mergedToVertices.insert(merge_tag_val[i]);
      mappingFromDeadToAlive[*rit] = static_cast<EntityHandle>(merge_tag_val[i]);
    }
  }

  return MB_SUCCESS;
}

//now before we delete the entities,
//we need to make sure that any mesh that is losing an explicit vertex
//has it replaced with the merged vertex, this isn't handled by perform_merge
//as it only does dim > 0
::moab::ErrorCode MergeMeshVertices::correct_vertex_merge(const ::moab::Range& meshsets)
{
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_FAILURE;
  using ::moab::MB_SUCCESS;
  using ::moab::MBVERTEX;
  using ::moab::Range;

  for (Range::const_iterator i = meshsets.begin(); i != meshsets.end(); ++i)
  {
    Range entitiesVerts;
    mbImpl->get_entities_by_dimension(*i, 0 /*dimension*/, entitiesVerts, /*recursive*/ true);

    //determine if we have a vert which is going deleted
    Range vertsToDelete = ::moab::intersect(deadEnts, entitiesVerts);
    if (!vertsToDelete.empty())
    {
      Range::iterator rit;
      std::size_t j;
      for (rit = vertsToDelete.begin(), j = 0; rit != vertsToDelete.end(); rit++, j++)
      {
        //now we add these entities to the new meshset
        EntityHandle t = mappingFromDeadToAlive[*rit];
        mbImpl->add_entities(*i, &t, 1);
      }
      mbImpl->remove_entities(*i, vertsToDelete);
    }
  }

  return MB_SUCCESS;
}

//Update the connectivity of the cells that used one or more of the
//soon to be dead points
::moab::ErrorCode MergeMeshVertices::update_connectivity()
{
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_FAILURE;
  using ::moab::MB_SUCCESS;
  using ::moab::MBVERTEX;
  using ::moab::Range;

  ErrorCode result;

  for (int dim = 1; dim <= 3; dim++)
  {
    Range entsToUpdate;
    result = mbImpl->get_adjacencies(deadEnts, dim, false, entsToUpdate, ::moab::Interface::UNION);

    if (MB_SUCCESS != result)
    {
      return result;
    }

    //get the connectivity for all the deadEnts
    Range::iterator iter = entsToUpdate.begin();
    Range::iterator end = entsToUpdate.end();
    while (iter != end)
    {
      int numCellsInSubRange = 0;
      int verts_per_ent = 0;
      EntityHandle* connectivity = nullptr;
      result = mbImpl->connect_iterate(iter, end, connectivity, verts_per_ent, numCellsInSubRange);
      if (MB_SUCCESS != result)
      {
        return result;
      }

      //now we can iterate the connectivity, fixing it up as needed
      std::size_t index = 0;
      for (int i = 0; i < numCellsInSubRange; ++i, ++iter)
      {
        for (int j = 0; j < verts_per_ent; ++j, ++index)
        {
          typedef std::map<::moab::EntityHandle, ::moab::EntityHandle> MapType;
          MapType::const_iterator pos = mappingFromDeadToAlive.find(connectivity[index]);
          if (pos != mappingFromDeadToAlive.end())
          {
            //when we update the connectivity array we also need to update
            //the adjacencies table. This makes sure that quick lookups
            //are aware of the merging of points
            mbImpl->remove_adjacencies(*iter, connectivity + index, 1);
            connectivity[index] = pos->second;
            mbImpl->add_adjacencies(*iter, connectivity + index, 1, true);
          }
        }
      }
    }
  }

  return MB_SUCCESS;
}

//Determine which higher dimensional entities should be merged
::moab::ErrorCode MergeMeshVertices::merge_higher_dimensions(::moab::Range& elems)
{
  using ::moab::EntityHandle;
  using ::moab::ErrorCode;
  using ::moab::MB_FAILURE;
  using ::moab::MB_SUCCESS;
  using ::moab::MBVERTEX;
  using ::moab::Range;

  // apply a different strategy
  // look at the vertices that were merged to, earlier, and find all entities adjacent to them
  // elems (input) are used just for initial connectivity
  ErrorCode result;
  Range verts;
  result = mbImpl->get_connectivity(elems, verts);
  verts.merge(elems.subset_by_dimension(0)); //don't forget these
  if (MB_SUCCESS != result)
    return result;

  // all higher dim entities that will be merged will be connected to the vertices that were
  // merged earlier; we will look at these vertices only
  Range vertsOfInterest = intersect(this->mergedToVertices, verts);
  //Go through each dimension
  Range possibleEntsToMerge, conn, matches, moreDeadEnts;

  for (int dim = 1; dim < 3; dim++)
  {
    moreDeadEnts.clear();
    possibleEntsToMerge.clear();
    result = mbImpl->get_adjacencies(
      vertsOfInterest, dim, false, possibleEntsToMerge, ::moab::Interface::UNION);
    if (MB_SUCCESS != result)
      return result;
    //Go through each possible entity and see if it shares vertices with another entity of same dimension
    for (Range::iterator pit = possibleEntsToMerge.begin(); pit != possibleEntsToMerge.end(); pit++)
    {
      EntityHandle eh = *pit; //possible entity to be matched
      conn.clear();
      //Get the vertices connected to it in a range
      if (mbImpl->type_from_handle(eh) != MBVERTEX)
      {
        result = mbImpl->get_connectivity(&eh, 1, conn);
      }
      else
      {
        conn.insert(eh);
        result = MB_SUCCESS;
      }

      if (MB_SUCCESS != result)
        return result;
      matches.clear();
      // now retrieve all entities connected to all conn vertices
      result = mbImpl->get_adjacencies(conn, dim, false, matches, ::moab::Interface::INTERSECT);
      if (MB_SUCCESS != result)
        return result;
      if (matches.size() > 1)
      {
        for (Range::iterator matchIt = matches.begin(); matchIt != matches.end(); matchIt++)
        {
          EntityHandle to_remove = *matchIt;
          if (to_remove != eh)
          {
            moreDeadEnts.insert(to_remove);
            result = mbImpl->merge_entities(eh, to_remove, false, false);
            if (result != MB_SUCCESS)
              return result;
            possibleEntsToMerge.erase(to_remove);
          }
        }
      }
    }
    //Delete the entities of dimension dim
    result = mbImpl->delete_entities(moreDeadEnts);
    if (result != MB_SUCCESS)
      return result;
  }
  return MB_SUCCESS;
}

} // namespace moab
} // namespace mesh
} // namespace smtk
