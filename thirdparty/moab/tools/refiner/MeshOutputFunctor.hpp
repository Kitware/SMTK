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

/**\class moab::MeshOutputFunctor
  *\brief Implements the abstract EntityRefinerOutputFunctor class.
  *
  * This class is a concrete implementation of the EntityRefinerOutputFunctor.
  * It creates new vertices and regions in a new or existing mesh as
  * the input entities are streamed through the refiner.
  *
  * \author David Thompson
  * \author Philippe Pebay
  *
  * \date 28 July 2008
  */
#ifndef MOAB_MESH_OUTPUT_FUNCTOR_HPP
#define MOAB_MESH_OUTPUT_FUNCTOR_HPP

#include "moab/Types.hpp"
#include "EntityRefiner.hpp"
#include "ProcessSet.hpp"

#include <vector>
#include <map>

#include <string.h>

namespace moab {

class SplitVerticesBase;
class EntitySource;
class ParallelComm;

class MeshOutputFunctor : public EntityRefinerOutputFunctor
{
public:
  MeshOutputFunctor( RefinerTagManager* tag_mgr );
  ~MeshOutputFunctor();

  void print_vert_crud( EntityHandle vout, int nvhash, EntityHandle* vhash, const double* vcoords, const void* vtags );
  void assign_global_ids( ParallelComm* comm );
  void exchange_handles( ParallelComm* comm );

  void assign_tags( EntityHandle vhandle, const void* vtags );

  virtual EntityHandle map_vertex( EntityHandle vhash, const double* vcoords, const void* vtags );
  using EntityRefinerOutputFunctor::operator();
  virtual EntityHandle operator () ( int nvhash, EntityHandle* vhash, const double* vcoords, const void* vtags );
  virtual void operator () ( EntityHandle h );
  virtual void operator () ( EntityType etyp );

  Interface* mesh_in;
  Interface* mesh_out;
  bool input_is_output;
  SplitVerticesBase* vertex_map;
  std::vector<SplitVerticesBase*> split_vertices;
  std::vector<EntitySource*> new_entities;
  std::vector<EntityHandle> elem_vert;
  RefinerTagManager* tag_manager;
  EntityHandle destination_set;
  std::map<ProcessSet,int> proc_partition_counts;
};

} // namespace moab 

#endif // MOAB_MESH_OUTPUT_FUNCTOR_HPP
