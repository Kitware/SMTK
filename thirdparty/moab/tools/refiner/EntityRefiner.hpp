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

/** \class moab::EntityRefiner
  *
  * This is an abstract class that contains the method used for per-entity refinement.
  * Subclasses must implement the pure virtual refine_entity() function and
  * may implement the vertices_per_split() function.
  * This class constructor requires a non-NULL pointer to a mesh so that, given an
  * entity handle, it can look up vertex coordinates and tags to prepare arguments for
  * the refine_entity() method.
  *
  * Although the MeshRefiner class may not initially support it, entity refiners
  * are required to support some level of recursion.
  * The maximum number of recursive calls allowed may be set with
  * EntityRefiner::set_maximum_number_of_subdivisions().
  * As a convenience, some of the framework for recursion is provided by the
  * EntityRefiner class.
  *
  * Specifically, EntityRefiner stores a pair of heap arrays
  * to hold edge midpoint vertex coordinates and tag values pre-allocated to the
  * maximum recursion depth so that no repeated allocation and deallocation
  * needs to take place during refinement.
  * To use these heaps, subclasses should call reset_heap_pointers() upon entry to
  * EntityRefiner::refine_entity().
  * Then, when the edge size evaluator requires an edge to be split, subclasses
  * should call heap_coord_storage() and heap_tag_storage() to obtain pointers as
  * required.
  *
  * \author David Thompson
  * \author Philippe Pebay
  *
  * \date 24 December 2007
  */

/**\class EntityRefinerOutputFunctor
  *
  * This is an abstract class used by EntityRefiner to output entities that are the product of refinement.
  * The parenthesis operator is overloaded with two forms:
  * one used for appending a vertex to an entity,
  * the other used to finalize the creation of the entity by specifying its type.
  *
  * You are also responsible for implementing the map_vertex() function to map an input vertex handle
  * into an output vertex handle (which may then be appended to an entity using the first form
  * of the parenthesis operator above).
  *
  * \author David Thompson
  * \author Philippe Pebay
  *
  * \date 26 December 2007
  */
#ifndef MOAB_ENTITY_REFINER_HPP
#define MOAB_ENTITY_REFINER_HPP

#include "moab/Types.hpp"
#include "moab/Compiler.hpp" // for MB_DLL_EXPORT

#include <vector>

namespace moab {

class Interface;
class EdgeSizeEvaluator;
class RefinerTagManager;

class EntityRefinerOutputFunctor
{
public:
  virtual ~EntityRefinerOutputFunctor() { }
  /// Map an input vertex to the output mesh. This should return the same value when given the same input across multiple calls.
  virtual EntityHandle map_vertex( EntityHandle vhash, const double* vcoords, const void* vtags ) = 0;
  /**\brief Create a new vertex along an edge.
    *
    * @param[in] h0 An edge endpoint handle on the output mesh.
    * @param[in] h1 An edge endpoint handle on the output mesh.
    * @param[in] vcoords The location of the midpoint in world coordinates.
    * @param[in] vtags Field values at the midpoint.
    * @retval    A handle for the midpoint on the output mesh.
    */
  EntityHandle operator () ( EntityHandle h0, EntityHandle h1, const double* vcoords, const void* vtags )
    {
    EntityHandle harr[2];
    harr[0] = h0;
    harr[1] = h1;
    return (*this)( 2, harr, vcoords, vtags );
    }
  /**\brief Create a new vertex on a triangular face.
    *
    * @param[in] h0 A triangle corner handle on the output mesh.
    * @param[in] h1 A triangle corner handle on the output mesh.
    * @param[in] h2 A triangle corner handle on the output mesh.
    * @param[in] vcoords The location of the mid-face point in world coordinates.
    * @param[in] vtags Field values at the mid-face point.
    * @retval    A handle for the mid-face point on the output mesh.
    */
  virtual EntityHandle operator () ( EntityHandle h0, EntityHandle h1, EntityHandle h2, const double* vcoords, const void* vtags )
    {
    EntityHandle harr[3];
    harr[0] = h0;
    harr[1] = h1;
    harr[2] = h2;
    return (*this)( 3, harr, vcoords, vtags );
    }
  /**\brief Create a new vertex along a \f$k\f$-facet.
    *
    * @param[in] nhash The number of corner vertices (i.e, \f$k\f$ ).
    * @param[in] hash An array of corner handles on the output mesh.
    * @param[in] vcoords The location of the new point in world coordinates.
    * @param[in] vtags Field values at the new point.
    * @retval    A handle for the new point on the output mesh.
    */
  virtual EntityHandle operator () ( int nhash, EntityHandle* hash, const double* vcoords, const void* vtags ) = 0;
  /**\brief Append an output vertex to the list of vertices defining a new entity.
    *
    * @param[in] vhash A vertex of the output mesh.
    */
  virtual void operator () ( EntityHandle vhash ) = 0;
  /**\brief Create a new entity from all previously appended output vertices.
    *
    * This resets the list of appended vertices.
    * @param[in] etyp The type of entity to create.
    */
  virtual void operator () ( EntityType etyp ) = 0;
};

class EntityRefiner
{
public:
  EntityRefiner();
  virtual ~EntityRefiner();

  virtual bool prepare( RefinerTagManager* tmgr, EntityRefinerOutputFunctor* ofunc );
  virtual bool refine_entity( EntityType typ, EntityHandle ent ) = 0;
  virtual unsigned long get_heap_size_bound( int max_recursions ) const = 0;

  virtual bool set_edge_size_evaluator( EdgeSizeEvaluator* );
  EdgeSizeEvaluator* get_edge_size_evaluator() { return this->edge_size_evaluator; }

  virtual bool set_output_functor( EntityRefinerOutputFunctor* func_obj );
  EntityRefinerOutputFunctor* get_output_functor() { return this->output_functor; }

  virtual bool set_minimum_number_of_subdivisions( int mn );
  int get_minimum_number_of_subdivisions() const { return this->minimum_number_of_subdivisions; }

  virtual bool set_maximum_number_of_subdivisions( int mx );
  int get_maximum_number_of_subdivisions() const { return this->maximum_number_of_subdivisions; }

protected:
  Interface* mesh_in;
  EdgeSizeEvaluator* edge_size_evaluator;
  EntityRefinerOutputFunctor* output_functor;
  int minimum_number_of_subdivisions;
  int maximum_number_of_subdivisions;
  std::vector<double> coord_heap;
  std::vector<double>::iterator current_coord;
  std::vector<char> tag_heap;
  std::vector<char>::iterator current_tag;

  void update_heap_size();
  void reset_heap_pointers();
  double* heap_coord_storage();
  void* heap_tag_storage();
};

} // namespace moab 

#endif // MOAB_ENTITY_REFINER_HPP
