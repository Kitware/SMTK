#include "MeshRefiner.hpp"

#include "EdgeSizeEvaluator.hpp"
#include "EntityRefiner.hpp"
#include "moab/Interface.hpp"
#include "RefinerTagManager.hpp"
#include "MeshOutputFunctor.hpp"

#ifdef MOAB_HAVE_MPI
#include "moab/ParallelComm.hpp"
#include "moab_mpi.h"
#else // MOAB_HAVE_MPI
typedef int MPI_Comm;
#endif // MOAB_HAVE_MPI

namespace moab {

/**\brief Construct a mesh refiner.
  * The input and output mesh pointers may be identical.
  * Existing elements will <b>not</b> be removed from the input mesh
  * as they are refined, so the adjacencies for entitities may appear
  * strange after refinement.
  */
MeshRefiner::MeshRefiner( Interface* imesh, Interface* omesh )
{  
  this->mesh_in = imesh;
  this->mesh_out = omesh;
  this->tag_manager = new RefinerTagManager( this->mesh_in, this->mesh_out );
  this->output_functor = new MeshOutputFunctor( this->tag_manager );
  this->entity_refiner = 0;
  this->comm = ParallelComm::get_pcomm( this->mesh_out, 0 );
}

/**\brief Destroy a mesh refiner.
  *
  * Note that any EntityRefiner object associated with the mesh refiner will be deleted inside this destructor.
  * Destruction is virtual so subclasses may clean up after refinement.
  */
MeshRefiner::~MeshRefiner()
{
  delete this->tag_manager;
  delete this->output_functor;
  if ( this->entity_refiner )
    delete this->entity_refiner;
}

/**\brief Specify which techniqe will be used to refine entities in the mesh.
  * The entity refiner object you specify is ``owned'' by the mesh refiner after this call;
  * the entity refiner will be deleted when this mesh refiner is destroyed.
  * You should not delete the entity refiner yourself.
  */
bool MeshRefiner::set_entity_refiner( EntityRefiner* er )
{
  if ( ! er || er == this->entity_refiner )
    return false;

  this->entity_refiner = er;
  return true;
}

/**\brief A convenience method to reset the list of tags to be copied to output vertices.
  * This simply calls the method of the same name on the tag manager.
  */
void MeshRefiner::reset_vertex_tags()
{
  this->tag_manager->reset_vertex_tags();
}

/**\brief A convenience method to add a tag to be copied/interpolated from input vertices to output vertices.
  * This simply calls the method of the same name on the tag manager.
  */
int MeshRefiner::add_vertex_tag( Tag tag_handle )
{
  return this->tag_manager->add_vertex_tag( tag_handle );
}

struct MeshRefinerIterator {
  Range subset;
  EntityHandle destination_set;
};

/**\brief Refine entities in a mesh set.
  * This will recursively descend any mesh sets contained in the \a range.
  * It returns false when not able to refine (because no entity refiner is
  * set or no edge size evaluator has been set on the entity refiner) and
  * true otherwise.
  */
bool MeshRefiner::refine( Range& range )
{
  this->tag_manager->create_output_tags();
  if ( ! this->entity_refiner->prepare( this->tag_manager, this->output_functor ) )
    { // Oops, perhaps the edge_size_evaluator was not set?
    return false;
    }

  MeshRefinerIterator entry;
  std::vector<MeshRefinerIterator> work;

  entry.subset = range;
  entry.destination_set = 0;
  work.push_back( entry );

  while ( ! work.empty() )
    {
    entry = work.back();
    work.pop_back();
    this->output_functor->destination_set = entry.destination_set;
    for ( Range::const_iterator it = entry.subset.begin(); it != entry.subset.end(); ++ it )
      {
      EntityType etyp = this->mesh_in->type_from_handle( *it );
      if ( etyp == MBENTITYSET )
        {
        Range set_ents;
        if ( this->mesh_in->get_entities_by_handle( *it, set_ents, false ) == MB_SUCCESS )
          {
          // Create a matching set on the output mesh.
          MeshRefinerIterator set_work;
          unsigned int set_work_opts;
          this->mesh_in->get_meshset_options( *it, set_work_opts );
          this->mesh_out->create_meshset( set_work_opts, set_work.destination_set );
          set_work.subset = set_ents;
          work.push_back( set_work );
          // Copy any per-element tag values the user has requested to the output set.
          this->tag_manager->set_element_tags_from_ent( *it );
          this->tag_manager->assign_element_tags( set_work.destination_set );
          // Copy the global ID to the new set (assuming it exists).
          this->tag_manager->copy_gid( *it, set_work.destination_set );
          }
        }
      else
        {
        this->tag_manager->set_element_tags_from_ent( *it );
        this->tag_manager->set_element_procs_from_ent( *it );
        this->entity_refiner->refine_entity( etyp, *it );
        }
      }
    }
  this->output_functor->assign_global_ids( this->comm );

  return true;
}

} // namespace moab 
