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


#ifndef __smtk_mesh_moab_Helpers_h
#define __smtk_mesh_moab_Helpers_h

#include "Interface.h"
#include "CellTypeToType.h"
#include "Tags.h"

#include <algorithm>
#include <cstring>
#include <set>
#include <vector>

namespace smtk {
namespace mesh {
namespace moab {

//----------------------------------------------------------------------------
inline
std::size_t numMeshes(smtk::mesh::Handle handle,
                      const smtk::mesh::moab::InterfacePtr& iface)
{
  int num_ents = 0;
  iface->get_number_entities_by_type( handle, ::moab::MBENTITYSET, num_ents);
  return static_cast<std::size_t>(num_ents);
}

//----------------------------------------------------------------------------
inline
smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                     const smtk::mesh::moab::InterfacePtr& iface)

{
  ::moab::Range range;
  iface->get_entities_by_type(handle, ::moab::MBENTITYSET, range);
  return range;
}

//----------------------------------------------------------------------------
inline
smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                     int dimension,
                                     const smtk::mesh::moab::InterfacePtr& iface)

{
  ::moab::Range all_meshes_with_dim_tag;
  ::moab::Range meshes_of_proper_dim;

  //construct a dim tag that matches the dimension coming in
  tag::QueryDimTag dimTag(dimension, iface);

  // get all the entities of that type in the mesh
  iface->get_entities_by_type_and_tag(handle,
                                      ::moab::MBENTITYSET,
                                      dimTag.moabTag(),
                                      NULL,
                                      1,
                                      all_meshes_with_dim_tag);

  typedef ::moab::Range::const_iterator iterator;
  for(iterator i=all_meshes_with_dim_tag.begin();
      i != all_meshes_with_dim_tag.end(); ++i)
    {
    int value = 0;
    iface->tag_get_data(dimTag.moabTagAsRef(), &(*i), 1, &value);
    if(value == dimTag.value())
      {
      meshes_of_proper_dim.insert(*i);
      }
    }
  return meshes_of_proper_dim;
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
inline
smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                     const std::string& name,
                                     const smtk::mesh::moab::InterfacePtr& iface)

{
  typedef std::vector< ::moab::EntityHandle >::const_iterator it;

  //I can't get get_entities_by_type_and_tag to work properly for this
  //query so I am going to do it the slow way by doing the checking manually

  //use a vector since we are going to do single element iteration, and
  //removal.
  std::vector< ::moab::EntityHandle > all_ents;
  std::vector< ::moab::EntityHandle > matching_ents;
  //get all ents
  iface->get_entities_by_type(handle, ::moab::MBENTITYSET, all_ents);

  //see which ones have a a matching name, and if so add it
  tag::QueryNameTag query_name(iface);
  for( it i = all_ents.begin(); i != all_ents.end(); ++i )
    {
    const bool has_name = query_name.fetch_name(*i);
    if(has_name &&
       ( std::strcmp(name.c_str(), query_name.current_name()) == 0 ) )
      { //has a matching name
      matching_ents.push_back(*i);
      }
    }

  all_ents.clear();

  smtk::mesh::HandleRange result;
  std::copy( matching_ents.rbegin(), matching_ents.rend(),
             ::moab::range_inserter(result) );
  return result;
}

//----------------------------------------------------------------------------
//get all cells held by this range
inline
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  const smtk::mesh::moab::InterfacePtr& iface)

{
  // get all non-meshset entities in meshset, including in contained meshsets
  typedef ::moab::Range::const_iterator iterator;
  ::moab::Range entitiesCells;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_handle appends to the range given
    iface->get_entities_by_handle(*i, entitiesCells, true);
    }
  return entitiesCells;
}


//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type
inline
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  smtk::mesh::CellType cellType,
                                  const smtk::mesh::moab::InterfacePtr& iface)
{
  smtk::mesh::moab::EntityType moabCellType =
                                  smtk::mesh::moab::smtkToMOABCell(cellType);

  ::moab::Range entitiesCells;

  // get all non-meshset entities in meshset of a given cell type
  typedef ::moab::Range::const_iterator iterator;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_type appends to the range given
    iface->get_entities_by_type(*i, moabCellType, entitiesCells, true);
    }
  return entitiesCells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type(s)
inline
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  const smtk::mesh::CellTypes& cellTypes,
                                  const smtk::mesh::moab::InterfacePtr& iface)

{
  const std::size_t cellTypesToFind = cellTypes.count();
  if( cellTypesToFind == cellTypes.size())
    { //if all the cellTypes are enabled we should just use get_cells
      //all() method can't be used as it was added in C++11
    return get_cells( meshsets, iface);
    }
  else if(cellTypesToFind == 0)
    {
    return smtk::mesh::HandleRange();
    }

  //we now search from highest cell type to lowest cell type adding everything
  //to the range. The reason for this is that ranges perform best when inserting
  //from high to low values
  ::moab::Range entitiesCells;
  for(int i = (cellTypes.size() -1); i >= 0; --i )
    {
    //skip all cell types we don't have
    if( !cellTypes[i] )
      { continue; }

    smtk::mesh::CellType currentCellType = static_cast<smtk::mesh::CellType>(i);

    ::moab::Range cellEnts = get_cells(meshsets, currentCellType, iface);

    entitiesCells.insert(cellEnts.begin(), cellEnts.end());
    }

  return entitiesCells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given dimension
inline
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  smtk::mesh::DimensionType dim,
                                  const smtk::mesh::moab::InterfacePtr& iface)

{
  const int dimension = static_cast<int>(dim);

  //get all non-meshset entities of a given dimension
  typedef ::moab::Range::const_iterator iterator;
  ::moab::Range entitiesCells;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_dimension appends to the range given
    iface->get_entities_by_dimension(*i, dimension, entitiesCells, true);
    }
  return entitiesCells;
}


//----------------------------------------------------------------------------
inline
std::vector< std::string > compute_names(const smtk::mesh::HandleRange& r,
                                         const smtk::mesh::moab::InterfacePtr& iface)
{
  //construct a name tag query helper class
  tag::QueryNameTag query_name(iface);

  typedef ::moab::Range::const_iterator it;
  std::set< std::string > unique_names;
  for(it i = r.begin(); i != r.end(); ++i)
    {
    const bool has_name = query_name.fetch_name(*i);
    if(has_name)
      {
      unique_names.insert( std::string(query_name.current_name()) );
      }
    }
  //return a vector of the unique names
  return std::vector< std::string >(unique_names.begin(), unique_names.end());
}

//----------------------------------------------------------------------------
inline
smtk::mesh::TypeSet compute_types(smtk::mesh::Handle handle,
                                  const smtk::mesh::moab::InterfacePtr& iface)
{
  int numMeshes = 0;
  iface->get_number_entities_by_type( handle, ::moab::MBENTITYSET, numMeshes);

  //iterate over all the celltypes and get the number for each
  //construct a smtk::mesh::CellTypes at the same time
  typedef ::smtk::mesh::CellType CellEnum;
  smtk::mesh::CellTypes ctypes;
  if(numMeshes > 0)
    {
    for(int i=0; i < ctypes.size(); ++i )
      {
      CellEnum ce = static_cast<CellEnum>(i);
      //now we need to convert from CellEnum to MoabType
      smtk::mesh::moab::EntityType moabEType =
                                  smtk::mesh::moab::smtkToMOABCell(ce);

      //some of the cell types that smtk supports moab doesn't support
      //so we can't query on those.
      int num = 0;
      iface->get_number_entities_by_type(handle, moabEType, num);
      ctypes[ce] = (num > 0);
      }
    }

  //determine the state of the typeset
  const bool hasMeshes = numMeshes > 0;
  const bool hasCells = ctypes.any();
  return smtk::mesh::TypeSet(ctypes, hasMeshes, hasCells) ;
}


}
}
}

#endif