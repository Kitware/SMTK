/*
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2004 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

#ifndef MOAB_REFINER_TAG_MANAGER_HPP
#define MOAB_REFINER_TAG_MANAGER_HPP

#include "moab/Types.hpp" // for MB_DLL_EXPORT

#include "ProcessSet.hpp"

#include <vector>

namespace moab {

class Interface;

/**
  * This a class that manages which tags an edge refiner should include
  * on output vertices created during mesh refinement.
  * The 
  *
  * \author David Thompson
  *
  * \date 12 June 2008
  */
class RefinerTagManager
{
public:
  RefinerTagManager( Interface* in_mesh, Interface* out_mesh );
  virtual ~RefinerTagManager();

  void reset_vertex_tags();
  int add_vertex_tag( Tag tag_handle );
  int get_vertex_tag_size() const { return this->vertex_size; }
  int get_number_of_vertex_tags() const { return this->input_vertex_tags.size(); }

  void reset_element_tags();
  int add_element_tag( Tag tag_handle );
  int get_element_tag_size() const { return this->element_size; }
  int get_number_of_element_tags() const { return this->input_element_tags.size(); }

  void create_output_tags();

  void get_input_vertex_tag( int i, Tag& tag, int& byte_offset );
  void get_output_vertex_tag( int i, Tag& tag, int& byte_offset );

  void get_input_element_tag( int i, Tag& tag, int& byte_offset );
  void get_output_element_tag( int i, Tag& tag, int& byte_offset );

  Interface* get_input_mesh() { return this->input_mesh; }
  Interface* get_output_mesh() { return this->output_mesh; }

  Tag input_parallel_status() { return this->tag_ipstatus; }
  Tag input_shared_proc() { return this->tag_ipsproc; }
  Tag input_shared_procs() { return this->tag_ipsprocs; }

  int get_input_gids( int n, const EntityHandle* ents, std::vector<int>& gids );
  int get_output_gids( int n, const EntityHandle* ents, std::vector<int>& gids );
  int set_gid( EntityHandle ent, int gid );
  int copy_gid( EntityHandle ent_input, EntityHandle ent_output );

  void set_sharing( EntityHandle ent_handle, ProcessSet& procs );
  void get_common_processes( int num, const EntityHandle* src, ProcessSet& common_shared_procs, bool on_output_mesh = true );

  void set_element_tags_from_ent( EntityHandle ent_input );
  void assign_element_tags( EntityHandle ent_output );

  void set_element_procs_from_ent( EntityHandle ent_input )
    {
    this->get_common_processes( 1, &ent_input, this->current_element_procs, false );
    }
  ProcessSet& get_element_procs()
    {
    return this->current_element_procs;
    }
  void set_element_sharing( EntityHandle ent_output )
    {
    this->set_sharing( ent_output, this->current_element_procs );
    }

protected:
  void create_tag_internal( Tag, int );

  std::vector< std::pair< Tag, int > > input_vertex_tags;
  std::vector< std::pair< Tag, int > > output_vertex_tags;
  std::vector< std::pair< Tag, int > > input_element_tags;
  std::vector< std::pair< Tag, int > > output_element_tags;
  int vertex_size;
  int element_size;
  Interface* input_mesh;
  Interface* output_mesh;
  Tag tag_ipstatus; // Handle for PARALLEL_STATUS on mesh_in
  Tag tag_ipsprocs; // Handle for PARALLEL_SHARED_PROCS on mesh_in
  Tag tag_ipsproc;  // Handle for PARALLEL_SHARED_PROC on mesh_in
  Tag tag_ipshands; // Handle for PARALLEL_SHARED_HANDLES on mesh_in
  Tag tag_ipshand;  // Handle for PARALLEL_SHARED_HANDLE on mesh_in
  Tag tag_igid;     // Handle for global IDs on mesh_in
  Tag tag_opstatus; // Handle for PARALLEL_STATUS on mesh_out
  Tag tag_opsprocs; // Handle for PARALLEL_SHARED_PROCS on mesh_out
  Tag tag_opsproc;  // Handle for PARALLEL_SHARED_PROC on mesh_out
  Tag tag_opshands; // Handle for PARALLEL_SHARED_HANDLES on mesh_out
  Tag tag_opshand;  // Handle for PARALLEL_SHARED_HANDLE on mesh_out
  Tag tag_ogid;     // Handle for global IDs on mesh_out
  int rank;
  std::vector<int> shared_procs_in; // Used to hold procs sharing an input vert.
  std::vector<int> shared_procs_out; // Used to hold procs sharing an output entity.
  ProcessSet current_shared_procs; // Holds process list as it is being accumulated
  ProcessSet current_element_procs; // The list of processes which should share an output element.
  std::vector<char> element_tag_data; // Holds tag data for per-element tags
};

} // namespace moab

#endif // MOAB_REFINER_TAG_MANAGER_HPP
