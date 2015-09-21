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

/**\class moab::MeshRefiner
  *\brief Refine a mesh using a streaming operation.
  *
  * This is an class that contains the method used for mesh refinement.
  *
  * \author Philippe Pebay
  * \author David Thompson
  *
  * \date 19 November 2007
  */
#ifndef MOAB_MESH_REFINER_HPP
#define MOAB_MESH_REFINER_HPP

#include "moabrefiner_export.h"

#include "moab/Compiler.hpp" // for MB_DLL_EXPORT
#include "moab/Range.hpp"

#include <vector>

namespace moab {

class Interface;
class EntityRefiner;
class ParallelComm;
class RefinerTagManager;
class MeshOutputFunctor;

class MOABREFINER_EXPORT MeshRefiner
{
public:
  MeshRefiner( Interface* imesh, Interface* omesh );
  virtual ~MeshRefiner();

  bool set_entity_refiner( EntityRefiner* );
  EntityRefiner* get_entity_refiner() { return this->entity_refiner; }

  bool set_comm( ParallelComm* c ) { if ( ! c || this->comm == c ) return false; this->comm = c; return true; }
  ParallelComm* get_comm() { return this->comm; }

  RefinerTagManager* get_tag_manager() { return this->tag_manager; }
  const RefinerTagManager* get_tag_manager() const { return this->tag_manager; }
  void reset_vertex_tags();
  int add_vertex_tag( Tag tag_handle );

  virtual bool refine( Range& );

protected:
  Interface* mesh_in;
  Interface* mesh_out;
  EntityRefiner* entity_refiner;
  RefinerTagManager* tag_manager;
  MeshOutputFunctor* output_functor;
  ParallelComm* comm;
};

} // namespace moab

#endif // MOAB_MESH_REFINER_HPP
