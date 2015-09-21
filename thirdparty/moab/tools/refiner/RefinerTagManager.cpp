#include "RefinerTagManager.hpp"

#include "moab/Interface.hpp"
#include "moab/ParallelComm.hpp"
#include "MBParallelConventions.h"
#include "MBTagConventions.hpp"

#include <iostream>
#include <stdexcept>
#include <assert.h>

#undef MB_DEBUG

namespace moab {

/// Construct an evaluator.
RefinerTagManager::RefinerTagManager( Interface* in_mesh, Interface* out_mesh )
  : shared_procs_in( 5 * MAX_SHARING_PROCS, -1 ), shared_procs_out( MAX_SHARING_PROCS, -1 )
{
  assert( in_mesh );
  if ( ! out_mesh )
    out_mesh = in_mesh;

  this->input_mesh = in_mesh;
  this->output_mesh = out_mesh;
  this->reset_vertex_tags();
  this->reset_element_tags();
  ParallelComm* ipcomm = ParallelComm::get_pcomm( this->input_mesh, 0 );
  ParallelComm* opcomm = 0;
  if ( this->output_mesh != this->input_mesh )
    {
    opcomm = ParallelComm::get_pcomm( this->output_mesh, 0 );
    if ( ! opcomm )
      {
#ifdef MB_DEBUG
      std::cout << "Creating opcomm: " << opcomm << "\n";
#endif // MB_DEBUG
      opcomm = new ParallelComm( this->output_mesh, MPI_COMM_WORLD );
      }
    }
  else
    {
    opcomm = ipcomm;
    }

  if ( ipcomm )
    {
    ipcomm->get_shared_proc_tags(
      this->tag_ipsproc, this->tag_ipsprocs,
      this->tag_ipshand, this->tag_ipshands,
      this->tag_ipstatus );
    }
  else
    {
    this->tag_ipsproc = this->tag_ipsprocs = 0;
    this->tag_ipshand = this->tag_ipshands = 0;
    this->tag_ipstatus = 0;
    }

  if ( opcomm )
    {
    opcomm->get_shared_proc_tags(
      this->tag_opsproc, this->tag_opsprocs,
      this->tag_opshand, this->tag_opshands,
      this->tag_opstatus );
    }
  else
    {
    this->tag_opsproc = this->tag_opsprocs = 0;
    this->tag_opshand = this->tag_opshands = 0;
    this->tag_opstatus = 0;
    }

  this->rank =
    ipcomm ? ipcomm->proc_config().proc_rank() :
    ( opcomm ? opcomm->proc_config().proc_rank() : 0 );

  // Create the mesh global ID tags if they aren't already there.
  int zero = 0;
  ErrorCode result;
  result = this->input_mesh->tag_get_handle(
    GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, this->tag_igid, MB_TAG_DENSE|MB_TAG_CREAT, &zero );
  if ( result != MB_SUCCESS )
    {
    throw new std::logic_error( "Unable to find input mesh global ID tag \"" GLOBAL_ID_TAG_NAME "\"" );
    }
  result = this->output_mesh->tag_get_handle(
    GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, this->tag_ogid, MB_TAG_DENSE|MB_TAG_CREAT, &zero );
  if ( result != MB_SUCCESS )
    {
    throw new std::logic_error( "Unable to find/create output mesh global ID tag \"" GLOBAL_ID_TAG_NAME "\"" );
    }

#ifdef MB_DEBUG
  std::cout
    << "psproc:  " << this->tag_ipsproc  << ", " << this->tag_opsproc << "\n"
    << "psprocs: " << this->tag_ipsprocs << ", " << this->tag_opsprocs << "\n"
    << "pshand:  " << this->tag_ipshand  << ", " << this->tag_opshand << "\n"
    << "pshands: " << this->tag_ipshands << ", " << this->tag_opshands << "\n"
    << "pstatus: " << this->tag_ipstatus << ", " << this->tag_opstatus << "\n"
    << "gid:     " << this->tag_igid     << ", " << this->tag_ogid     << "\n";
#endif // MB_DEBUG
}

/// Destruction is virtual so subclasses may clean up after refinement.
RefinerTagManager::~RefinerTagManager()
{
}

/**\fn bool RefinerTagManager::evaluate_edge( \
  *         const double* p0, const void* t0, double* p1, void* t1, const double* p2, const void* t2 )
  *\brief Returns true if the edge \a p0 - \a p2 should be subdivided, false otherwise.
  *
  * The arguments \a p0, \a p1, and \a p2 are all pointers to arrays of 6 doubles each
  * while the arguments \a t0, \a t1, and \a t2 are all pointers to arrays of tag data
  * defined at the corresponding point. While the endpoints \a p0 and \a p2 are
  * immutable, the mid-edge point coordinates \a p1 and tag data \a t1 may be altered by
  * evaluate_edge(). Altered values will be ignored if evaluate_edge() returns false.
  * Be careful to ensure that all calls to evaluate_edge() perform identical modifications
  * given identical input values!
  *
  * A list of tags passed in \a t0, \a t1, and \a t2 is stored in the \a input_vertex_tags member.
  * (for tag handles defined on the input mesh) and the \a output_vertex_tags (for the same tag handles
  * defined on the output mesh).
  * The vertex_size member stores the total length of data associated with each pointer (in bytes).
  * Subclasses may access input_vertex_tags, output_vertex_tags, and vertex_size directly;
  * the refiner uses public methods to set them before any entities are evaluated for subdivision.
  * The output_vertex_tags member is populated when the refiner calls create_output_tags().
  * When the input mesh and output mesh pointers are identical, this simply copies input_vertex_tags
  * to output_vertex_tags.
  * When the pointers are distinct, tags are created on the output mesh.
  */

/// Clear the list of tag values that will appear past the vertex coordinates in \a p0, \a p1, and \a p2.
void RefinerTagManager::reset_vertex_tags()
{
  this->vertex_size = 0;
  this->input_vertex_tags.clear();
  this->output_vertex_tags.clear();
}

/** Add a tag to the list of tag values that will appear past the vertex coordinates.
  * The return value is the offset into each vertex coordinate pointer (\a p0, \a p1, \a p2) where the
  * tag value(s) will be stored.
  */
int RefinerTagManager::add_vertex_tag( Tag tag_handle )
{
  int offset = this->vertex_size; // old size is offset of tag being added
  int tag_size;
  TagType tagType;
  if ( this->input_mesh->tag_get_bytes( tag_handle, tag_size ) != MB_SUCCESS )
    return -1;

  if ( this->input_mesh->tag_get_type( tag_handle, tagType ) != MB_SUCCESS )
    return -1;

  if ( tagType == MB_TAG_BIT )
    {
    // Pad any bit tags to a size in full bytes.
    tag_size = ( tag_size % 8 ? 1 : 0 ) + ( tag_size / 8 );
    }

  // Now pad so that the next tag will be word-aligned:
  while ( tag_size % sizeof(int) )
    ++tag_size;

  this->vertex_size += tag_size;

  this->input_vertex_tags.push_back( std::pair< Tag, int >( tag_handle, offset ) );
  return offset;
}

/**\fn int RefinerTagManager::get_vertex_tag_size()
  *\brief Return the number of bytes to allocate for tag data per point.
  */

/**\fn int RefinerTagManager::get_number_of_vertex_tags() const
  *\brief Return the number of tags that will be output with each new vertex.
  */

/// Clear the list of tag values that will appear past the element coordinates in \a p0, \a p1, and \a p2.
void RefinerTagManager::reset_element_tags()
{
  this->element_size = 0;
  this->input_element_tags.clear();
  this->output_element_tags.clear();
  this->element_tag_data.clear();
}

/** Add a tag to the list of tag values that will appear past the element coordinates.
  * The return value is the offset into each element coordinate pointer (\a p0, \a p1, \a p2) where the
  * tag value(s) will be stored.
  */
int RefinerTagManager::add_element_tag( Tag tag_handle )
{
  int offset = this->element_size; // old size is offset of tag being added
  int tag_size;
  TagType tagType;
  if ( this->input_mesh->tag_get_bytes( tag_handle, tag_size ) != MB_SUCCESS )
    return -1;

  if ( this->input_mesh->tag_get_type( tag_handle, tagType ) != MB_SUCCESS )
    return -1;

  if ( tagType == MB_TAG_BIT )
    {
    // Pad any bit tags to a size in full bytes.
    tag_size = ( tag_size % 8 ? 1 : 0 ) + ( tag_size / 8 );
    }

  // Now pad so that the next tag will be word-aligned:
  while ( tag_size % sizeof(int) )
    ++tag_size;

  this->element_size += tag_size;
  this->element_tag_data.resize( this->element_size );

  this->input_element_tags.push_back( std::pair< Tag, int >( tag_handle, offset ) );
  return offset;
}

/**\fn int RefinerTagManager::get_element_tag_size()
  *\brief Return the number of bytes to allocate for tag data per point.
  */

/**\fn int RefinerTagManager::get_number_of_element_tags() const
  *\brief Return the number of tags that will be output with each new element.
  */

/**\brief Populate the list of output tags to match the list of input tags.
  *
  * When the input mesh and output mesh pointers are identical, this simply copies the list of input tags.
  * When the two meshes are distinct, the corresponding tags are created on the output mesh.
  */
void RefinerTagManager::create_output_tags()
{
  if ( this->input_mesh == this->output_mesh )
    {
    this->output_vertex_tags = this->input_vertex_tags;
    this->output_element_tags = this->input_element_tags;
    return;
    }

  std::vector< std::pair< Tag, int > >::iterator it;
  for ( it = this->input_vertex_tags.begin(); it != this->input_vertex_tags.end(); ++ it )
    {
    this->create_tag_internal( it->first, it->second );
    }
}

/**\brief Return the tag handle and its offset in the array of tag data of each vertex.
  *
  * @param[in] i An index into the list of tags for the vertex.
  * @param[out] tag The tag handle on the input mesh for the $i$-th vertex tag.
  * @param[out] byte_offset The offset (in bytes) of the start of this tag's data in a vertex tag record.
  */
void RefinerTagManager::get_input_vertex_tag( int i, Tag& tag, int& byte_offset )
{
  std::vector< std::pair< Tag, int > >::iterator it = this->input_vertex_tags.begin() + i;
  tag = it->first;
  byte_offset = it->second;
}

/**\brief Return the tag handle and its offset in the array of tag data of each vertex.
  *
  * @param[in] i An index into the list of tags for the vertex.
  * @param[out] tag The tag handle on the output mesh for the $i$-th vertex tag.
  * @param[out] byte_offset The offset (in bytes) of the start of this tag's data in a vertex tag record.
  */
void RefinerTagManager::get_output_vertex_tag( int i, Tag& tag, int& byte_offset )
{
  std::vector< std::pair< Tag, int > >::iterator it = this->output_vertex_tags.begin() + i;
  tag = it->first;
  byte_offset = it->second;
}

/**\brief Retrieve the global ID of each input entity and push it onto the output vector.
  *
  * The \a gids array is emptied by this call before any new values are added.
  * Note that this routine fetches global IDs from the input mesh, not the output mesh;
  * your entity handles must be from the input mesh.
  *
  * @param[in] ents An array of entities in the input mesh whose global IDs you desire
  * @param[in] n The number of entities in the \a ents array.
  * @param[out] gids A vector to contain the resulting global IDs.
  * @retval A MOAB error code as supplied by the Interface::tag_get_data() call.
  */
int RefinerTagManager::get_input_gids( int n, const EntityHandle* ents, std::vector<int>& gids )
{
  int stat=0;
  gids.clear();
  for ( int i = 0; i < n; ++ i )
    {
    int gid = -1;
    stat |= this->input_mesh->tag_get_data( this->tag_igid, ents + i, 1, &gid );
    gids.push_back( gid );
    }
  return stat;
}

/**\brief Retrieve the global ID of each output entity and push it onto the output vector.
  *
  * The \a gids array is emptied by this call before any new values are added.
  * Note that this routine fetches global IDs from the output mesh, not the input mesh;
  * your entity handles must be from the output mesh.
  * Also, be aware that many output entities will not have global IDs assigned;
  * only those vertices which exist in the input mesh are guaranteed to have global IDs
  * assigned to them -- vertices that only exist in the output mesh and all higher-dimensional
  * output entities have no global IDs assigned until after a complete subdivision pass has been made.
  *
  * @param[in] ents An array of entities in the output mesh whose global IDs you desire
  * @param[in] n The number of entities in the \a ents array.
  * @param[out] gids A vector to contain the resulting global IDs.
  * @retval A MOAB error code as supplied by the Interface::tag_get_data() call.
  */
int RefinerTagManager::get_output_gids( int n, const EntityHandle* ents, std::vector<int>& gids )
{
  int stat=0;
  gids.clear();
  for ( int i = 0; i < n; ++ i )
    {
    int gid = -1;
    stat |= this->output_mesh->tag_get_data( this->tag_ogid, ents + i, 1, &gid );
    gids.push_back( gid );
    }
  return stat;
}

/**\brief Assign a global ID to an output entity.
  *
  * @param[in] ent The entity whose ID will be set
  * @param[out] id The global ID
  * @retval An error code as returned by Interface::tag_set_data().
  */
int RefinerTagManager::set_gid( EntityHandle ent, int gid )
{
  return this->output_mesh->tag_set_data( this->tag_ogid, &ent, 1, &gid );
}

/**\brief Copy a global ID from an entity of the input mesh to an entity of the output mesh.
  *
  * @param[in] ent_input An entity on the input mesh with a global ID.
  * @param[in] ent_output An entity on the output mesh whose global ID should be set.
  * @retval               Normally MB_SUCCESS, but returns other values if tag_get_data or tag_set_data fail.
  */
int RefinerTagManager::copy_gid( EntityHandle ent_input, EntityHandle ent_output )
{
  int gid = -1;
  int status;
  if ( ( status =  this->input_mesh->tag_get_data( this->tag_igid, &ent_input, 1, &gid ) ) == MB_SUCCESS )
    {
    status = this->output_mesh->tag_set_data( this->tag_ogid, &ent_output, 1, &gid );
    }
  return status;
}

/**\brief Set parallel status and sharing process list on an entity.
  *
  * This sets tag values for the PARALLEL_STATUS and one of PARALLEL_SHARED_PROC or PARALLEL_SHARED_PROCS tags
  * if \a procs contains any processes (the current process is assumed <b>not</b> to be set in \a procs).
  *
  * @param[in] ent_handle The entity whose information will be set
  * @param[in] procs The set of sharing processes.
  */
void RefinerTagManager::set_sharing( EntityHandle ent_handle, ProcessSet& procs )
{
  int pstat;
  if ( procs.get_process_members( this->rank, this->shared_procs_out ) )
    pstat = PSTATUS_SHARED | PSTATUS_INTERFACE;
  else
    pstat = PSTATUS_SHARED | PSTATUS_INTERFACE | PSTATUS_NOT_OWNED;
  if ( this->shared_procs_out[0] >= 0 )
    {
    // assert( MAX_SHARING_PROCS > 1 );
    // Since get_process_members pads to MAX_SHARING_PROCS, this will be work:
    if ( this->shared_procs_out[1] <= 0 )
      {
      //std::cout << "  (proc )";
      this->output_mesh->tag_set_data( this->tag_opsproc, &ent_handle, 1, &this->shared_procs_out[0] );
      this->output_mesh->tag_set_data( this->tag_opstatus, &ent_handle, 1, &pstat );
      }
    else
      {
      //std::cout << "  (procS)";
      this->output_mesh->tag_set_data( this->tag_opsprocs, &ent_handle, 1, &this->shared_procs_out[0] );
      this->output_mesh->tag_set_data( this->tag_opstatus, &ent_handle, 1, &pstat );
      }
    }
  else
    {
    //std::cout << "  (none )";
    }
  //std::cout << " new pstat: " << pstat << "\n";
}

/**\brief Determine the subset of processes which all share the specified entities.
  *
  * This is used to determine which processes an output entity should reside on when
  * it is defined using several input entities (such as vertices).
  */
void RefinerTagManager::get_common_processes(
  int num, const EntityHandle* src, ProcessSet& common_shared_procs, bool on_output_mesh )
{
  Interface* mesh;
  Tag psproc;
  Tag psprocs;
  if ( on_output_mesh )
    {
    mesh = this->output_mesh;
    psproc = this->tag_opsproc;
    psprocs = this->tag_opsprocs;
    }
  else
    {
    mesh = this->input_mesh;
    psproc = this->tag_ipsproc;
    psprocs = this->tag_ipsprocs;
    }
  bool first_ent = true;
  common_shared_procs.clear();
  for ( int i = 0; i < num; ++ i )
    {
    EntityHandle ent_in = src[i];
    //std::cout << "<(" << ent_in << ")>";
    int stat;
    bool got = false;
    this->current_shared_procs.clear();
    stat = mesh->tag_get_data( psproc, &ent_in, 1, &this->shared_procs_in[0] );
    if ( stat == MB_SUCCESS && this->shared_procs_in[0] != -1 )
      {
      got = true;
      //std::cout << " s" << this->rank << " s" << this->shared_procs_in[0] << " | ";
      this->shared_procs_in[1] = -1;
      }
    stat = mesh->tag_get_data( psprocs, &ent_in, 1, &this->shared_procs_in[0] );
    if ( stat == MB_SUCCESS && this->shared_procs_in[0] != -1 )
      {
      got = true;
      /*
      int i;
      for ( i = 0; i < MAX_SHARING_PROCS && this->shared_procs_in[i] != -1; ++ i )
        std::cout << " m" << this->shared_procs_in[i];
      std::cout << " | ";
      */
      }
    if ( got )
      {
      this->current_shared_procs.set_process_members( this->shared_procs_in );
      this->current_shared_procs.set_process_member( this->rank );
      if ( first_ent )
        {
        common_shared_procs.unite( this->current_shared_procs );
        first_ent = false;
        }
      else
        {
        common_shared_procs.intersect( this->current_shared_procs );
        }
      }
    else
      {
      // not shared, but everthing exists on this process, so make sure that bit is set...
      common_shared_procs.set_process_member( this->rank );
      }
    }
#ifdef MB_DEBUG
  std::cout << "    Common procs " << common_shared_procs;
  std::cout << "\n";
#endif // MB_DEBUG
}

void RefinerTagManager::create_tag_internal( Tag tag_in, int offset )
{
  std::pair< Tag, int > tag_rec;
  std::vector< char > tag_default;
  std::string tag_name;
  TagType tag_type;
  DataType tag_data_type;
  int tag_size;

  tag_rec.second = offset;
  this->input_mesh->tag_get_name( tag_in, tag_name );
  this->input_mesh->tag_get_bytes( tag_in, tag_size );
  this->input_mesh->tag_get_type( tag_in, tag_type );
  this->input_mesh->tag_get_data_type( tag_in, tag_data_type );
  this->input_mesh->tag_get_default_value( tag_in, (void*) &tag_default[0] );
  tag_default.resize( tag_size );
  ErrorCode res = this->output_mesh->tag_get_handle(
    tag_name.c_str(), tag_size, tag_data_type, tag_rec.first, tag_type|MB_TAG_BYTES|MB_TAG_EXCL, &tag_default[0] );
#ifdef MB_DEBUG
  std::cout
    << "Creating output tag: \"" << tag_name.c_str() << "\" handle: " << tag_rec.first
    << " input handle: " << tag_in << "\n";
#endif // MB_DEBUG
  if ( res == MB_FAILURE )
    {
    std::cerr
      << "Could not create output tag name: \"" << tag_name.c_str() << "\" type: "
      << tag_type << " data type: " << tag_data_type << "\n";
    }
  else
    {
    this->output_vertex_tags.push_back( tag_rec );
    }
}

void RefinerTagManager::set_element_tags_from_ent( EntityHandle ent_input )
{
  std::vector< std::pair< Tag, int > >::iterator it;
  for ( it = this->input_element_tags.begin(); it != this->input_element_tags.end(); ++ it )
    {
    this->input_mesh->tag_get_data( it->first, &ent_input, 1, &this->element_tag_data[it->second] );
    }
}

void RefinerTagManager::assign_element_tags( EntityHandle ent_output )
{
  std::vector< std::pair< Tag, int > >::iterator it;
  for ( it = this->output_element_tags.begin(); it != this->output_element_tags.end(); ++ it )
    {
    this->output_mesh->tag_set_data( it->first, &ent_output, 1, &this->element_tag_data[it->second] );
    }
}

} // namespace moab
