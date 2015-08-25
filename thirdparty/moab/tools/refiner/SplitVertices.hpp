/*
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2007 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

/**\class moab::SplitVertices
  *\brief A dictionary of new vertices.
  *
  * An array of existing vertex ids used as a key in a dictionary of new vertices.
  */
#ifndef MOAB_SPLIT_VERTICES_HPP
#define MOAB_SPLIT_VERTICES_HPP

#include "moab/Types.hpp"
#include "ProcessSet.hpp"
#include "MBTagConventions.hpp"

#include <map>
#include <vector>
#include <algorithm>

#undef MB_DEBUG

namespace moab { 

class RefinerTagManager;

template< int _n >
class SplitVertexIndex
{
public:
  SplitVertexIndex() { }
  SplitVertexIndex( const int* src )
    { for ( int i = 0; i < _n; ++ i ) this->ids[i] = src[i]; std::sort( this->ids, this->ids + _n ); }
  SplitVertexIndex( const SplitVertexIndex<_n>& src )
    { for ( int i = 0; i < _n; ++ i ) this->ids[i] = src.ids[i]; this->process_set = src.process_set; }
  SplitVertexIndex& operator = ( const SplitVertexIndex<_n>& src )
    { for ( int i = 0; i < _n; ++ i ) this->ids[i] = src.ids[i]; this->process_set = src.process_set; return *this; }

  void set_common_processes( const ProcessSet& procs )
    { this->process_set = procs; }
  ProcessSet& common_processes()
    { return this->process_set; }
  const ProcessSet& common_processes() const
    { return this->process_set; }

  bool operator < ( const SplitVertexIndex<_n>& other ) const
    {
    // Ignore the process set. Only program errors lead to mismatched process sets with identical ids.
    for ( int i = 0; i < _n; ++ i )
      if ( this->ids[i] < other.ids[i] )
        return true;
      else if ( this->ids[i] > other.ids[i] )
        return false;
    return false;
    }

  int ids[_n + 1];
  ProcessSet process_set;
};

template< int _n >
std::ostream& operator << ( std::ostream& os, const SplitVertexIndex<_n>& idx )
{
  for ( int i = 0; i < _n; ++ i )
    {
    os << idx.ids[i] << " ";
    }
  os << "(" << idx.process_set << ")";
  return os;
}

class EntitySourceRecord
{
public:
  EntitySourceRecord() { }
  EntitySourceRecord( int nc, EntityHandle ent, const ProcessSet& procs )
    { this->ids.resize( nc ); this->handle = ent; this->process_set = procs; }
  EntitySourceRecord( const EntitySourceRecord& src )
    { this->handle = src.handle; this->process_set = src.process_set; this->ids = src.ids; }
  EntitySourceRecord& operator = ( const EntitySourceRecord& src )
    { this->handle = src.handle; this->process_set = src.process_set; this->ids = src.ids; return *this; }

  void set_common_processes( const ProcessSet& procs )
    { this->process_set = procs; }
  ProcessSet& common_processes()
    { return this->process_set; }
  const ProcessSet& common_processes() const
    { return this->process_set; }

  bool operator < ( const EntitySourceRecord& other ) const
    {
    //assert( this->ids.size() == other.ids.size() );
    std::vector<int>::size_type N = this->ids.size();
    std::vector<int>::size_type i;
    // Ignore the process set. Only program errors lead to mismatched process sets with identical ids.
    for ( i = 0; i < N; ++ i )
      if ( this->ids[i] < other.ids[i] )
        return true;
      else if ( this->ids[i] > other.ids[i] )
        return false;
    return false;
    }

  std::vector<int> ids;
  ProcessSet process_set;
  EntityHandle handle;
};


/** A non-templated base class that the SplitVertices template subclasses all share.
  *
  * All methods that need to be accessed by other classes should be
  * declared by the base class so that no knowledge of template parameters
  * is required.
  */
class SplitVerticesBase
{
public:
  SplitVerticesBase( RefinerTagManager* tag_mgr );
  virtual ~SplitVerticesBase();

  virtual bool find_or_create(
    const EntityHandle* split_src, const double* coords, EntityHandle& vert_handle,
    std::map<ProcessSet,int>& proc_partition_counts, bool handles_on_output_mesh ) = 0;

  virtual void assign_global_ids( std::map<ProcessSet,int>& gids ) = 0;

  Interface* mesh_out; // Output mesh. Needed for new vertex set in vert_handle
  RefinerTagManager* tag_manager;
  std::vector<int> split_gids; // Used to hold global IDs of split vertices
  ProcessSet common_shared_procs; // Holds intersection of several shared_procs_ins.
};

/** A vector of pre-existing entities to a new mesh entity.
  *
  * This is used as a dictionary to determine whether a new vertex should be
  * created on the given n-simplex (n being the template parameter) or whether
  * it has already been created as part of the refinement of a neighboring entity.
  */
class EntitySource : public std::vector<EntitySourceRecord>
{
public:
  typedef std::vector<EntitySourceRecord> VecType;
  typedef std::vector<EntitySourceRecord>::iterator VecIteratorType;

  EntitySource( int num_corners, RefinerTagManager* tag_mgr );
  ~EntitySource();
  bool create_element(
    EntityType etyp, int nconn, const EntityHandle* split_src, EntityHandle& elem_handle,
    std::map<ProcessSet,int>& proc_partition_counts );

  void assign_global_ids( std::map<ProcessSet,int>& gids );

  Interface* mesh_out; // Output mesh. Needed for new vertex set in vert_handle
  RefinerTagManager* tag_manager;
  ProcessSet common_shared_procs; // Holds intersection of several shared_procs_ins.
  int num_corners;
};


/** A map from a set of pre-existing vertices to a new mesh vertex.
  *
  * This is used as a dictionary to determine whether a new vertex should be
  * created on the given n-simplex (n being the template parameter) or whether
  * it has already been created as part of the refinement of a neighboring entity.
  */
template< int _n >
class SplitVertices : public std::map<SplitVertexIndex<_n>,EntityHandle>, public SplitVerticesBase
{
public:
  typedef std::map<SplitVertexIndex<_n>,EntityHandle> MapType;
  typedef typename std::map<SplitVertexIndex<_n>,EntityHandle>::iterator MapIteratorType;

  SplitVertices( RefinerTagManager* tag_mgr );
  virtual ~SplitVertices();
  virtual bool find_or_create(
    const EntityHandle* split_src, const double* coords, EntityHandle& vert_handle,
    std::map<ProcessSet,int>& proc_partition_counts, bool handles_on_output_mesh );

  virtual void assign_global_ids( std::map<ProcessSet,int>& gids );
};

// ------------------------- Template member definitions ----------------------
template< int _n >
SplitVertices<_n>::SplitVertices( RefinerTagManager* tag_mgr )
  : SplitVerticesBase( tag_mgr )
{
  this->split_gids.resize( _n );
}

template< int _n >
SplitVertices<_n>::~SplitVertices()
{
}

template< int _n >
bool SplitVertices<_n>::find_or_create(
  const EntityHandle* split_src, const double* coords, EntityHandle& vert_handle,
  std::map<ProcessSet,int>& proc_partition_counts, bool handles_on_output_mesh )
{
  // Get the global IDs of the input vertices
  if ( handles_on_output_mesh )
    {
    this->tag_manager->get_output_gids( _n, split_src, this->split_gids );
    }
  else
    {
    this->tag_manager->get_input_gids( _n, split_src, this->split_gids );
    }
  SplitVertexIndex<_n> key( &this->split_gids[0] );
  MapIteratorType it = this->find( key );
  if ( it == this->end() )
    {
#ifdef MB_DEBUG
    std::cout << " wrt output: " << handles_on_output_mesh << " ";
#endif // MB_DEBUG
    this->tag_manager->get_common_processes( _n, split_src, this->common_shared_procs, handles_on_output_mesh );
    proc_partition_counts[this->common_shared_procs]++;
    key.set_common_processes( this->common_shared_procs );
    if ( this->mesh_out->create_vertex( coords + 3, vert_handle ) != MB_SUCCESS )
      {
      return false;
      }
    (*this)[key] = vert_handle;
    this->tag_manager->set_sharing( vert_handle, this->common_shared_procs );
    return true;
    }
  vert_handle = it->second;
  return false;
}

template< int _n >
void SplitVertices<_n>::assign_global_ids( std::map<ProcessSet,int>& gids )
{
  typename std::map<SplitVertexIndex<_n>,EntityHandle>::iterator it;
  for ( it = this->begin(); it != this->end(); ++ it )
    {
    int gid = gids[it->first.process_set] ++;
    this->tag_manager->set_gid( it->second, gid );
#ifdef MB_DEBUG
    std::cout << "Assigning entity: " << it->first << " GID: " << gid << "\n";
#endif // MB_DEBUG
    }
}

} // namespace moab 

#endif /* MOAB_SPLIT_VERTICES_HPP */
