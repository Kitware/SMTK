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
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/ContainsFunctors.h"

#include "smtk/mesh/moab/CellTypeToType.h"
#include "smtk/mesh/moab/Allocator.h"

#include "moab/Core.hpp"
#include "moab/FileOptions.hpp"
#include "moab/Interface.hpp"
#include "moab/ReaderIface.hpp"

#define BEING_INCLUDED_BY_INTERFACE_CXX
//required to go after moab includes
#include "smtk/mesh/moab/Tags.h"
#undef BEING_USED_BY_INTERFACE_CXX

#include <algorithm>
#include <cstring>
#include <set>

namespace smtk {
namespace mesh {
namespace moab {

//construct an empty interface instance
smtk::mesh::moab::InterfacePtr make_interface()
{
  //Core is a fully implemented moab::Interface
  return smtk::mesh::moab::InterfacePtr( new smtk::mesh::moab::Interface() );
}

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface
smtk::mesh::moab::InterfacePtr extract_interface( const smtk::mesh::CollectionPtr& c)
{
  return smtk::dynamic_pointer_cast< smtk::mesh::moab::Interface > ( c->interface() );
}

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface, and than
//extract the raw moab interface pointer from that
::moab::Interface *const extract_moab_interface( const smtk::mesh::InterfacePtr &iface)
{
  smtk::mesh::moab::Interface* mi = dynamic_cast< smtk::mesh::moab::Interface*>(iface.get());
  return (mi == NULL) ? NULL : mi->moabInterface();
}

//----------------------------------------------------------------------------
Interface::Interface():
  m_iface( new ::moab::Core() ),
  m_alloc( new smtk::mesh::moab::Allocator( this->m_iface.get() ) )
{

}

//----------------------------------------------------------------------------
Interface::~Interface()
{

}
//----------------------------------------------------------------------------
smtk::mesh::AllocatorPtr Interface::allocator()
{
  return this->m_alloc;
}

//----------------------------------------------------------------------------
smtk::mesh::Handle Interface::getRoot() const

{
  return m_iface->get_root_set();
}

//----------------------------------------------------------------------------
bool Interface::createMesh(const smtk::mesh::HandleRange& cells,
                           smtk::mesh::Handle& meshHandle)
{
  const unsigned int options = 0;
  ::moab::ErrorCode rval = m_iface->create_meshset( options , meshHandle );
  if(rval == ::moab::MB_SUCCESS)
    {
    m_iface->add_entities( meshHandle, cells );
    m_iface->add_parent_child( m_iface->get_root_set(),
                               meshHandle );
    }
   return (rval == ::moab::MB_SUCCESS);
}

//----------------------------------------------------------------------------
std::size_t Interface::numMeshes(smtk::mesh::Handle handle) const
{
  int num_ents = 0;
  m_iface->get_number_entities_by_type( handle, ::moab::MBENTITYSET, num_ents);
  return static_cast<std::size_t>(num_ents);
}


//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle) const

{
  smtk::mesh::HandleRange range;
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, range);
  return range;
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                                int dimension) const

{
  smtk::mesh::HandleRange all_meshes_with_dim_tag;
  smtk::mesh::HandleRange meshes_of_proper_dim;

  //construct a dim tag that matches the dimension coming in
  tag::QueryDimTag dimTag(dimension, this->moabInterface());

  // get all the entities of that type in the mesh
  m_iface->get_entities_by_type_and_tag(handle,
                                      ::moab::MBENTITYSET,
                                      dimTag.moabTag(),
                                      NULL,
                                      1,
                                      all_meshes_with_dim_tag);

  typedef smtk::mesh::HandleRange::const_iterator iterator;
  for(iterator i=all_meshes_with_dim_tag.begin();
      i != all_meshes_with_dim_tag.end(); ++i)
    {
    int value = 0;
    m_iface->tag_get_data(dimTag.moabTagAsRef(), &(*i), 1, &value);
    if(value == dimTag.value())
      {
      meshes_of_proper_dim.insert(*i);
      }
    }
  return meshes_of_proper_dim;
}

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
smtk::mesh::HandleRange Interface::getMeshsets(smtk::mesh::Handle handle,
                                                const std::string& name) const

{
  typedef std::vector< ::moab::EntityHandle >::const_iterator it;

  //I can't get get_entities_by_type_and_tag to work properly for this
  //query so I am going to do it the slow way by doing the checking manually

  //use a vector since we are going to do single element iteration, and
  //removal.
  std::vector< ::moab::EntityHandle > all_ents;
  std::vector< ::moab::EntityHandle > matching_ents;
  //get all ents
  m_iface->get_entities_by_type(handle, ::moab::MBENTITYSET, all_ents);

  //see which ones have a a matching name, and if so add it
  tag::QueryNameTag query_name(this->moabInterface());
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
smtk::mesh::HandleRange Interface::getCells(const HandleRange &meshsets) const

{
  // get all non-meshset entities in meshset, including in contained meshsets
  typedef smtk::mesh::HandleRange::const_iterator iterator;
  smtk::mesh::HandleRange entitiesCells;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_handle appends to the range given
    m_iface->get_entities_by_handle(*i, entitiesCells, true);
    }
  return entitiesCells;
}


//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type
smtk::mesh::HandleRange Interface::getCells(const HandleRange &meshsets,
                                             smtk::mesh::CellType cellType) const
{
  int moabCellType = smtk::mesh::moab::smtkToMOABCell(cellType);

  smtk::mesh::HandleRange entitiesCells;

  // get all non-meshset entities in meshset of a given cell type
  typedef smtk::mesh::HandleRange::const_iterator iterator;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_type appends to the range given
    m_iface->get_entities_by_type(*i,
                                static_cast< ::moab::EntityType >(moabCellType),
                                entitiesCells,
                                true);
    }
  return entitiesCells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type(s)
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets,
                                             const smtk::mesh::CellTypes& cellTypes) const

{
  const std::size_t cellTypesToFind = cellTypes.count();
  if( cellTypesToFind == cellTypes.size())
    { //if all the cellTypes are enabled we should just use get_cells
      //all() method can't be used as it was added in C++11
    return this->getCells( meshsets );
    }
  else if(cellTypesToFind == 0)
    {
    return smtk::mesh::HandleRange();
    }

  //we now search from highest cell type to lowest cell type adding everything
  //to the range. The reason for this is that ranges perform best when inserting
  //from high to low values
  smtk::mesh::HandleRange entitiesCells;
  for(int i = (cellTypes.size() -1); i >= 0; --i )
    {
    //skip all cell types we don't have
    if( !cellTypes[i] )
      { continue; }

    smtk::mesh::CellType currentCellType = static_cast<smtk::mesh::CellType>(i);

    smtk::mesh::HandleRange cellEnts = this->getCells(meshsets, currentCellType);

    entitiesCells.insert(cellEnts.begin(), cellEnts.end());
    }

  return entitiesCells;
}

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given dimension
smtk::mesh::HandleRange Interface::getCells(const smtk::mesh::HandleRange& meshsets,
                                            smtk::mesh::DimensionType dim) const

{
  const int dimension = static_cast<int>(dim);

  //get all non-meshset entities of a given dimension
  typedef smtk::mesh::HandleRange::const_iterator iterator;
  smtk::mesh::HandleRange entitiesCells;
  for(iterator i = meshsets.begin(); i != meshsets.end(); ++i)
    {
    //get_entities_by_dimension appends to the range given
    m_iface->get_entities_by_dimension(*i, dimension, entitiesCells, true);
    }
  return entitiesCells;
}


//----------------------------------------------------------------------------
std::vector< std::string > Interface::computeNames(const smtk::mesh::HandleRange& meshsets) const
{
  //construct a name tag query helper class
  tag::QueryNameTag query_name(this->moabInterface());

  typedef smtk::mesh::HandleRange::const_iterator it;
  std::set< std::string > unique_names;
  for(it i = meshsets.begin(); i != meshsets.end(); ++i)
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
smtk::mesh::TypeSet Interface::computeTypes(smtk::mesh::Handle handle) const
{
  int numMeshes = 0;
  m_iface->get_number_entities_by_type( handle, ::moab::MBENTITYSET, numMeshes);

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
      int moabEType = smtk::mesh::moab::smtkToMOABCell(ce);

      //some of the cell types that smtk supports moab doesn't support
      //so we can't query on those.
      int num = 0;
      m_iface->get_number_entities_by_type(handle,
                                         static_cast< ::moab::EntityType >(moabEType),
                                         num);
      ctypes[ce] = (num > 0);
      }
    }

  //determine the state of the typeset
  const bool hasMeshes = numMeshes > 0;
  const bool hasCells = ctypes.any();
  return smtk::mesh::TypeSet(ctypes, hasMeshes, hasCells) ;
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::rangeIntersect(const smtk::mesh::HandleRange& a,
                                                  const smtk::mesh::HandleRange& b) const
{
  return ::moab::intersect(a,b);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::rangeDifference(const smtk::mesh::HandleRange& a,
                                                   const smtk::mesh::HandleRange& b) const
{
  return ::moab::subtract(a,b);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::rangeUnion(const smtk::mesh::HandleRange& a,
                                              const smtk::mesh::HandleRange& b) const
{
  return ::moab::unite(a,b);
}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::pointIntersect(const smtk::mesh::HandleRange& a,
                                                   const smtk::mesh::HandleRange& b,
                                                   const smtk::mesh::ContainsFunctor& containsFunctor) const
{
  if(a.empty() || b.empty())
    { //the intersection with nothing is nothing
    return smtk::mesh::HandleRange();
    }

  //first get all the points of a
  smtk::mesh::HandleRange a_points; m_iface->get_connectivity(a, a_points);

  //result storage for creating the range. This is used since inserting
  //into a range is horribly slow
  std::vector< ::moab::EntityHandle > vresult;
  vresult.reserve( b.size() );

  //Some elements (e.g. structured mesh) may not have an explicit connectivity list.
  //we pass storage to the interface so that it can use that memory to construct
  //an explicit connectivity list for us.
  std::vector< ::moab::EntityHandle > storage;

  typedef smtk::mesh::HandleRange::const_iterator c_it;
  for(c_it i = b.begin(); i != b.end(); ++i)
    {
    const ::moab::EntityHandle* connectivity; //handle back to node list
    int num_nodes; //tells us the number of nodes
    const bool corners_only = false; //explicitly state we want all nodes of the cell

    //grab the raw connectivity array so we don't waste any memory
    m_iface->get_connectivity(*i, connectivity, num_nodes, corners_only, &storage);

    //call the contains functor to determine if this cell is considered
    //to be contained by a_points.
    bool contains = containsFunctor(a_points, connectivity, num_nodes);
    if(contains)
      { vresult.push_back( *i ); }
    }

  //now that we have all the cells that are the partial intersection
  smtk::mesh::HandleRange resulting_range;
  smtk::mesh::HandleRange::iterator hint = resulting_range.begin();

  const std::size_t size = vresult.size();
  for(std::size_t i = 0; i < size;)
    {
    std::size_t j;
    for(j = i + 1; j < size && vresult[j] == 1 + vresult[j-1]; j++);
      //empty for loop
    hint = resulting_range.insert( hint, vresult[i], vresult[i] + (j-i-1) );
    i = j;
    }

  return resulting_range;

}

//----------------------------------------------------------------------------
smtk::mesh::HandleRange Interface::pointDifference(const smtk::mesh::HandleRange& a,
                                                    const smtk::mesh::HandleRange& b,
                                                    const smtk::mesh::ContainsFunctor& containsFunctor) const
{
  if(b.empty())
    { //taking the difference from nothing results in nothing
    return smtk::mesh::HandleRange();
    }
  else if(a.empty())
    { //if a is empty that means all b of is the difference
    return b;
    }

  //first get all the points of a
  smtk::mesh::HandleRange a_points; m_iface->get_connectivity(a, a_points);

  //result storage for creating the range. This is used since inserting
  //into a range is horribly slow
  std::vector< ::moab::EntityHandle > vresult;
  vresult.reserve( b.size() );

  //Some elements (e.g. structured mesh) may not have an explicit connectivity list.
  //we pass storage to the interface so that it can use that memory to construct
  //an explicit connectivity list for us.
  std::vector< ::moab::EntityHandle > storage;

  typedef smtk::mesh::HandleRange::const_iterator c_it;
  for(c_it i = b.begin(); i != b.end(); ++i)
    {
    const ::moab::EntityHandle* connectivity; //handle back to node list
    int num_nodes; //tells us the number of nodes
    const bool corners_only = false; //explicitly state we want all nodes of the cell

    //grab the raw connectivity array so we don't waste any memory
    m_iface->get_connectivity(*i, connectivity, num_nodes, corners_only, &storage);

    //call the contains functor to determine if this cell is considered
    //to be contained by a_points. If we aren't contained than we go into
    //the difference result
    bool contains = containsFunctor(a_points, connectivity, num_nodes);
    if(!contains)
      { vresult.push_back( *i ); }
    }

  //now that we have all the cells that are the partial intersection
  smtk::mesh::HandleRange resulting_range;
  smtk::mesh::HandleRange::iterator hint = resulting_range.begin();

  const std::size_t size = vresult.size();
  for(std::size_t i = 0; i < size;)
    {
    std::size_t j;
    for(j = i + 1; j < size && vresult[j] == 1 + vresult[j-1]; j++);
      //empty for loop
    hint = resulting_range.insert( hint, vresult[i], vresult[i] + (j-i-1) );
    i = j;
    }

  return resulting_range;

}

//----------------------------------------------------------------------------
::moab::Interface *const Interface::moabInterface() const
{
  return this->m_iface.get();
}

}
}
}
