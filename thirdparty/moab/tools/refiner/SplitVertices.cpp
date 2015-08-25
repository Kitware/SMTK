#include "SplitVertices.hpp"
#include "RefinerTagManager.hpp"

#include "MBParallelConventions.h"

namespace moab {

SplitVerticesBase::SplitVerticesBase( RefinerTagManager* tag_mgr )
{
  this->tag_manager = tag_mgr;
  this->mesh_out = tag_mgr->get_output_mesh();
}

SplitVerticesBase::~SplitVerticesBase()
{
}

EntitySource::EntitySource( int nc, RefinerTagManager* tag_mgr )
{
  this->tag_manager = tag_mgr;
  this->mesh_out = tag_mgr->get_output_mesh();
  this->num_corners = nc;
}

EntitySource::~EntitySource()
{
}

bool EntitySource::create_element(
  EntityType etyp, int nconn, const EntityHandle* elem_verts, EntityHandle& elem_handle,
  std::map<ProcessSet,int>& proc_partition_counts )
{
  // Get the global IDs of the input vertices
  //int stat;
  proc_partition_counts[this->tag_manager->get_element_procs()]++;
  if ( this->mesh_out->create_element( etyp, elem_verts, nconn, elem_handle ) != MB_SUCCESS )
    {
    return false;
    }
  this->push_back( EntitySourceRecord( this->num_corners, elem_handle, this->tag_manager->get_element_procs() ) );
  this->tag_manager->set_sharing( elem_handle, this->tag_manager->get_element_procs() );
  return true;
}

void EntitySource::assign_global_ids( std::map<ProcessSet,int>& gids )
{
  std::vector<EntityHandle> adjacencies;
  adjacencies.resize( this->num_corners );
  std::vector<EntitySourceRecord>::iterator it;
  for ( it = this->begin(); it != this->end(); ++ it )
    {
    int num_nodes;
    const EntityHandle* conn;
    this->mesh_out->get_connectivity( it->handle, conn, num_nodes );
    this->tag_manager->get_output_gids( this->num_corners, conn, it->ids );
    std::sort( it->ids.begin(), it->ids.end() );
    }
  std::sort( this->begin(), this->end() );
  for ( it = this->begin(); it != this->end(); ++ it )
    {
    int gid = gids[it->process_set] ++;
    this->tag_manager->set_gid( it->handle, gid );
#ifdef MB_DEBUG
    std::cout << "Assigning entity: " << it->handle << " GID: " << gid << "\n";
#endif // MB_DEBUG
    }
}

} // namespace moab
