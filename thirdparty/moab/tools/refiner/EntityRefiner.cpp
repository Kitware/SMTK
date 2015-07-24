#include "EntityRefiner.hpp"

#include "EdgeSizeEvaluator.hpp"
#include "moab/Interface.hpp"

namespace moab {

/// Construct an entity refiner.
EntityRefiner::EntityRefiner()
{  
  this->mesh_in = 0;
  this->edge_size_evaluator = 0;
  this->output_functor = 0;
  // By default, allow at most one subdivision per edge
  this->minimum_number_of_subdivisions = 0;
  this->maximum_number_of_subdivisions = 1;
}

/// Destruction is virtual so subclasses may clean up after refinement.
EntityRefiner::~EntityRefiner()
{
  if ( this->edge_size_evaluator )
    delete this->edge_size_evaluator;
}

/**\brief Prepare to start refining entities on a given mesh.
  * This is called once before refine_entity() is ever invoked.
  * The tag manager specifies the input and output meshes upon which the refiner will act.
  *
  * This function returns false if calling refine_entity() immediately
  * afterwards would cause a failure (due, for example, to a NULL edge_size_evaluator).
  * Otherwise it returns true.
  */
bool EntityRefiner::prepare( RefinerTagManager* tmgr, EntityRefinerOutputFunctor* ofunc )
{
  bool rval = true;
  if ( this->edge_size_evaluator )
    {
    this->edge_size_evaluator->set_tag_manager( tmgr );
    }
  else
    {
    rval = false;
    }
  this->set_output_functor( ofunc );
  this->mesh_in = tmgr->get_input_mesh();
  this->update_heap_size();
  return rval;
}

/**\fn bool EntityRefiner::refine_entity( EntityHandle )
  *\brief Method implemented by subclasses to create decompositions of entities using edge subdivisions.
  */

/**\fn int EntityRefiner::get_heap_size_bound( int max_recursions ) const
  *\brief When an entity is refined, what is the maximum number of new vertices that will be created?
  *
  * This must be the maximum number of initial corner vertices for any entity type (hex, tet, etc.)
  * to be processed plus the maximum number of new vertices that might be created at edge or face
  * mid-points during the refinement of a single entity.
  */

/**\brief Set the object that specifies which edges of a given entity should be subdivided.
  *
  * The entity refiner takes ownership of edge size evaluator and will delete it when
  * a new value is set or when the entity refiner is destroyed.
  *
  * @param ese The new edge size evaluator object.
  * @retval Returns true if the value was changed and false otherwise.
  */
bool EntityRefiner::set_edge_size_evaluator( EdgeSizeEvaluator* ese )
{
  if ( ! ese || ese == this->edge_size_evaluator )
    return false;

  if ( this->edge_size_evaluator )
    {
    delete this->edge_size_evaluator;
    }
  this->edge_size_evaluator = ese;

  return true;
}

/**\fn EdgeSizeEvaluator* EntityRefiner::get_edge_size_evaluator()
  *\brief Return a pointer to the object that specifies which edges of a given entity should be subdivided.
  *
  * This may return NULL if no value has been previously specified.
  *
  * @retval A pointer to an edge size evaluator object or NULL.
  */

/**\brief Set the functor through which output entities are streamed.
  *
  * Any previously assigned functor will be deleted when a new functor is set.
  * 
  * @retvalReturns true if the value was changed and false otherwise.
  */
bool EntityRefiner::set_output_functor( EntityRefinerOutputFunctor* func_obj )
{
  if ( ! func_obj || func_obj == this->output_functor )
    return false;

  if ( this->output_functor )
    {
    delete this->output_functor;
    }
  this->output_functor = func_obj;
  return true;
}

/**\fn EntityRefinerOutputFunctor* EntityRefiner::get_output_functor()
  *\brief Return the functor used to stream output.
  *
  * @retval A pointer to the functor. This may be NULL.
  */

/**\brief Set the minimum number of recursive subdivisions that should occur, regardless of the edge_size_evaluator's response.
  *
  * This is useful for forcing global refinement.
  *
  * @retval True if the number of subdivisions was changed; false otherwise.
  */
bool EntityRefiner::set_minimum_number_of_subdivisions( int mn )
{
  if ( mn < 0 || mn == this->minimum_number_of_subdivisions )
    {
    return false;
    }

  this->minimum_number_of_subdivisions = mn;
  return true;
}

/**\fn int EntityRefiner::get_minimum_number_of_subdivisions() const
  *\brief Return the minimum number of recursive edge subdivisions guaranteed to take place, regardless of the edge size evaluator.
  *
  * This may any non-negative integer.
  *
  * @retval The guaranteed minimum number of subdivisions that will take place on each and every edge of the mesh.
  */

/**\brief Set the maximum number of recursive subdivisions that should occur, regardless of the edge_size_evaluator's response.
  *
  * This is useful for preventing infinite recursion.
  * A value of 0 is allowed although not terribly practical.
  *
  * @retval True if the number of subdivisions was changed; false otherwise.
  */
bool EntityRefiner::set_maximum_number_of_subdivisions( int mx )
{
  if ( mx < 0 || mx == this->maximum_number_of_subdivisions )
    {
    return false;
    }

  this->maximum_number_of_subdivisions = mx;
  this->update_heap_size();
  return true;
}

/**\fn int EntityRefiner::get_maximum_number_of_subdivisions() const
  *\brief Return the maximum number of recursive edge subdivisions guaranteed to take place, regardless of the edge size evaluator.
  *
  * This may any non-negative integer.
  *
  * @retval The guaranteed maximum number of subdivisions that will take place on each and every edge of the mesh.
  */

/**\brief This is called when the edge size evaluator or maximum number of subdivisions is changed
  *       to make sure the heaps are properly sized.
  *
  * Tag heap size cannot be computed if the edge_size_evaluator is NULL.
  */
void EntityRefiner::update_heap_size()
{
  unsigned long n = this->get_heap_size_bound( this->maximum_number_of_subdivisions );
  this->coord_heap.resize( 6 * n );
  if ( this->edge_size_evaluator )
    {
    unsigned long m = this->edge_size_evaluator->get_tag_manager()->get_vertex_tag_size();
    this->tag_heap.resize( m * n );
    }
}

/**\brief Subclasses should call this on entry to refine_entity().
  *
  * When called, future calls to heap_coord_storage() and heap_tag_storage() will
  * re-use the allocated storage starting from the beginning.
  */
void EntityRefiner::reset_heap_pointers()
{
  this->current_coord = this->coord_heap.begin();
  this->current_tag = this->tag_heap.begin();
}

/**\brief Return a pointer to temporary storage for edge midpoint vertex coordinates inside refine_entity().
  *
  * The returned pointer references 6 uninitialized double values to hold parametric coordinates and world coordinates.
  */
double* EntityRefiner::heap_coord_storage()
{
  double* rval;
  if ( this->current_coord != this->coord_heap.end() )
    {
    rval = &(*this->current_coord);
    this->current_coord += 6;
    }
  else
    {
    rval = 0;
    }
  return rval;
}

/**\brief Return a pointer to temporary storage for edge midpoint vertex coordinates inside refine_entity().
  *
  * The returned pointer references enough bytes to store all the tags for a vertex as reported by the
  * current edge size evaluator's EdgeSizeEvaluator::get_vertex_tag_size().
  */
void* EntityRefiner::heap_tag_storage()
{
  void* rval;
  if ( this->edge_size_evaluator && this->current_tag != this->tag_heap.end() )
    {
    rval = (void*) &(*this->current_tag);
    this->current_tag += this->edge_size_evaluator->get_tag_manager()->get_vertex_tag_size();
    }
  else
    {
    rval = 0;
    }
  return rval;
}

} // namespace moab
