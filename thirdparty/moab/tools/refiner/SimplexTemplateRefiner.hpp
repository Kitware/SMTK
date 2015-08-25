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

/**\class moab::SimplexTemplateRefiner
  *
  * This is a concrete subclass of EntityRefiner that implements
  * refinement using templates applied to simplices.
  * Entities that are not simplices are divided into tetrahedra,
  * triangles, or lines before being processed.
  * Points are passed through unchanged.
  *
  * \author David Thompson
  * \author Philippe Pebay
  *
  * \date 24 December 2007
  */
#ifndef MB_SIMPLEX_TEMPLATE_REFINER_HPP
#define MB_SIMPLEX_TEMPLATE_REFINER_HPP

#include "moabrefiner_export.h"

#include "EntityRefiner.hpp"
#include "SimplexTemplateTagAssigner.hpp"

#include "moab/Types.hpp" // for MB_DLL_EXPORT

namespace moab { 

class RefinerTagManager;

class MOABREFINER_EXPORT SimplexTemplateRefiner : public EntityRefiner
{
public:
  SimplexTemplateRefiner();
  virtual ~SimplexTemplateRefiner();

  virtual bool refine_entity( EntityType etyp, EntityHandle entity );
  virtual unsigned long get_heap_size_bound( int max_recursions ) const { return 48 * 4 * ( 1 << max_recursions ) + 8; }

  virtual bool set_tag_assigner( SimplexTemplateTagAssigner* ta );
  SimplexTemplateTagAssigner* get_tag_assigner() const { return this->tag_assigner; }

  virtual bool prepare( RefinerTagManager* tmgr, EntityRefinerOutputFunctor* ofunc );

protected:
  SimplexTemplateTagAssigner* tag_assigner;
  RefinerTagManager* tag_manager;
  std::vector<double> corner_coords;
  std::vector<void*> corner_tags;
  std::vector<EntityHandle> corner_handles;
  bool input_is_output;

  static int template_index[64][2];
  static int permutations_from_index[24][14];
  static int templates[];

  void refine_0_simplex( const double* v0, const void* t0, EntityHandle h0 );
  bool refine_1_simplex( int max_depth,
                         const double* v0, const void* t0, EntityHandle h0,
                         const double* v1, const void* t1, EntityHandle h1 );
  bool refine_2_simplex( int max_depth, int move,
                         const double* v0, const void* t0, EntityHandle h0,
                         const double* v1, const void* t1, EntityHandle h1,
                         const double* v2, const void* t2, EntityHandle h2 );
  bool refine_3_simplex( int max_depth,
                         double* v0, void* t0, EntityHandle h0,
                         double* v1, void* t1, EntityHandle h1,
                         double* v2, void* t2, EntityHandle h2,
                         double* v3, void* t3, EntityHandle h3 );

  int best_tets( int* alternates, double*[14], int, int ) { return alternates[0]; }
  void assign_parametric_coordinates( int num_nodes, const double* src, double* tgt );
  static bool compare_Hopf_cross_string_dist( const double* v00, const double* v01, const double* v10, const double* v11 );
};

} // namespace moab 

#endif // MB_SIMPLEX_TEMPLATE_REFINER_HPP

