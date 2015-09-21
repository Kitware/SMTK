#include "MeshOutputFunctor.hpp"

#include "SplitVertices.hpp"
#include "moab/ParallelComm.hpp"
#include "RefinerTagManager.hpp"

#include <iostream>
#include <set>
#include <iterator>
#include <algorithm>

#undef MB_DEBUG

namespace moab {

MeshOutputFunctor::MeshOutputFunctor( RefinerTagManager* tag_mgr )
{
  this->mesh_in  = tag_mgr->get_input_mesh();
  this->mesh_out = tag_mgr->get_output_mesh();
  this->input_is_output = ( this->mesh_in == this->mesh_out );
  this->tag_manager = tag_mgr;
  this->destination_set = 0; // don't place output entities in a set by default.

  // When the input mesh and the output mesh are different, this map serves
  // as a dictionary from input vertices to output vertices.
  this->vertex_map = new SplitVertices<1>( this->tag_manager );

  // Hold information about newly-created vertices on subdivided edges and faces.
  this->split_vertices.resize( 4 );
  this->split_vertices[0] = 0; // Vertices (0-faces) cannot be split
  this->split_vertices[1] = new SplitVertices<1>( this->tag_manager );
  this->split_vertices[2] = new SplitVertices<2>( this->tag_manager );
  this->split_vertices[3] = new SplitVertices<3>( this->tag_manager );

  // Hold information about newly-created mesh entities (other than split vertices)
  // This is necessary in order for global IDs to be assigned consistently across processes.
  this->new_entities.resize( 5 );
  this->new_entities[0] = new EntitySource( 1, this->tag_manager );
  this->new_entities[1] = new EntitySource( 2, this->tag_manager );
  this->new_entities[2] = new EntitySource( 3, this->tag_manager );
  this->new_entities[3] = new EntitySource( 4, this->tag_manager );
  this->new_entities[4] = new EntitySource( 5, this->tag_manager );
}

MeshOutputFunctor::~MeshOutputFunctor()
{
  delete this->vertex_map;
  for ( int i = 1; i < 4; ++ i )
    delete this->split_vertices[i];
  for ( int i = 0; i < 5; ++ i )
    delete this->new_entities[i];
}

void MeshOutputFunctor::print_vert_crud(
    EntityHandle vout, int nvhash, EntityHandle* vhash, const double* vcoords, const void* /*vtags*/ )
{
  std::cout << "+ {";
  for ( int i = 0; i < nvhash; ++ i )
    std::cout << " " << vhash[i];
  std::cout << " } -> " << vout << " ";

  std::cout << "[ " << vcoords[0];
  for ( int i = 1; i < 6; ++i )
    std::cout << ", " << vcoords[i];
  std::cout << " ] ";

#if 0
  double* x = (double*)vtags;
  int* m = (int*)( (char*)vtags + 2 * sizeof( double ) );
  std::cout << "< " << x[0]
    << ", " << x[1];
  for ( int i = 0; i < 4; ++i )
    std::cout << ", " << m[i];
#endif // 0

  std::cout << " >\n";
  //std::cout << "##############################\n";
  //this->mesh_out->list_entities( 0, 1 );
  //std::cout << "##############################\n";
}

void MeshOutputFunctor::assign_global_ids( ParallelComm* comm )
{
  // First, we must gather the number of entities in each
  // partition (for all partitions, not just those resident locally).
  int lnparts = this->proc_partition_counts.size();
  std::vector<unsigned char> lpdefns;
  std::vector<int> lpsizes;
  lpdefns.resize( ProcessSet::SHARED_PROC_BYTES * lnparts );
  lpsizes.resize( lnparts );
#ifdef MB_DEBUG
  std::cout << "**** Partition Counts ****\n";
#endif // MB_DEBUG
  int i = 0;
  std::map<ProcessSet,int>::iterator it;
  for ( it = this->proc_partition_counts.begin(); it != this->proc_partition_counts.end(); ++ it, ++ i )
    {
    for ( int j = 0; j < ProcessSet::SHARED_PROC_BYTES; ++ j )
      lpdefns[ProcessSet::SHARED_PROC_BYTES * i + j] = it->first.data()[j];
    lpsizes[i] = it->second;
#ifdef MB_DEBUG
    std::cout << "Partition " << it->first << ": " << it->second << "\n";
#endif // MB_DEBUG
    }

  if ( ! comm )
    return;

  std::vector<int> nparts;
  std::vector<int> dparts;
  //unsigned long prank = comm->proc_config().proc_rank();
  unsigned long psize = comm->proc_config().proc_size();
  nparts.resize( psize );
  dparts.resize( psize + 1 );
  MPI_Allgather( &lnparts, 1, MPI_INT, &nparts[0], 1, MPI_INT, comm->proc_config().proc_comm() );
  //unsigned long ndefs = 0;
  for ( unsigned long rank = 1; rank <= psize; ++ rank )
    {
    dparts[rank] = nparts[rank - 1] + dparts[rank - 1];
#ifdef MB_DEBUG
    std::cout << "Proc " << rank << ": " << nparts[rank-1] << " partitions, offset: " << dparts[rank] << "\n";
#endif // MB_DEBUG
    }
  std::vector<unsigned char> part_defns;
  std::vector<int> part_sizes;
  part_defns.resize( ProcessSet::SHARED_PROC_BYTES * dparts[psize] );
  part_sizes.resize( dparts[psize] );
  MPI_Allgatherv(
    &lpsizes[0], lnparts, MPI_INT,
    &part_sizes[0], &nparts[0], &dparts[0], MPI_INT, comm->proc_config().proc_comm() );
  for ( unsigned long rank = 0; rank < psize; ++ rank )
    {
    nparts[rank] *= ProcessSet::SHARED_PROC_BYTES;
    dparts[rank] *= ProcessSet::SHARED_PROC_BYTES;
    }
  MPI_Allgatherv(
    &lpdefns[0], ProcessSet::SHARED_PROC_BYTES * lnparts, MPI_UNSIGNED_CHAR,
    &part_defns[0], &nparts[0], &dparts[0], MPI_UNSIGNED_CHAR, comm->proc_config().proc_comm() );

  // Now that we have the number of new entities in every partition, we
  // can deterministically assign the same GID to the same entity even
  // when shared across processors because we have an ordering that is
  // identical on all processes -- the vertex splits.
  for ( int j = 0; j < dparts[psize]; ++ j )
    {
    ProcessSet pset( &part_defns[ProcessSet::SHARED_PROC_BYTES * j] );
    std::map<ProcessSet,int>::iterator mit = this->proc_partition_counts.find( pset );
    if ( mit != this->proc_partition_counts.end() )
      {
#ifdef MB_DEBUG
      std::cout << "Partition " << pset << ( mit->second == part_sizes[j] ? " matches" : " broken" ) << ".\n";
#endif // MB_DEBUG
      }
    else
      {
      this->proc_partition_counts[pset] = part_sizes[j];
      }
    }
  
  std::map<ProcessSet,int> gids;
  std::map<ProcessSet,int>::iterator pcit;
  EntityHandle start_gid = 100; // FIXME: Get actual maximum GID across all processes and add 1
  for ( pcit = this->proc_partition_counts.begin(); pcit != this->proc_partition_counts.end(); ++ pcit )
    {
    gids[pcit->first] = start_gid;
    start_gid += pcit->second;
#ifdef MB_DEBUG
    std::cout << "Partition " << pcit->first << ": " << pcit->second << " # [" << gids[pcit->first] << "]\n";
#endif // MB_DEBUG
    }
  std::vector<SplitVerticesBase*>::iterator vit;
  vit = this->split_vertices.begin();
  ++ vit; // Skip split_vertices[0] since it's empty.
  ++ vit; // Skip split_vertices[1] since those entries already have global IDs... they exist in the input mesh.
  for ( /* skip */; vit != this->split_vertices.end(); ++ vit )
    {
    (*vit)->assign_global_ids( gids );
    }
  std::vector<EntitySource*>::iterator sit;
  for ( sit = this->new_entities.begin(); sit != this->new_entities.end(); ++ sit )
    {
    if ( *sit )
      (*sit)->assign_global_ids( gids );
    }
}

void MeshOutputFunctor::exchange_handles( ParallelComm*  )
{
}

void MeshOutputFunctor::assign_tags( EntityHandle vhandle, const void* vtags )
{
  if ( ! vhandle )
    return; // Ignore bad vertices

  int num_tags = this->tag_manager->get_number_of_vertex_tags();
  Tag tag_handle;
  int tag_offset;
  for ( int i = 0; i < num_tags; ++i )
    {
    this->tag_manager->get_output_vertex_tag( i, tag_handle, tag_offset );
    this->mesh_out->tag_set_data( tag_handle, &vhandle, 1, vtags );
    }
}

EntityHandle MeshOutputFunctor::map_vertex( EntityHandle vhash, const double* vcoords, const void* vtags )
{
  if ( this->input_is_output )
    { // Don't copy the original vertex!
#ifdef MB_DEBUG
    this->print_vert_crud( vhash, 1, &vhash, vcoords, vtags );
#endif // MB_DEBUG
    return vhash;
    }
  EntityHandle vertex_handle;
  bool newly_created = this->vertex_map->find_or_create(
    &vhash, vcoords, vertex_handle, this->proc_partition_counts, false );
  if ( newly_created )
    {
    std::vector<int> gid;
    this->assign_tags( vertex_handle, vtags );
    if ( this->tag_manager->get_input_gids( 1, &vhash, gid ) == MB_SUCCESS )
      {
      this->tag_manager->set_gid( vertex_handle, gid[0] );
      }
    }
  if ( ! vertex_handle )
    {
    std::cerr << "Could not insert vertex into new mesh!\n";
    }
#ifdef MB_DEBUG
  this->print_vert_crud( vertex_handle, 1, &vhash, vcoords, vtags );
  std::cout << "\nMap vert: " << vhash << " to: " << vertex_handle << "\n";
#endif // MB_DEBUG
  return vertex_handle;
}

EntityHandle MeshOutputFunctor::operator () ( int nvhash, EntityHandle* vhash, const double* vcoords, const void* vtags )
{
  EntityHandle vertex_handle;
  if ( nvhash < 4 )
    {
    bool newly_created = this->split_vertices[nvhash]->find_or_create(
      vhash, vcoords, vertex_handle, this->proc_partition_counts, true );
    if ( newly_created )
      {
      this->assign_tags( vertex_handle, vtags );
      }
    if ( ! vertex_handle )
      {
      std::cerr << "Could not insert mid-edge vertex!\n";
      }
#ifdef MB_DEBUG
    std::cout << "(-" << nvhash << "-) ";
    this->print_vert_crud( vertex_handle, nvhash, vhash, vcoords, vtags );
#endif // MB_DEBUG
    }
  else
    {
    vertex_handle = 0;
    std::cerr << "Not handling splits on faces with " << nvhash << " corners yet.\n";
    }
  return vertex_handle;
}

void MeshOutputFunctor::operator () ( EntityHandle h )
{
#ifdef MB_DEBUG
  std::cout << h << " ";
#endif // MB_DEBUG
  if ( ! this->input_is_output )
    {
    // FIXME: Copy to output mesh
    }
  this->elem_vert.push_back( h );
}

void MeshOutputFunctor::operator () ( EntityType etyp )
{
  EntityHandle elem_handle;
  int nconn = this->elem_vert.size();
  bool newly_created = this->new_entities[nconn]->create_element(
    etyp, nconn, &this->elem_vert[0], elem_handle, this->proc_partition_counts );
  if ( newly_created )
    {
#ifdef MB_DEBUG
    std::cout << " *** ";
#endif // MB_DEBUG
    // FIXME: Handle tag assignment for elements as well as vertices
    this->tag_manager->assign_element_tags( elem_handle );
    }
#ifdef MB_DEBUG
  std::cout << "---------> " << elem_handle << " ( " << etyp << " )\n\n";
#endif // MB_DEBUG
  this->elem_vert.clear();
}

} // namespace moab

