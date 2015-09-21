#include "iMeshP.h"
#include "moab_mpi.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <assert.h>
#include <math.h>
#include <map>
#include <string.h>
#include <stdio.h> // remove()

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#endif

#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
const char* const FILENAME = "iMeshP_test_file";


/**************************************************************************
                              Error Checking
 **************************************************************************/

#define CHKERR do { \
  if (ierr) { \
    std::cerr << "Error code  " << ierr << " at " << __FILE__ << ":" << __LINE__ << std::endl;\
    return ierr; \
  } \
} while (false) 

#define PCHECK do { ierr = is_any_proc_error(ierr); CHKERR; } while(false)

// Use my_rank instead of rank to avoid shadowed declaration
#define ASSERT(A) do { \
  if (is_any_proc_error(!(A))) { \
    int my_rank = 0; \
    MPI_Comm_rank( MPI_COMM_WORLD, &my_rank ); \
    if (0 == my_rank) std::cerr << "Failed assertion: " #A << std::endl \
                         << "  at " __FILE__ ":" << __LINE__ << std::endl; \
    return 1; \
  } } while (false)
              
// Test if is_my_error is non-zero on any processor in MPI_COMM_WORLD
int is_any_proc_error( int is_my_error )
{
  int result;
  int err = MPI_Allreduce( &is_my_error, &result, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD );
  return err || result;
}

/**************************************************************************
                           Test  Declarations
 **************************************************************************/

class PartMap;

/**\brief Consistency check for parallel load
 *
 * All other tests depend on this one.
 */
int test_load( iMesh_Instance, iMeshP_PartitionHandle prtn, PartMap& map, int comm_size );


/**\brief Test partition query methods
 *
 * Test:
 * - iMeshP_getPartitionComm
 * - iMeshP_getNumPartitions
 * - iMeshP_getPartitions
 */
int test_get_partitions( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test part quyery methods
 *
 * Test:
 * - iMeshP_getNumGlobalParts
 * - iMeshP_getNumLocalParts
 * - iMeshP_getLocalParts
 */
int test_get_parts( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test query by entity type
 *
 * Test:
 * - iMeshP_getNumOfTypeAll
 * - iMeshP_getNumOfType
 * - iMeshP_getEntities
 * - 
 */
int test_get_by_type( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test query by entity topology
 *
 * Test:
 * - iMeshP_getNumOfTopoAll
 * - iMeshP_getNumOfTopo
 * - iMeshP_getEntities
 * - 
 */
int test_get_by_topo( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test mapping from part id to part handle
 * 
 * Test:
 * - iMeshP_getPartIdFromPartHandle
 * - iMeshP_getPartIdsFromPartHandlesArr
 * - iMeshP_getPartHandleFromPartId
 * - iMeshP_getPartHandlesFromPartsIdsArr
 * - iMeshP_getRankOfPart
 * - iMeshP_getRankOfPartArr
 */
int test_part_id_handle( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test get part rank
 *
 * Tests:
 * - iMeshP_getRankOfPart
 * - iMeshP_getRankOfPartArr
 */
int test_part_rank( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test querying of part neighbors
 *
 * Test:
 * - iMeshP_getNumPartNbors
 * - iMeshP_getNumPartNborsArr
 * - iMeshP_getPartNbors
 * - iMeshP_getPartNborsArr
 */
int test_get_neighbors( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test querying of part boundary entities
 *
 * Test:
 * - iMeshP_getNumPartBdryEnts
 * - iMeshP_getPartBdryEnts
 */
int test_get_part_boundary( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test querying of part boundary entities
 *
 * Test:
 * - iMeshP_initPartBdryEntIter
 * - iMeshP_initPartBdryEntArrIter
 */
int test_part_boundary_iter( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test adjacent entity query
 *
 * Test:
 * - iMeshP_getAdjEntities
 */
int test_get_adjacencies( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test entity iterators
 *
 * Test:
 * - iMeshP_initEntIter
 * - iMeshP_initEntArrIter
 */
int test_entity_iterator( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test entity owner queries
 *
 * Test:
 * - iMeshP_getEntOwnerPart
 * - iMeshP_getEntOwnerPartArr
 * - iMeshP_isEntOwner
 * - iMeshP_isEntOwnerArr
 */
int test_entity_owner( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test entity status
 *
 * Test:
 * - iMeshP_getEntStatus
 * - iMeshP_getEntStatusArr
 */
int test_entity_status( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test information about entity copies for interface entities
 *
 * Test:
 * - iMeshP_getNumCopies
 * - iMeshP_getCopyParts
 */
int test_entity_copy_parts( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test information about entity copies for interface entities
 *
 * Test:
 * - iMeshP_getCopies
 * - iMeshP_getCopyOnPart
 * - iMeshP_getOwnerCopy
 */
int test_entity_copies( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test creation of ghost entities
 *
 * Test:
 * - iMeshP_createGhostEntsAll
 */
int test_create_ghost_ents( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

/**\brief Test commuinication of tag data
 *
 * Test:
 * - iMeshP_pushTags
 * - iMeshP_pushTagsEnt
 */
int test_push_tag_data_iface( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );
int test_push_tag_data_ghost( iMesh_Instance, iMeshP_PartitionHandle prtn, const PartMap& );

int test_exchange_ents( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map );

/**************************************************************************
                              Helper Funcions
 **************************************************************************/
 
class PartMap
{
public:
  int num_parts() const 
    { return sortedPartList.size(); }

  iMeshP_Part part_id_from_local_id( int local_id ) const
    { return sortedPartList[idx_from_local_id(local_id)]; }
    
  int local_id_from_part_id( iMeshP_Part part ) const
    { return partLocalIds[idx_from_part_id(part)]; }
  
  int rank_from_part_id( iMeshP_Part part ) const
    { return partRanks[idx_from_part_id(part)]; }
  
  int rank_from_local_id( int id ) const
    { return partRanks[idx_from_local_id(id)]; }
  
  int count_from_rank( int rank ) const
    { return std::count( partRanks.begin(), partRanks.end(), rank ); }
    
  void part_id_from_rank( int rank, std::vector<iMeshP_Part>& parts ) const;
  
  void local_id_from_rank( int rank, std::vector<int>& ids ) const;
  
  const std::vector<iMeshP_Part>& get_parts() const 
    { return sortedPartList; }
  
  const std::vector<int>& get_ranks() const 
    { return partRanks; }

  int build_map( iMesh_Instance imesh,
                 iMeshP_PartitionHandle partition,
                 int num_expected_parts );

  static int part_from_coords( iMesh_Instance imesh, 
                               iMeshP_PartHandle part, 
                               int& id_out );

private:
  inline int idx_from_part_id( iMeshP_Part id ) const
    { return std::lower_bound( sortedPartList.begin(), sortedPartList.end(), id ) 
          -  sortedPartList.begin(); }
  inline int idx_from_local_id( int id ) const
    { return localIdReverseMap[id]; }
    
  std::vector<iMeshP_Part> sortedPartList;
  std::vector<int> partRanks;
  std::vector<int> partLocalIds;
  std::vector<int> localIdReverseMap;
};

/**\brief Create mesh for use in parallel tests */
int create_mesh( const char* filename, int num_parts );

/**\brief get unique identifier for each vertex */
int vertex_tag( iMesh_Instance imesh, iBase_EntityHandle vertex, int& tag );

int get_local_parts( iMesh_Instance instance,
                     iMeshP_PartitionHandle prtn,
                     std::vector<iMeshP_PartHandle>& handles,
                     std::vector<iMeshP_Part>* ids = 0 )
{
  iMeshP_PartHandle* arr = 0;
  int ierr, alloc = 0, size;
  iMeshP_getLocalParts( instance, prtn, &arr, &alloc, &size, &ierr );
  CHKERR;
  handles.resize( size );
  std::copy( arr, arr + size, handles.begin() );
  if (!ids)
    return iBase_SUCCESS;
  
  ids->resize( size );
  alloc = size;
  iMeshP_Part* ptr = &(*ids)[0];
  iMeshP_getPartIdsFromPartHandlesArr( instance, prtn, &handles[0], handles.size(),
                                       &ptr, &alloc, &size, &ierr );
  CHKERR;
  assert( size == (int)ids->size() );
  assert( ptr == &(*ids)[0] );
  return iBase_SUCCESS;
}


static int get_entities( iMesh_Instance imesh,
                         iBase_EntitySetHandle set,
                         iBase_EntityType type,
                         iMesh_EntityTopology topo,
                         std::vector<iBase_EntityHandle>& entities )
{
  iBase_EntityHandle* array = 0;
  int junk = 0, size = 0, err;
  iMesh_getEntities( imesh, set, type, topo, &array, &junk, &size, &err );
  if (!err) {
    entities.clear();
    entities.resize( size );
    std::copy( array, array + size, entities.begin() );
    free( array );
  }
  return err;
}

static int get_part_quads_and_verts( iMesh_Instance imesh,
                                     iMeshP_PartHandle part,
                                     std::vector<iBase_EntityHandle>& elems,
                                     std::vector<iBase_EntityHandle>& verts )
{
  int ierr = get_entities( imesh, part, iBase_FACE, iMesh_QUADRILATERAL, elems );
  CHKERR;
  
  verts.resize(4*elems.size());
  std::vector<int> junk(elems.size()+1);
  int junk1 = verts.size(), count, junk2 = junk.size(), junk3;
  iBase_EntityHandle* junk4 = &verts[0];
  int* junk5 = &junk[0];
  iMesh_getEntArrAdj( imesh, &elems[0], elems.size(), iBase_VERTEX,
                      &junk4, &junk1, &count,
                      &junk5, &junk2, &junk3, &ierr );
  CHKERR;
  assert( junk1 == (int)verts.size() );
  assert( count == (int)(4*elems.size()) );
  assert( junk2 == (int)junk.size() );
  assert( junk4 == &verts[0] );
  assert( junk5 == &junk[0] );
  std::sort( verts.begin(), verts.end() );
  verts.erase( std::unique( verts.begin(), verts.end() ), verts.end() );
  return iBase_SUCCESS;
}
  
static int get_coords( iMesh_Instance imesh,
                       const iBase_EntityHandle* verts,
                       int num_verts,
                       double* coords )
{
  double* junk1 = coords;
  int junk2 = 3*num_verts;
  int junk3;
  int ierr;
  iMesh_getVtxArrCoords( imesh, verts, num_verts, iBase_INTERLEAVED, &junk1, &junk2, &junk3, &ierr );
  if (iBase_SUCCESS != ierr)
    return ierr;
  assert( junk1 == coords );
  assert( junk2 == 3*num_verts );
  assert( junk3 == 3*num_verts );
  return iBase_SUCCESS;
}
  
/**************************************************************************
                              Main Method
 **************************************************************************/

#define RUN_TEST(A) run_test( &A, #A )

int run_test( int (*func)(iMesh_Instance, iMeshP_PartitionHandle, const PartMap&), 
              const char* func_name )
{
  int rank, size, ierr;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  
  if (rank == 0) {
    ierr = create_mesh( FILENAME, size );
  }
  MPI_Bcast( &ierr, 1, MPI_INT, 0, MPI_COMM_WORLD );
  if (ierr) {
    if (rank == 0) {
      std::cerr << "Failed to create input test file on root processor.  Aborting."
                << std::endl;
    }
    abort();
  }
  
  iMesh_Instance imesh;
  iMesh_newMesh( 0, &imesh, &ierr, 0 );
  PCHECK;
  
  iMeshP_PartitionHandle prtn;
  iMeshP_createPartitionAll( imesh, MPI_COMM_WORLD, &prtn, &ierr );
  PCHECK;
  
  PartMap map;
  ierr = test_load( imesh, prtn, map, size );
  if (ierr) {
    if (rank == 0) {
      std::cerr << "Failed to load input mesh." << std::endl
                << "Cannot run further tests." << std::endl
                << "ABORTING" << std::endl;
    }
    abort();
  }

  int result = (*func)(imesh,prtn,map);
  int is_err = is_any_proc_error( result );
  if (rank == 0) {
    if (is_err) 
      std::cout << func_name << " : FAILED!!" << std::endl;
    else
      std::cout << func_name << " : success" << std::endl;
  }
  
  iMesh_dtor( imesh, &ierr );
  CHKERR;
  return is_err;
}

int main( int argc, char* argv[] )
{
  MPI_Init(&argc, &argv);
  int size, rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  if (argc > 2 && !strcmp(argv[1], "-p")) {
#if !defined(_MSC_VER) && !defined(__MINGW32__)
    std::cout << "Processor " << rank << " of " << size << " with PID " << getpid() << std::endl;
    std::cout.flush();
#endif
      // loop forever on requested processor, giving the user time
      // to attach a debugger.  Once the debugger in attached, user
      // can change 'pause'.  E.g. on gdb do "set var pause = 0"
    if (atoi(argv[2]) == rank) {
      volatile int pause = 1;
      while (pause);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  int num_errors = 0;
  num_errors += RUN_TEST( test_get_partitions );
  num_errors += RUN_TEST( test_get_parts );
  num_errors += RUN_TEST( test_get_by_type );
  num_errors += RUN_TEST( test_get_by_topo );
  num_errors += RUN_TEST( test_part_id_handle );
  num_errors += RUN_TEST( test_part_rank );
  num_errors += RUN_TEST( test_get_neighbors );
  num_errors += RUN_TEST( test_get_part_boundary );
  num_errors += RUN_TEST( test_part_boundary_iter );
  num_errors += RUN_TEST( test_get_adjacencies );
  num_errors += RUN_TEST( test_entity_iterator );
  num_errors += RUN_TEST( test_entity_owner );
  num_errors += RUN_TEST( test_entity_status );
  num_errors += RUN_TEST( test_entity_copy_parts );
  num_errors += RUN_TEST( test_entity_copies );
  num_errors += RUN_TEST( test_push_tag_data_iface );
  num_errors += RUN_TEST( test_push_tag_data_ghost );
  num_errors += RUN_TEST( test_create_ghost_ents );
  num_errors += RUN_TEST( test_exchange_ents );
  
    // wait until all procs are done before writing summary data
  std::cout.flush();
  MPI_Barrier( MPI_COMM_WORLD );
  
    // clean up output file
  if (rank == 0)
    remove( FILENAME );
  
  if (rank == 0) {
    if (!num_errors) 
      std::cout << "All tests passed" << std::endl;
    else
      std::cout << num_errors << " TESTS FAILED!" << std::endl;
  }
  
  MPI_Finalize();
  return num_errors;
}

// Create a mesh
//
// 
// Groups of four quads will be arranged into parts as follows:
// +------+------+------+------+------+-----
// |             |             |
// |             |             |
// +    Part 0   +    Part 2   +    Part 4
// |             |             |
// |             |             |
// +------+------+------+------+------+-----
// |             |             |
// |             |             |
// +    Part 1   +    Part 3   +    Part 5
// |             |             |
// |             |             |
// +------+------+------+------+------+-----
//
// Vertices will be enumerated as follows:
// 1------6-----11-----16-----21-----26-----
// |             |             |
// |             |             |
// 2      7     12     17     22     27
// |             |             |
// |             |             |
// 3------8-----13-----18-----23-----28-----
// |             |             |
// |             |             |
// 4      9     14     19     24     29
// |             |             |
// |             |             |
// 5-----10-----15-----20-----25-----30-----
//
// Element IDs will be [4*rank+1,4*rank+5]
template <int size> struct EHARR
{ 
  iBase_EntityHandle h[size];
  iBase_EntityHandle& operator[](int i){ return h[i]; } 
  operator iBase_EntityHandle*() { return h; }
};
int create_mesh( const char* filename, int num_parts )
{
  const char* tagname = "GLOBAL_ID";
  int ierr;
  
  iMesh_Instance imesh;
  iMesh_newMesh( 0, &imesh, &ierr, 0 ); CHKERR;
  
  const int num_full_cols = 2 * (num_parts / 2);
  const int need_half_cols = num_parts % 2;
  const int num_cols = num_full_cols + 2*need_half_cols;
  const int num_vtx = 5+5*num_cols - 4*(num_parts%2);
  std::vector< EHARR<5> > vertices( num_cols + 1 );
  std::vector< EHARR<4> > elements( num_cols );
  std::vector<int> vertex_ids( num_vtx );
  std::vector<iBase_EntityHandle> vertex_list(num_vtx);
  for (int i = 0; i < num_vtx; ++i)
    vertex_ids[i] = i+1;
  
    // create vertices
  int vl_pos = 0;
  for (int i = 0; i <= num_cols; ++i) {
    double coords[15] = { static_cast<double>(i), 0, 0,
                          static_cast<double>(i), 1, 0,
                          static_cast<double>(i), 2, 0,
                          static_cast<double>(i), 3, 0,
                          static_cast<double>(i), 4, 0 };
    iBase_EntityHandle* ptr = vertices[i];
    const int n = (num_full_cols == num_cols || i <= num_full_cols) ? 5 : 3;
    int junk1 = n, junk2 = n;
    iMesh_createVtxArr( imesh, n, iBase_INTERLEAVED, coords, 3*n,
                        &ptr, &junk1, &junk2, &ierr ); CHKERR;
    assert( ptr == vertices[i] );
    assert( junk1 == n );
    assert( junk2 == n );
    for (int j = 0; j < n; ++j)
      vertex_list[vl_pos++] = vertices[i][j];
  }
  
    // create elements
  for (int i = 0; i < num_cols; ++i) {
    iBase_EntityHandle conn[16];
    for (int j = 0; j < 4; ++j) {
      conn[4*j  ] = vertices[i  ][j  ];
      conn[4*j+1] = vertices[i  ][j+1];
      conn[4*j+2] = vertices[i+1][j+1];
      conn[4*j+3] = vertices[i+1][j  ];
    }
    iBase_EntityHandle* ptr = elements[i];
    const int n = (i < num_full_cols) ? 4 : 2;
    int junk1 = n, junk2 = n, junk3 = n, junk4 = n;
    int stat[4];
    int* ptr2 = stat;
    iMesh_createEntArr( imesh, 
                        iMesh_QUADRILATERAL, 
                        conn, 4*n,
                        &ptr, &junk1, &junk2,
                        &ptr2, &junk3, &junk4, 
                        &ierr ); CHKERR;
    assert( ptr == elements[i] );
    assert( junk1 == n );
    assert( junk2 == n );
    assert( ptr2 == stat );
    assert( junk3 == n );
    assert( junk4 == n );
  }
  
    // create partition
  iMeshP_PartitionHandle partition;
  iMeshP_createPartitionAll( imesh, MPI_COMM_SELF, &partition, &ierr ); CHKERR;
  for (int i = 0; i < num_parts; ++i) {
    iMeshP_PartHandle part;
    iMeshP_createPart( imesh, partition, &part, &ierr ); CHKERR;
    iBase_EntityHandle quads[] = { elements[2*(i/2)  ][2*(i%2)  ],
                                   elements[2*(i/2)+1][2*(i%2)  ],
                                   elements[2*(i/2)  ][2*(i%2)+1],
                                   elements[2*(i/2)+1][2*(i%2)+1] };
    iMesh_addEntArrToSet( imesh, quads, 4, part, &ierr ); CHKERR;
  }
  
    // assign global ids to vertices
  iBase_TagHandle id_tag = 0;
  iMesh_getTagHandle( imesh, tagname, &id_tag, &ierr, strlen(tagname) );
  if (iBase_SUCCESS == ierr) {
    int tag_size, tag_type;
    iMesh_getTagSizeValues( imesh, id_tag, &tag_size, &ierr );
    CHKERR;
    if (tag_size != 1)
      return iBase_TAG_ALREADY_EXISTS;
    iMesh_getTagType( imesh, id_tag, &tag_type, &ierr );
    CHKERR;
    if (tag_type != iBase_INTEGER)
      return iBase_TAG_ALREADY_EXISTS;
  }
  else {
    iMesh_createTag( imesh, tagname, 1, iBase_INTEGER, &id_tag, &ierr, strlen(tagname) );
    CHKERR;
  }
  iMesh_setIntArrData( imesh, &vertex_list[0], num_vtx, id_tag, &vertex_ids[0], num_vtx, &ierr );
  CHKERR;
  
    // write file
  iBase_EntitySetHandle root_set;
  iMesh_getRootSet( imesh, &root_set, &ierr );
  iMeshP_saveAll( imesh, partition, root_set, filename, 0, &ierr, strlen(filename), 0 );
  CHKERR;
  
  iMesh_dtor( imesh, &ierr ); CHKERR;
  
  return 0;
}

// generate unique for each vertex from coordinates.
// Assume integer coordinate values with x in [0,inf] and y in [0,4]
// as generated by create_mean(..).
int vertex_tag( iMesh_Instance imesh, iBase_EntityHandle vertex, int& tag ) 
{
  int ierr;
  double x, y, z;
  iMesh_getVtxCoord( imesh, vertex, &x, &y, &z, &ierr );
  CHKERR;
  
  int xc = (int)round(x);
  int yc = (int)round(y);
  tag = 5*xc + yc + 1;
  return ierr;
}

/**************************************************************************
                           Test  Implementations
 **************************************************************************/

int test_load( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, PartMap& map, int proc_size )
{
  int ierr;
  
  iBase_EntitySetHandle root_set;
  iMesh_getRootSet( imesh, &root_set, &ierr );
  const char *opt = "moab:PARTITION=PARALLEL_PARTITION";
  iMeshP_loadAll( imesh, prtn, root_set, FILENAME, opt, &ierr, strlen(FILENAME), strlen(opt) );
  PCHECK;

  
  ierr = map.build_map( imesh, prtn, proc_size );
  CHKERR;
  return iBase_SUCCESS;
}


/**\brief Test partition query methods
 *
 * Test:
 * - iMeshP_getPartitionComm
 * - iMeshP_getNumPartitions
 * - iMeshP_getPartitions
 */
int test_get_partitions( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& )
{
  int ierr;
  
    // test iMeshP_getPartitionCom
  MPI_Comm comm = MPI_COMM_SELF;
  iMeshP_getPartitionComm( imesh, prtn, &comm, &ierr );
  PCHECK;
  ASSERT(comm == MPI_COMM_WORLD);
  
  
    // test iMeshP_getPartitions
  iMeshP_PartitionHandle* array = 0;
  int alloc = 0, size = -1;
  iMeshP_getPartitions( imesh, &array, &alloc, &size, &ierr );
  PCHECK;
  ASSERT(array != 0);
  ASSERT(alloc == size);
  ASSERT(size > 0);
  int idx = std::find(array, array+size, prtn) - array;
  free(array);
  ASSERT(idx < size);
  
    // test iMesP_getNumPartitions
  int size2 = -1;
  iMeshP_getNumPartitions( imesh, &size2, &ierr );
  PCHECK;
  ASSERT(size2 == size);
  return 0;
}

  

/**\brief Test part quyery methods
 *
 * Test:
 * - iMeshP_getNumGlobalParts
 * - iMeshP_getNumLocalParts
 * - iMeshP_getLocalParts
 */
int test_get_parts( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int size, rank, ierr;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  
  int num_part_g;
  iMeshP_getNumGlobalParts( imesh, prtn, &num_part_g, &ierr );
  PCHECK;
  ASSERT( num_part_g == map.num_parts() );
  
  int num_part_l;
  iMeshP_getNumLocalParts( imesh, prtn, &num_part_l, &ierr );
  PCHECK;
  ASSERT( num_part_l == map.count_from_rank( rank ) );
  
  std::vector<iMeshP_PartHandle> parts(num_part_l);
  iMeshP_PartHandle* ptr = &parts[0];
  int junk1 = num_part_l, count = -1;
  iMeshP_getLocalParts( imesh, prtn, &ptr, &junk1, &count, &ierr );
  PCHECK;
  assert( ptr == &parts[0] );
  assert( junk1 == num_part_l );
  ASSERT( count == num_part_l );
  
  return iBase_SUCCESS;
}

static int test_get_by_type_topo_all( iMesh_Instance imesh,
                                      iMeshP_PartitionHandle prtn,
                                      bool test_type,
                                      int num_parts )
{
    // calculate number of quads and vertices in entire mesh
    // from number of parts (see create_mesh(..) function.)
  const int expected_global_quad_count = 4 * num_parts;
  const int num_col = 2 * (num_parts / 2 + num_parts % 2);
  const int expected_global_vtx_count = num_parts == 1 ? 9 :
                                        num_parts % 2  ? 1 + 5*num_col :
                                                         5 + 5*num_col;
  
    // test getNumOf*All for root set
  int ierr, count;
  iBase_EntitySetHandle root;
  iMesh_getRootSet( imesh, &root, &ierr );  
  if (test_type) 
    iMeshP_getNumOfTypeAll( imesh, prtn, root, iBase_VERTEX, &count, &ierr );
  else
    iMeshP_getNumOfTopoAll( imesh, prtn, root, iMesh_POINT, &count, &ierr );
  PCHECK;
  ASSERT( count == expected_global_vtx_count );
  if (test_type) 
    iMeshP_getNumOfTypeAll( imesh, prtn, root, iBase_FACE, &count, &ierr );
  else
    iMeshP_getNumOfTopoAll( imesh, prtn, root, iMesh_QUADRILATERAL, &count, &ierr );
  PCHECK;
  ASSERT( count == expected_global_quad_count );
  
    // create an entity set containing half of the quads
  std::vector<iBase_EntityHandle> all_quads, half_quads;
  ierr = get_entities( imesh, root, iBase_FACE, iMesh_QUADRILATERAL, all_quads );
  assert( 0 == all_quads.size() % 2 );
  half_quads.resize(all_quads.size()/2);
  for (size_t i = 0; i < all_quads.size() / 2; ++i)
    half_quads[i] = all_quads[2*i];
  iBase_EntitySetHandle set;
  iMesh_createEntSet( imesh, 1, &set, &ierr );
  CHKERR;
  iMesh_addEntArrToSet( imesh, &half_quads[0], half_quads.size(), set, &ierr );
  CHKERR;
  
    // test getNumOf*All with defined set
  if (test_type) 
    iMeshP_getNumOfTypeAll( imesh, prtn, set, iBase_VERTEX, &count, &ierr );
  else
    iMeshP_getNumOfTopoAll( imesh, prtn, set, iMesh_POINT, &count, &ierr );
  PCHECK;
  ASSERT( count == 0 );
  if (test_type) 
    iMeshP_getNumOfTypeAll( imesh, prtn, set, iBase_FACE, &count, &ierr );
  else
    iMeshP_getNumOfTopoAll( imesh, prtn, set, iMesh_QUADRILATERAL, &count, &ierr );
  PCHECK;
  ASSERT( count == expected_global_quad_count/2 );
  
  return 0;
}

static int test_get_by_type_topo_local( iMesh_Instance imesh,
                                        iMeshP_PartitionHandle prtn,
                                        bool test_type )
{
  int ierr;
  iBase_EntitySetHandle root;
  iMesh_getRootSet( imesh, &root, &ierr );  
 
    // select a single part
  std::vector<iMeshP_PartHandle> parts;
  ierr = get_local_parts( imesh, prtn, parts );
  CHKERR;
  iMeshP_PartHandle part = parts.front();
  
    // get the entities contained in the part
  std::vector<iBase_EntityHandle> part_quads, part_all;
  ierr = get_entities( imesh, part, iBase_FACE, iMesh_QUADRILATERAL, part_quads ); CHKERR;
  ierr = get_entities( imesh, part, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, part_all ); CHKERR;
  
    // compare local counts (using root set)
    
  int count;
  if (test_type)
    iMeshP_getNumOfType( imesh, prtn, part, root, iBase_FACE, &count, &ierr );
  else
    iMeshP_getNumOfTopo( imesh, prtn, part, root, iMesh_QUADRILATERAL, &count, &ierr );
  CHKERR;
  ASSERT( count == (int)part_quads.size() );

  if (test_type)
    iMeshP_getNumOfType( imesh, prtn, part, root, iBase_ALL_TYPES, &count, &ierr );
  else
    iMeshP_getNumOfTopo( imesh, prtn, part, root, iMesh_ALL_TOPOLOGIES, &count, &ierr );
  CHKERR;
  ASSERT( count == (int)part_all.size() );
  
    // compare local contents (using root set)

  iBase_EntityHandle* ptr = 0;
  int num_ent, junk1 = 0;
  iMeshP_getEntities( imesh, prtn, part, root, test_type ? iBase_FACE : iBase_ALL_TYPES,
                      test_type ? iMesh_ALL_TOPOLOGIES : iMesh_QUADRILATERAL,
                      &ptr, &junk1, &num_ent, &ierr ); CHKERR;
  std::vector<iBase_EntityHandle> act_quads( ptr, ptr+num_ent );
  free(ptr);
  junk1 = num_ent = 0;
  ptr = 0;
  iMeshP_getEntities( imesh, prtn, part, root, iBase_ALL_TYPES,
                      iMesh_ALL_TOPOLOGIES,
                      &ptr, &junk1, &num_ent, &ierr ); CHKERR;
  std::vector<iBase_EntityHandle> act_all( ptr, ptr+num_ent );
  free(ptr);
  std::sort( part_quads.begin(), part_quads.end() );
  std::sort( part_all.begin(), part_all.end() );
  std::sort( act_quads.begin(), act_quads.end() );
  std::sort( act_all.begin(), act_all.end() );
  ASSERT( part_quads == act_quads );
  ASSERT( part_all   == act_all   );
  
    // create an entity set containing half of the quads from the part
  std::vector<iBase_EntityHandle> half_quads(part_quads.size()/2);
  for (size_t i = 0; i < half_quads.size(); ++i)
    half_quads[i] = part_quads[2*i];
  iBase_EntitySetHandle set;
  iMesh_createEntSet( imesh, 1, &set, &ierr );
  CHKERR;
  iMesh_addEntArrToSet( imesh, &half_quads[0], half_quads.size(), set, &ierr );
  CHKERR;
  
    // check if there exists any quads not in the part that we 
    // can add to the set
  std::vector<iBase_EntityHandle> all_quads, other_quads;
  ierr = get_entities( imesh, root, iBase_FACE, iMesh_QUADRILATERAL, all_quads); CHKERR;
  std::sort( all_quads.begin(), all_quads.end() );
  std::sort( part_quads.begin(), part_quads.end() );
  std::set_difference( all_quads.begin(), all_quads.end(),
                       part_quads.begin(), part_quads.end(),
                       std::back_inserter( other_quads ) );
  iMesh_addEntArrToSet( imesh, &other_quads[0], other_quads.size(), set, &ierr );
  CHKERR;
  
    // compare local counts (using non-root set)
    
  if (test_type)
    iMeshP_getNumOfType( imesh, prtn, part, set, iBase_FACE, &count, &ierr );
  else
    iMeshP_getNumOfTopo( imesh, prtn, part, set, iMesh_QUADRILATERAL, &count, &ierr );
  CHKERR;
  ASSERT( count == (int)half_quads.size() );

  if (test_type)
    iMeshP_getNumOfType( imesh, prtn, part, set, iBase_VERTEX, &count, &ierr );
  else
    iMeshP_getNumOfTopo( imesh, prtn, part, set, iMesh_POINT, &count, &ierr );
  CHKERR;
  ASSERT( count == 0 );
  
    // compare local contents (using non-root set)

  junk1 = 0; num_ent = 0;
  ptr = 0;
  iMeshP_getEntities( imesh, prtn, part, set, test_type ? iBase_FACE : iBase_ALL_TYPES,
                      test_type ? iMesh_ALL_TOPOLOGIES : iMesh_QUADRILATERAL,
                      &ptr, &junk1, &num_ent, &ierr ); CHKERR;
  act_quads.resize(num_ent);
  std::copy( ptr, ptr + num_ent, act_quads.begin() );
  free(ptr);
  std::sort( half_quads.begin(), half_quads.end() );
  std::sort( act_quads.begin(), act_quads.end() );
  ASSERT( act_quads == half_quads );
  
  return iBase_SUCCESS;
}
    


/**\brief Test query by entity type
 *
 * Test:
 * - iMeshP_getNumOfTypeAll
 * - iMeshP_getNumOfType
 * - iMeshP_getEntities
 * - 
 */
int test_get_by_type( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr;
  ierr = test_get_by_type_topo_all( imesh, prtn, true, map.num_parts() );
  PCHECK;
  ierr = test_get_by_type_topo_local( imesh, prtn, true );
  PCHECK;
  return 0;
}

/**\brief Test query by entity topology
 *
 * Test:
 * - iMeshP_getNumOfTopoAll
 * - iMeshP_getNumOfTopo
 * - iMeshP_getEntities
 * - 
 */
int test_get_by_topo( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr;
  ierr = test_get_by_type_topo_all( imesh, prtn, false, map.num_parts() );
  PCHECK;
  ierr = test_get_by_type_topo_local( imesh, prtn, false );
  PCHECK;
  return 0;
}


/**\brief Test mapping from part id to part handle
 * 
 * Test:
 * - iMeshP_getPartIdFromPartHandle
 * - iMeshP_getPartIdsFromPartHandlesArr
 * - iMeshP_getPartHandleFromPartId
 * - iMeshP_getPartHandlesFromPartsIdsArr
 */
int test_part_id_handle( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
    // get local part ids
  int rank, ierr;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  std::vector<iMeshP_Part> ids;
  map.part_id_from_rank( rank, ids );
  
    // check single-part functions and build list of part handles
  std::vector<iMeshP_PartHandle> handles( ids.size() );
  size_t i;
  for (i = 0; i < ids.size(); ++i) {
    iMeshP_getPartHandleFromPartId( imesh, prtn, ids[i], &handles[i], &ierr );
    CHKERR;
    iMeshP_Part id;
    iMeshP_getPartIdFromPartHandle( imesh, prtn, handles[i], &id, &ierr );
    CHKERR;
    if (id != ids[i])
      break;
  }
  ASSERT( i == ids.size() );
  
    // test iMeshP_getPartIdsFromPartHandlesArr
  std::vector<iMeshP_Part> ids2( ids.size() );
  int junk1 = ids.size(), junk2 = 0;
  iMeshP_Part* ptr = &ids2[0];
  iMeshP_getPartIdsFromPartHandlesArr( imesh, prtn, &handles[0], handles.size(),
                                       &ptr, &junk1, &junk2, &ierr );
  PCHECK;
  ASSERT( ptr == &ids2[0] );
  ASSERT( junk2 == (int)ids2.size() );
  ASSERT( ids == ids2 );
  
    // test iMeshP_getPartHandlesFromPartsIdsArr
  std::vector<iMeshP_PartHandle> handles2(handles.size());
  junk1 = handles.size();
  junk2 = 0;
  iMeshP_PartHandle* ptr2 = &handles2[0];
  iMeshP_getPartHandlesFromPartsIdsArr( imesh, prtn, &ids[0], ids.size(),
                                        &ptr2, &junk1, &junk2, &ierr );
  PCHECK;
  ASSERT( ptr2 == &handles2[0] );
  ASSERT( junk2 == (int)handles2.size() );
  ASSERT( handles == handles2 );
  
  return 0;
}

/**\brief Test get part rank
 *
 * Tests:
 * - iMeshP_getRankOfPart
 * - iMeshP_getRankOfPartArr
 */
int test_part_rank( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr = 0, rank;
  std::vector<iMeshP_Part> invalid, failed;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  
    // test iMeshP_getRankOfPart
  for (size_t i = 0; i < map.get_parts().size(); ++i) {
    int pr;
    iMeshP_getRankOfPart( imesh, prtn, map.get_parts()[i], &pr, &ierr );
    if (iBase_SUCCESS != ierr)
      failed.push_back( map.get_parts()[i] );
    else if (pr != map.get_ranks()[i])
      invalid.push_back( map.get_parts()[i] );
  }
  if (!failed.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getRankOfPart failed for " << failed.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!invalid.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getRankOfPart was incorrect for " << invalid.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  PCHECK;
   
    // test iMeshP_getRankOfPartArr
  std::vector<int> ranks( map.get_parts().size() );
  int junk1 = ranks.size(), junk2, *ptr = &ranks[0];
  iMeshP_getRankOfPartArr( imesh, prtn, &map.get_parts()[0], map.get_parts().size(),
                           &ptr, &junk1, &junk2, &ierr );
  PCHECK; 
  assert( ptr == &ranks[0] );
  assert( junk1 == (int)ranks.size() );
  ASSERT( junk2 == (int)ranks.size() );
  for (size_t i = 0; i < map.get_parts().size(); ++i) {
    if (ranks[i] != map.get_ranks()[i])
      invalid.push_back( map.get_parts()[i] );
  }
  if (!invalid.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getRankOfPartArr was incorrect for " << invalid.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  PCHECK;
  
  return 0;
}
   

// see create_mesh(..)
static void get_part_neighbors( int logical_part_id,
                                int num_parts,
                                int neighbors[5],
                                int& num_neighbors )
{
  num_neighbors = 0;
  if (logical_part_id + 1 < num_parts)
    neighbors[num_neighbors++] = logical_part_id + 1;
  if (logical_part_id + 2 < num_parts)
    neighbors[num_neighbors++] = logical_part_id + 2;
  if (logical_part_id % 2) {
    neighbors[num_neighbors++] = logical_part_id - 1;
    if (logical_part_id > 2) {
      neighbors[num_neighbors++] = logical_part_id - 3;
      neighbors[num_neighbors++] = logical_part_id - 2;
    }
  }
  else {
    if (logical_part_id + 3 < num_parts)
      neighbors[num_neighbors++] = logical_part_id + 3;
    if (logical_part_id > 1) {
      neighbors[num_neighbors++] = logical_part_id - 1;
      neighbors[num_neighbors++] = logical_part_id - 2;
    }
  }
}

/**\brief Test querying of part neighbors
 *
 * Test:
 * - iMeshP_getNumPartNbors
 * - iMeshP_getNumPartNborsArr
 * - iMeshP_getPartNbors
 * - iMeshP_getPartNborsArr
 */
int test_get_neighbors( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr, rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  
  std::vector<iMeshP_Part> local_parts;
  map.part_id_from_rank( rank, local_parts );
  
    // get handles for local parts
  std::vector<iMeshP_PartHandle> handles(local_parts.size());
  iMeshP_PartHandle* ptr = &handles[0];
  int junk1 = handles.size(), junk2 = 0;
  iMeshP_getPartHandlesFromPartsIdsArr( imesh, prtn, &local_parts[0], local_parts.size(),
                                        &ptr, &junk1, &junk2, &ierr );
  PCHECK;
  assert( ptr == &handles[0] );
  assert( junk2 == (int)handles.size() );
  
    // get logical ids for local parts
  std::vector<int> logical_ids;
  map.local_id_from_rank( rank, logical_ids );
  
    // get neighbors for each local part
  std::vector< std::vector<iMeshP_Part> > neighbors( logical_ids.size() );
  for (size_t i = 0;i < logical_ids.size(); ++i) {
    int logical_neighbors[5], num_neighbors;
    get_part_neighbors( logical_ids[i], map.num_parts(), logical_neighbors, num_neighbors );
    neighbors[i].resize( num_neighbors );
    for (int j = 0; j < num_neighbors; ++j)
      neighbors[i][j] = map.part_id_from_local_id( logical_neighbors[j] );
    std::sort( neighbors[i].begin(), neighbors[i].end() );
  }
  
    // test iMeshP_getNumPartNbors
  std::vector< iMeshP_Part > invalid, failed;
  for (size_t i = 0; i < local_parts.size(); ++i) {
    int count;
    iMeshP_getNumPartNbors( imesh, prtn, handles[i], iBase_VERTEX, &count, &ierr );
    if (ierr)
      failed.push_back( local_parts[i] );
    else if (count != (int)neighbors[i].size())
      invalid.push_back( local_parts[i] );
  }
  if (!failed.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getNumPartNbors failed for " << failed.size() << " parts." << std::endl;
    ierr = iBase_FAILURE; PCHECK;
  }
  if (!invalid.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getNumPartNbors was incorrect for " << invalid.size() << " parts." << std::endl;
    ierr = iBase_FAILURE; PCHECK;
  }
  
    // test iMeshP_getPartNbors
  ierr = 0;
  for (size_t i = 0; i < local_parts.size(); ++i) {
    int count, junk = 0, another_count;
    iMeshP_Part* list = 0;
    iMeshP_getPartNbors( imesh, prtn, handles[i], iBase_VERTEX, &another_count, &list, &junk, &count, &ierr );
    assert( count == another_count );
    if (ierr)
      failed.push_back( local_parts[i] );
    else {
      std::sort( list, list+count );
      std::vector<iMeshP_Part> cpy( list, list+count );
      if (cpy != neighbors[i])
        invalid.push_back( local_parts[i] );
      free(list);
    }
  }
  if (!failed.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getPartNbors failed for " << failed.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!invalid.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getPartNbors was incorrect for " << invalid.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  PCHECK;
        
    // test iMeshP_getNumPartNborsArr
  std::vector<int> count_vect( handles.size() );
  int* count_arr = &count_vect[0];
  junk1 = handles.size();
  iMeshP_getNumPartNborsArr( imesh, prtn, &handles[0], handles.size(), iBase_VERTEX,
                             &count_arr, &junk1, &junk2, &ierr );
  PCHECK;
  assert( count_arr = &count_vect[0] );
  assert( junk2 == (int)handles.size() );
  for (size_t i = 0; i < local_parts.size(); ++i) {
    if (count_arr[i] != (int)neighbors[i].size())
      invalid.push_back( local_parts[i] );
  }
  if (!invalid.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getNumPartNborsArr was incorrect for " << invalid.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  PCHECK;
  
    // test iMeshP_getPartNborsArr
  iMeshP_Part* nbor_arr = 0;
  junk1  = handles.size(), junk2 = 0;
  int junk3 = 0, nbor_size;
  iMeshP_getPartNborsArr( imesh, prtn, &handles[0], handles.size(), iBase_VERTEX,
                          &count_arr, &junk1, &junk2, 
                          &nbor_arr, &junk3, &nbor_size, 
                          &ierr );
  PCHECK;
  assert( count_arr = &count_vect[0] );
  assert( junk2 == (int)handles.size() );
  std::vector<iMeshP_Part> all_nbors( nbor_arr, nbor_arr + nbor_size );
  free( nbor_arr );
  std::vector<iMeshP_Part>::iterator j = all_nbors.begin();
  bool bad_length = false;
  for (size_t i = 0; i < local_parts.size(); ++i) {
    if (all_nbors.end() - j > count_arr[i]) {
      bad_length = true;
      break;
    }
    if (count_arr[i] != (int)neighbors[i].size()) {
      invalid.push_back( local_parts[i] );
    }
    else {
      std::vector<iMeshP_Part>::iterator e = j + count_arr[i];
      std::sort( j, e );
      if (!std::equal( j, e, neighbors[i].begin() ))
        invalid.push_back( local_parts[i] );
    }
  }
  if (bad_length)  {
    std::cerr << "Processor " << rank << ": iMeshP_getPartNborsArr had inconsistent result array lengths." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!invalid.empty()) {
    std::cerr << "Processor " << rank << ": iMeshP_getPartNborsArr was incorrect for " << invalid.size() << " parts." << std::endl;
    ierr = iBase_FAILURE;
  }
  PCHECK;
  
  return 0;
}

// Determine the expected vertices on the interface between two parts.
// Returns no vertices for non-adjacient parts and fails if both parts
// are the same.
// See create_mesh(..) for the assumed mesh.
static int interface_verts( iMesh_Instance imesh,
                            iMeshP_PartitionHandle prtn,
                            iMeshP_PartHandle local_part,
                            iMeshP_Part other_part,
                            const PartMap& map,
			    std::vector<iBase_EntityHandle> &vtx_handles )
{
  int ierr, rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );

  iMeshP_Part local_id;
  iMeshP_getPartIdFromPartHandle( imesh, prtn, local_part, &local_id, &ierr );
  CHKERR;
  
  const int local_logical = map.local_id_from_part_id( local_id );
  const int other_logical = map.local_id_from_part_id( other_part );
  
    // get grid of local vertices
  
  iBase_EntityHandle verts[3][3];
  const double xbase = (local_id / 2) * 2;
  const double ybase = (local_id % 2) * 2;
  
    // get quads in partition
  iBase_EntityHandle quads[4], *ptr = quads;
  int junk1 = 4, junk2;
  iMesh_getEntities( imesh, local_part, iBase_FACE, iMesh_QUADRILATERAL, &ptr, &junk1, &junk2, &ierr );
  CHKERR;
  assert( ptr == quads );
  assert( junk1 == 4 );
  assert( junk2 == 4 );
  
    // get vertices in quads
  iBase_EntityHandle conn[16];
  int offsets[5], *off_ptr = offsets, junk3 = 5, junk4;
  ptr = conn;
  junk1 = 16;
  iMesh_getEntArrAdj( imesh, quads, 4, iBase_VERTEX, 
                      &ptr, &junk1, &junk2,
                      &off_ptr, &junk3, &junk4,
                      &ierr );
  CHKERR;
  assert( ptr == conn );
  assert( junk1 == 16 );
  assert( junk2 == 16 );
  assert( off_ptr == offsets );
  assert( junk3 == 5 );
  assert( junk4 == 5 );
  
    // make unique vertex list
  std::sort( conn, conn + 16 );
  const int num_vtx = std::unique( conn, conn+16 ) - conn;
  assert(9 == num_vtx);
  
    // get vertex coords
  std::vector<double> coords(27);
  ierr = get_coords( imesh, conn, 9, &coords[0] );
  CHKERR;
  
    // use vertex coords to determine logical position
  for (int i = 0; i < num_vtx; ++i) {
    int x = (int)round(coords[3*i  ] - xbase);
    int y = (int)round(coords[3*i+1] - ybase);
    if (x < 0 || x > 2 || y < 0 || y > 2) {
      std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
                << "  Invalid vertex coordinate: (" << coords[3*i] << ", " << coords[3*i+1]
                << ", " << coords[3*i+2] << ")" << std::endl
                << "  For logical partition " << local_id << std::endl;
      return iBase_FAILURE;
    }
    verts[x][y] = conn[i];
  }
  
  if (local_logical % 2) {
    switch (other_logical - local_logical) {
      case 0:
        return iBase_FAILURE;
      case 1: // upper right
        vtx_handles.resize(1);
        vtx_handles[0] = verts[2][0];
        break;
      case 2: // right
        vtx_handles.resize(3);
        std::copy( verts[2], verts[2]+3, vtx_handles.begin() );
        break;
      case -1: // above
        vtx_handles.resize(3);
        vtx_handles[0] = verts[0][0];
        vtx_handles[1] = verts[1][0];
        vtx_handles[2] = verts[2][0];
        break;
      case -2: // left
        vtx_handles.resize(3);
        std::copy( verts[0], verts[0]+3, vtx_handles.begin() );
        break;
      case -3: // upper left
        vtx_handles.resize(1);
        vtx_handles[0] = verts[0][0];
        break;
      default:
        vtx_handles.clear();
        break;
    }
  }
  else {
    switch (other_logical - local_logical) {
      case 0:
        return iBase_FAILURE;
      case 1: // below
        vtx_handles.resize(3);
        vtx_handles[0] = verts[0][2];
        vtx_handles[1] = verts[1][2];
        vtx_handles[2] = verts[2][2];
        break;
      case 2: // right
        vtx_handles.resize(3);
        std::copy( verts[2], verts[2]+3, vtx_handles.begin() );
        break;
      case 3: // lower right
        vtx_handles.resize(1);
        vtx_handles[0] = verts[2][2];
        break;
      case -1: // lower left
        vtx_handles.resize(1);
        vtx_handles[0] = verts[0][2];
        break;
      case -2: // left
        vtx_handles.resize(3);
        std::copy( verts[0], verts[0]+3, vtx_handles.begin() );
        break;
      default:
        vtx_handles.clear();
        break;
    }
  }
  
  return iBase_SUCCESS;
}
       
  
  
  

/**\brief Test querying of part boundary entities
 *
 * Test:
 * - iMeshP_getNumPartBdryEnts
 * - iMeshP_getPartBdryEnts
 */
int test_get_part_boundary( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr, rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  
    // get local part handles and part ids, and global part id list
  std::vector<iMeshP_PartHandle> local_handles;
  std::vector<iMeshP_Part> local_ids;
  std::vector<iMeshP_Part> all_parts = map.get_parts();
  std::map< iMeshP_PartHandle, std::vector<iBase_EntityHandle> > part_bdry;
  ierr = get_local_parts( imesh, prtn, local_handles, &local_ids );
  CHKERR;
  
    // for each combination of local part with any other part,
    // check for valid function values.
  std::vector< std::pair<iMeshP_Part,iMeshP_Part> > num_failed, num_error, list_failed, list_error, error;
  for (size_t i = 0; i < local_handles.size(); ++i) {
    iMeshP_PartHandle local_handle = local_handles[i];
    iMeshP_Part local_id = local_ids[i];
    for (std::vector<iMeshP_Part>::iterator j = all_parts.begin(); j != all_parts.end(); ++j) {
            iMeshP_Part other_id = *j;
      if (other_id == local_id)
        continue;
      
      std::pair<iMeshP_Part,iMeshP_Part> part_pair;
      part_pair.first = local_id;
      part_pair.second = other_id; 
      
        // get expected values
      std::vector<iBase_EntityHandle> shared_verts;
      ierr = interface_verts( imesh, prtn, local_handle, other_id, map, shared_verts );
      if (ierr != iBase_SUCCESS) {
        error.push_back( part_pair );
        continue;
      }
      std::sort( shared_verts.begin(), shared_verts.end() );
      
        // test iMeshP_getNumPartBdryEnts
      int count;
      iMeshP_getNumPartBdryEnts( imesh, prtn, local_handle, iBase_VERTEX, iMesh_POINT,
                                 other_id, &count, &ierr );
      if (iBase_SUCCESS != ierr)
        num_error.push_back( part_pair );
      else if (count != (int)shared_verts.size())
        num_failed.push_back( part_pair );
      
        // test iMeshP_getPartBdryEnts
      iBase_EntityHandle* ptr = 0;
      int junk = 0;
      iMeshP_getPartBdryEnts( imesh, prtn, local_handle, iBase_VERTEX, iMesh_POINT, other_id,
                              &ptr, &junk, &count, &ierr );
      if (iBase_SUCCESS != ierr)
        list_error.push_back( part_pair );
      else {
        std::copy( ptr, ptr + count, std::back_inserter( part_bdry[local_handles[i]] ) );
        std::sort( ptr, ptr + count );
        if ((int)shared_verts.size() != count ||
            !std::equal( shared_verts.begin(), shared_verts.end(), ptr ))
          list_failed.push_back( part_pair );
        free(ptr);
      }
    }
  }
  
  if (!error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  Internal error for " << error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!num_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getNumPartBdryEnts return error for " << num_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!list_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getPartBdryEnts return error for " << list_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!num_failed.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getNumPartBdryEnts return incorrect results for " << num_failed.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!list_failed.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getPartBdryEnts return incorrect results for " << list_failed.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  
  if (iBase_SUCCESS != ierr)
    return ierr;
  
  
    // test with iMeshP_ALL_PARTS
  for (size_t i = 0; i < local_handles.size(); ++i) {
    std::vector<iBase_EntityHandle>& exp_bdry = part_bdry[local_handles[i]];
    std::sort( exp_bdry.begin(), exp_bdry.end() );
    exp_bdry.erase( std::unique( exp_bdry.begin(), exp_bdry.end() ), exp_bdry.end() );
    std::pair<iMeshP_Part,iMeshP_Part> part_pair;
    part_pair.first = local_ids[i];
    part_pair.second = iMeshP_ALL_PARTS; 
    
    int num = 0;
    iMeshP_getNumPartBdryEnts( imesh, prtn, local_handles[i], iBase_VERTEX, iMesh_POINT, 
                               iMeshP_ALL_PARTS, &num, &ierr );
    if (ierr)
      num_error.push_back( part_pair );
    else if (num != (int)exp_bdry.size())
      num_failed.push_back( part_pair );
      
    iBase_EntityHandle* bdry = 0;
    int junk = num = 0;
    iMeshP_getPartBdryEnts( imesh, prtn, local_handles[i], iBase_VERTEX, iMesh_POINT,
                            iMeshP_ALL_PARTS, &bdry, &junk, &num, &ierr );
    if (ierr)
      list_error.push_back( part_pair );
    else {
      std::sort( bdry, bdry+num );
      if (num != (int)exp_bdry.size() || !std::equal( bdry, bdry+num, exp_bdry.begin() ))
        list_failed.push_back( part_pair );
      free(bdry);
    }
  }  
  if (!num_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getNumPartBdryEnts return error for " << num_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!list_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getPartBdryEnts return error for " << list_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!num_failed.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getNumPartBdryEnts return incorrect results for " << num_failed.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!list_failed.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_getPartBdryEnts return incorrect results for " << list_failed.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  
  return ierr;
}

/**\brief Test querying of part boundary entities
 *
 * Test:
 * - iMeshP_initPartBdryEntIter
 * - iMeshP_initPartBdryEntArrIter
 */
int test_part_boundary_iter( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr, rank, has_data;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    
    // get local part handles and part ids, and global part id list
  std::vector<iMeshP_PartHandle> local_handles;
  std::vector<iMeshP_Part> local_ids;
  std::vector<iMeshP_Part> all_parts = map.get_parts();
  ierr = get_local_parts( imesh, prtn, local_handles, &local_ids );
  CHKERR;

  std::vector< std::pair<iMeshP_Part,iMeshP_Part> > single_failed, single_error, 
                                                   single_step_error, array_failed, 
                                                   array_error, array_step_error;
  for (size_t i = 0; i < local_handles.size(); ++i) {
    iMeshP_PartHandle local_handle = local_handles[i];
    iMeshP_Part local_id = local_ids[i];
    for (std::vector<iMeshP_Part>::iterator j = all_parts.begin(); j != all_parts.end(); ++j) {
      iMeshP_Part other_id = *j;
      if (other_id == local_id)
        continue;
      
      std::pair<iMeshP_Part,iMeshP_Part> part_pair;
      part_pair.first = local_id;
      part_pair.second = other_id; 
      
        // get expected values
      std::vector<iBase_EntityHandle> shared_verts;
      ierr = interface_verts( imesh, prtn, local_handle, other_id, map, shared_verts );
      if (ierr != iBase_SUCCESS || 0 == shared_verts.size())
        continue;
      std::sort( shared_verts.begin(), shared_verts.end() );
  
        // test single entity iterator
      iBase_EntityIterator siter;
      iMeshP_initPartBdryEntIter( imesh, prtn, local_handle, iBase_VERTEX, iMesh_POINT,
                                  other_id, &siter, &ierr );
      if (ierr != iBase_SUCCESS) {
        single_error.push_back( part_pair );
      }
      else {
        std::vector<iBase_EntityHandle> results;
        for (;;) {
          iBase_EntityHandle handle;
          iMesh_getNextEntIter( imesh, siter, &handle, &has_data, &ierr );
          if (ierr != iBase_SUCCESS) {
            single_step_error.push_back( part_pair );
            break;
          }
          if (!has_data)
            break;
          results.push_back(handle);
        }
      
        std::sort( results.begin(), results.end() );
        if (results.size() != shared_verts.size() ||
            !std::equal( results.begin(), results.end(), shared_verts.begin()))
          single_failed.push_back( part_pair );
      }
      iMesh_endEntIter( imesh, siter, &ierr );
      
        // test array iterator
      iBase_EntityArrIterator aiter;
      iMeshP_initPartBdryEntArrIter( imesh, prtn, local_handle, iBase_VERTEX, iMesh_POINT,
                                     shared_verts.size(), other_id, &aiter, &ierr );
      if (ierr != iBase_SUCCESS) {
        array_error.push_back( part_pair );
        continue;
      }
      iBase_EntityHandle results[5], *ptr = results;
      int junk = 5, count;
      iMesh_getNextEntArrIter( imesh, aiter, &ptr, &junk, &count, &has_data, &ierr );
      if (ierr != iBase_SUCCESS || !has_data) {
        array_step_error.push_back( part_pair );
        continue;
      }
      assert(count <= 5);
      assert(ptr == results);
      std::sort(ptr, ptr + count);
      if (count != (int)shared_verts.size() ||
          !std::equal( shared_verts.begin(), shared_verts.end(), results ))
        array_failed.push_back( part_pair );
    }
  }
  
  if (!single_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_initPartBdryEntIter return error for " << single_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!single_step_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMesh_getNextEntIter return error for " << single_step_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!single_failed.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_initPartBdryEntIter iterator iterated over invalid entities for " 
              << single_failed.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  
  if (!array_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_initPartBdryEntArrIter return error for " << array_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!array_step_error.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMesh_getNextEntArrIter return error for " << array_step_error.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  if (!array_failed.empty()) {
    std::cerr << "Processor " << rank << ": Error at " __FILE__ ":" << __LINE__ << std::endl
              << "  iMeshP_initPartBdryEntArrIter iterator iterated over invalid entities for " 
              << array_failed.size() << " part pairs." << std::endl;
    ierr = iBase_FAILURE;
  }
  
  return ierr;
}

/**\brief Test adjacent entity query
 *
 * Test:
 * - iMeshP_getAdjEntities
 */
int test_get_adjacencies( iMesh_Instance /* imesh */, iMeshP_PartitionHandle /* prtn */, const PartMap& )
{
  return iBase_SUCCESS;
}

/**\brief Test entity iterators
 *
 * Test:
 * - iMeshP_initEntIter
 * - iMeshP_initEntArrIter
 */
int test_entity_iterator( iMesh_Instance /*imesh */, iMeshP_PartitionHandle /*prtn*/, const PartMap& )
{
  return iBase_SUCCESS;
}

/**\brief Test entity owner queries
 *
 * Test:
 * - iMeshP_getEntOwnerPart
 * - iMeshP_getEntOwnerPartArr
 * - iMeshP_isEntOwner
 * - iMeshP_isEntOwnerArr
 */
int test_entity_owner( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& /* map */ )
{
  int ierr, rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
    
    // get local part handles and part ids
  std::vector<iMeshP_PartHandle> local_handles;
  std::vector<iMeshP_Part> local_ids;
  ierr = get_local_parts( imesh, prtn, local_handles, &local_ids );
  PCHECK;

    // test iMeshP_getEntOwnerPart for quads in each part
  std::vector<iBase_EntityHandle> all_quads;
  std::vector<iMeshP_Part> quad_owners;
  int invalid_count = 0;
  for (size_t i = 0; i < local_handles.size(); ++i) {
    std::vector<iBase_EntityHandle> quads;
    ierr = get_entities( imesh, local_handles[0], iBase_FACE, iMesh_QUADRILATERAL, quads );
    if (ierr)
      break;
    
    for (size_t j = 0; j < quads.size(); ++j) {
      all_quads.push_back( quads[j] );
      quad_owners.push_back( local_ids[i] );
      iMeshP_Part owner;
      iMeshP_getEntOwnerPart( imesh, prtn, quads[j], &owner, &ierr );
      if (iBase_SUCCESS != ierr)
        break;
      
      if (owner != local_ids[i])
        ++invalid_count;
    }
    if (iBase_SUCCESS != ierr)
      break;
  }
  PCHECK;
  ASSERT(0 == invalid_count);

    // test iMeshP_getEntOwnerPartArr for quads in each part
  invalid_count = 0;
  for (size_t i = 0; i < local_handles.size(); ++i) {
    std::vector<iBase_EntityHandle> quads;
    ierr = get_entities( imesh, local_handles[0], iBase_FACE, iMesh_QUADRILATERAL, quads );
    if (ierr)
      break;
    
    std::vector<iMeshP_Part> owners(quads.size()), expected(quads.size(),local_ids[i]);
    int junk = owners.size(), count;
    iMeshP_Part* ptr = &owners[0];
    iMeshP_getEntOwnerPartArr( imesh, prtn, &quads[0], quads.size(),
                               &ptr, &junk, &count, &ierr );
    if (ierr)
      break;
    assert( ptr == &owners[0] );
    assert( junk == (int)owners.size() );
    assert( count == (int)quads.size() );
    if (owners != expected)
      ++invalid_count;
  }
  PCHECK;
  ASSERT(0 == invalid_count);

    // get all vertices
  iBase_EntityHandle* vtx_arr = 0;
  int junk1 = 0, num_vtx;
  int *junk2 = 0, junk3 = 0, junk4;
  iMesh_getEntArrAdj( imesh, &all_quads[0], all_quads.size(), iBase_VERTEX,
                      &vtx_arr, &junk1, &num_vtx, &junk2, &junk3, &junk4, &ierr );
  PCHECK;
  free(junk2);
  std::sort( vtx_arr, vtx_arr + num_vtx );
  num_vtx = std::unique( vtx_arr, vtx_arr + num_vtx ) - vtx_arr;
  std::vector<iBase_EntityHandle> all_verts( vtx_arr, vtx_arr + num_vtx );
  free(vtx_arr);
  
    // check consistency between iMeshP_getEntOwnerPart and iMeshP_getEntOwnerPartArr
    // for all vertices
  std::vector<iMeshP_Part> vert_owners( all_verts.size() );
  junk1 = vert_owners.size();
  iMeshP_Part* junk5 = &vert_owners[0];
  iMeshP_getEntOwnerPartArr( imesh, prtn, &all_verts[0], all_verts.size(),
                             &junk5, &junk1, &junk3, &ierr );
  PCHECK;
  assert( junk5 == &vert_owners[0] );
  assert( junk1 == (int)vert_owners.size() );
  assert( junk3 == (int)all_verts.size() );
  
  invalid_count = 0;
  for (size_t i = 0; i < all_verts.size(); ++i) {
    iMeshP_Part owner;
    iMeshP_getEntOwnerPart( imesh, prtn, all_verts[i], &owner, &ierr );
    if (iBase_SUCCESS != ierr || owner != vert_owners[i])
      ++invalid_count;
  }
  ASSERT(0 == invalid_count);
  
    // get lists for all entities
  std::vector<iBase_EntityHandle> all_entities(all_verts);
  std::copy( all_quads.begin(), all_quads.end(), std::back_inserter(all_entities) );
  std::vector<iMeshP_Part> all_owners( vert_owners );
  std::copy( quad_owners.begin(), quad_owners.end(), std::back_inserter(all_owners) );
    
    // check consistency of iMeshP_isEntOwner for all entities
  invalid_count = 0;
  ierr = iBase_SUCCESS;
  for (size_t i = 0; i < local_handles.size(); ++i) {
    for (size_t j = 0; ierr == iBase_SUCCESS && j < all_entities.size(); ++j) {
      int is_owner;
      iMeshP_isEntOwner( imesh, prtn, local_handles[i], all_entities[j], &is_owner, &ierr );
      if (ierr != iBase_SUCCESS)
        break;
      if (!is_owner == (local_ids[i] == all_owners[j]))
        ++invalid_count;
    }
  }
  PCHECK;
  ASSERT(0 == invalid_count);
    
    // check consistency of iMeshP_isEntOwnerArr for all entities
  for (size_t i = 0; i < local_handles.size(); ++i) {
    std::vector<int> is_owner_list(all_entities.size());
    junk1 = is_owner_list.size(); 
    int* junk6 = &is_owner_list[0];
    iMeshP_isEntOwnerArr( imesh, prtn, local_handles[i], &all_entities[0], all_entities.size(),
                          &junk6, &junk1, &junk3, &ierr );
    if (iBase_SUCCESS != ierr)
      break;
    assert( junk6 == &is_owner_list[0] );
    assert( junk1 == (int)is_owner_list.size() );
    assert( junk3 == (int)all_entities.size() );
    invalid_count = 0;
    for (size_t j = 0; j < all_entities.size(); ++j) {
      if (!(is_owner_list[j]) == (local_ids[0] == all_owners[j]))
        ++invalid_count;
    }
  }
  PCHECK;
  ASSERT(0 == invalid_count);
  
    
    // check globally consistent owners for all vertices
  
    // first communicate total number of vertex entries to be sent to root proc
  int local_count = all_verts.size(), global_count = 0;
  ierr = MPI_Reduce( &local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
  CHKERR;
  
    // for each vertex, store { (x << 2) | y, owning part id }
  std::vector<int> vtxdata( 2 * all_verts.size() );
  std::vector<double> coords( 3 * all_verts.size() );
  ierr = get_coords( imesh, &all_verts[0], all_verts.size(), &coords[0] );
  CHKERR;
  for (size_t i = 0; i < all_verts.size(); ++i) {
    int x = (int)round(coords[3*i  ]);
    int y = (int)round(coords[3*i+1]);
    vtxdata[2*i  ] = (x << 2) | y;
    vtxdata[2*i+1] = vert_owners[i];
  }
  
    // collect all data on root procesor
  std::vector<int> all_data( 2*global_count );
  std::vector<int> displ(size), counts(size);
  if (1 == size) {
    std::copy(vtxdata.begin(), vtxdata.end(), all_data.begin());
    counts[0] = vtxdata.size();
    displ[0] = 0;
  }
  else {
    ierr = MPI_Gatherv( &vtxdata[0], vtxdata.size(), MPI_INT,
                        &all_data[0], &counts[0], &displ[0], MPI_INT, 
                        0, MPI_COMM_WORLD );
    CHKERR;
  }
  if (rank == 0) {
      // map from vertex tag to indices into data
    std::multimap<int,int> data_map; 
    for (int i = 0; i < global_count; ++i) {
      std::pair<int,int> p;
      p.first = all_data[2*i];
      p.second = i;
      data_map.insert( p );
    }
    
      // check consistent data for each vtx
    std::multimap<int,int>::const_iterator a, b;
    for (a = data_map.begin(); a != data_map.end(); a = b) {
      for (b = a; b != data_map.end() && a->first == b->first; ++b) {
        int idx1 = a->second;
        int idx2 = b->second;
        if (all_data[2*idx1+1] == all_data[2*idx2+1])
          continue;
        
        ierr = iBase_FAILURE;
        
        int proc1 = std::lower_bound(displ.begin(), displ.end(),2*idx1) - displ.begin();
        if (displ[proc1] != 2*idx1)
          ++proc1;
        int proc2 = std::lower_bound(displ.begin(), displ.end(),2*idx2) - displ.begin();
        if (displ[proc2] != 2*idx2)
          ++proc2;
        
        std::cerr << "Error at " __FILE__ ":" << __LINE__ << " : " << std::endl
                  << "  For vertex at (" << (a->first >> 2) << ", " << (a->first & 3) << ") :" << std::endl
                  << "  Processor " << proc1 << " has " << all_data[2*idx1+1] << " as the owning part" << std::endl
                  << "  Processor " << proc2 << " has " << all_data[2*idx2+1] << " as the owning part" << std::endl;
      }
    }
  }
  return ierr;
}

static int get_part_boundary_verts( iMesh_Instance imesh,
                                    iMeshP_PartitionHandle prtn,
                                    const PartMap& map,
                                    iMeshP_PartHandle part,
                                    std::vector<iBase_EntityHandle>& boundary )
{
  int ierr, logical_id;
  ierr = map.part_from_coords( imesh, part, logical_id );
  CHKERR;

  int neighbors[5], num_neighbors;
  get_part_neighbors( logical_id, map.get_parts().size(), neighbors, num_neighbors );

  for (int j = 0; j < num_neighbors; ++j) {
    std::vector<iBase_EntityHandle> iface;
    ierr = interface_verts( imesh, prtn, part, neighbors[j], map, iface );
    CHKERR;
    std::copy( iface.begin(), iface.end(), std::back_inserter(boundary) );
  }

  std::sort( boundary.begin(), boundary.end() );
  boundary.erase( std::unique( boundary.begin(), boundary.end() ), boundary.end() );
  return iBase_SUCCESS;
}

/**\brief Test entity status
 *
 * Test:
 * - iMeshP_getEntStatus
 * - iMeshP_getEntStatusArr
 */
int test_entity_status( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr, rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
    
    // get local part handles
  std::vector<iMeshP_PartHandle> parts;
  ierr = get_local_parts( imesh, prtn, parts );
  PCHECK;

    // for each part
  int num_quad_ent_incorrect = 0, num_quad_ent_error = 0;
  int num_quad_arr_incorrect = 0, num_quad_arr_error = 0;
  int num_vert_ent_incorrect = 0, num_vert_ent_error = 0;
  int num_vert_arr_incorrect = 0, num_vert_arr_error = 0;
  for (size_t i = 0; i < parts.size(); ++i) {
    const iMeshP_PartHandle part = parts[i];
    
      // get quads and vertices
    std::vector<iBase_EntityHandle> quads, verts;
    ierr = get_part_quads_and_verts( imesh, part, quads, verts );
    if (ierr)
      break;
    
      // check quad status (no ghosting yet)
    for (size_t j = 0; j < quads.size(); ++j) {
      int status;
      iMeshP_getEntStatus( imesh, prtn, part, quads[j], &status, &ierr );
      if (ierr != iBase_SUCCESS) {
        ++num_quad_ent_error;
        ierr = iBase_SUCCESS;
        continue;
      }
      
      if (status != iMeshP_INTERNAL)
        ++num_quad_ent_incorrect;
    }
    
      // check quad status using iMeshP_getEntStatusArr
    std::vector<int> stat_list(quads.size());
    int* junk1 = &stat_list[0];
    int junk2 = stat_list.size(), count;
    iMeshP_getEntStatusArr( imesh, prtn, part, &quads[0], quads.size(),
                            &junk1, &junk2, &count, &ierr );
    if (ierr != iBase_SUCCESS) {
      ++num_quad_arr_error;
      ierr = iBase_SUCCESS;
      continue;
    }
    assert( junk1 == &stat_list[0] );
    assert( junk2 == (int)stat_list.size() );
    assert( count == (int)quads.size() );
    for (size_t j = 0; j < quads.size(); ++j)
      if (stat_list[j] != iMeshP_INTERNAL)
        ++num_quad_arr_incorrect;
    
      // figure out which vertices are on the boundary
    std::vector<iBase_EntityHandle> boundary;
    ierr = get_part_boundary_verts(imesh, prtn, map, part, boundary);
    if (ierr)
      break;
    std::sort( boundary.begin(), boundary.end() );
    
      // check vertex status (no ghosting yet)
    for (size_t j = 0; j < verts.size(); ++j) {
      int status;
      iMeshP_getEntStatus( imesh, prtn, part, verts[j], &status, &ierr );
      if (ierr != iBase_SUCCESS) {
        ++num_vert_ent_error;
        ierr = iBase_SUCCESS;
        continue;
      }
      bool on_boundary = std::binary_search( boundary.begin(), boundary.end(), verts[j] );
      if (status != (on_boundary ? iMeshP_BOUNDARY : iMeshP_INTERNAL))
         ++num_vert_ent_incorrect;
    }
     
      // check vert status using iMeshP_getEntStatusArr
    stat_list.resize(verts.size());
    junk1 = &stat_list[0];
    junk2 = stat_list.size();
    iMeshP_getEntStatusArr( imesh, prtn, part, &verts[0], verts.size(),
                            &junk1, &junk2, &count, &ierr );
    if (ierr != iBase_SUCCESS) {
      ++num_vert_arr_error;
      ierr = iBase_SUCCESS;
      continue;
    }
    assert( junk1 == &stat_list[0] );
    assert( junk2 == (int)stat_list.size() );
    assert( count == (int)verts.size() );
    for (size_t j = 0; j < verts.size(); ++j) {
      bool on_boundary = std::binary_search( boundary.begin(), boundary.end(), verts[j] );
      if (stat_list[j] != (on_boundary ? iMeshP_BOUNDARY : iMeshP_INTERNAL))
         ++num_vert_arr_incorrect;
    }
  }
  PCHECK; // check if loop interrupted by any internal errors

  ASSERT( 0 == num_quad_ent_error );
  ASSERT( 0 == num_quad_arr_error );
  ASSERT( 0 == num_vert_ent_error );
  ASSERT( 0 == num_vert_arr_error );
  ASSERT( 0 == num_quad_ent_incorrect );
  ASSERT( 0 == num_quad_arr_incorrect );
  ASSERT( 0 == num_vert_ent_incorrect );
  ASSERT( 0 == num_vert_arr_incorrect );
  
  return iBase_SUCCESS;
}

/**\brief Test information about entity copies for interface entities
 *
 * Test:
 * - iMeshP_getNumCopies
 * - iMeshP_getCopyParts
 */
int test_entity_copy_parts( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr, rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
    
    // get local part handles
  std::vector<iMeshP_PartHandle> parts;
  ierr = get_local_parts( imesh, prtn, parts );
  PCHECK;
  ASSERT( !parts.empty() );

    // select a singe part to test
  const iMeshP_PartHandle part = parts[0];
  int logical_id;
  ierr = map.part_from_coords( imesh, part, logical_id );
  CHKERR;
  const iMeshP_Part part_id = map.part_id_from_local_id( logical_id );
  
    // get vertices in part
  std::vector<iBase_EntityHandle> quads, verts;
  ierr = get_part_quads_and_verts( imesh, part, quads, verts );
  PCHECK;
  
    // get neighbors
  int neighbors[5], num_neighbors;
  get_part_neighbors( logical_id, map.get_parts().size(), neighbors, num_neighbors );
    
    // build map of sharing data for each vertex
  std::map< iBase_EntityHandle, std::vector<iMeshP_Part> > vert_sharing;
  for (int j = 0; j < num_neighbors; ++j) {
    std::vector<iBase_EntityHandle> iface;
    ierr = interface_verts( imesh, prtn, part, neighbors[j], map, iface );
    CHKERR;
    for (size_t k = 0; k < iface.size(); ++k)
      vert_sharing[iface[k]].push_back( map.part_id_from_local_id( neighbors[j] ) );
  }
  
    // test getNumCopies for each vertex
  std::map< iBase_EntityHandle, std::vector<iMeshP_Part> >::iterator i;
  int num_failed = 0, num_incorrect = 0;
  for (i = vert_sharing.begin(); i != vert_sharing.end(); ++i) {
    int count;
    iBase_EntityHandle vtx = i->first;
    iMeshP_getNumCopies( imesh, prtn, vtx, &count, &ierr );
    if (ierr)
      ++num_failed;
    else if ((unsigned)count != i->second.size()+1) // add one for the part we queried from
      ++num_incorrect;
  }
  ASSERT( 0 == num_failed );
  ASSERT( 0 == num_incorrect );
  
    // get getCopyParts for each vertex
  num_failed = num_incorrect = 0;
  for (i = vert_sharing.begin(); i != vert_sharing.end(); ++i) {
    iMeshP_Part* list = 0;
    int junk = 0, count;
    iMeshP_getCopyParts( imesh, prtn, i->first, &list, &junk, &count, &ierr );
    if (iBase_SUCCESS != ierr) {
      ++num_failed;
      continue;
    }
    if ((unsigned)count != i->second.size()+1) { // add one for the part we queried from
      ++num_incorrect;
      free(list);
      continue;
    }
    
    std::vector<iMeshP_Part> expected( i->second );
    expected.push_back( part_id );
    std::sort( list, list+count );
    std::sort( expected.begin(), expected.end() );
    bool eq = std::equal( list, list+count, expected.begin() );
    free( list );
    if (!eq)
      ++num_incorrect;
  }
  ASSERT( 0 == num_failed );
  ASSERT( 0 == num_incorrect );
  
  return iBase_SUCCESS;
}

// store remote handle data for a vertex
struct VtxCopyData {
  std::vector<iMeshP_Part> parts;
  std::vector<iBase_EntityHandle> handles;
};

/**\brief Test information about entity copies for interface entities
 *
 * Test:
 * - iMeshP_getCopies
 * - iMeshP_getCopyOnPart
 * - iMeshP_getOwnerCopy
 */
int test_entity_copies( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& /* map */ )
{
  int ierr, rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  // generate a unique ID for each vertex using the coordinates.
  // see create_mesh(..): each vertex has integer coordinates (x,y,0)
  //                      with x in [0,inf] and y in [0,4]
  // then to an Allgatherv to exchange handles for each processor
  
  // cast everything to iBase_EntityHandle so we can pack it all in one communication
  MPI_Datatype tmp_type;
  if (sizeof(iBase_EntityHandle) == sizeof(unsigned))
    tmp_type = MPI_UNSIGNED;
  else if (sizeof(iBase_EntityHandle) == sizeof(unsigned long))
    tmp_type = MPI_UNSIGNED_LONG;
  else
    return iBase_FAILURE;
  const MPI_Datatype type = tmp_type; // make it const
    
    // get local part handles
  std::vector<iMeshP_PartHandle> parts;
  ierr = get_local_parts( imesh, prtn, parts );
  PCHECK;
  std::vector<iMeshP_Part> part_ids(parts.size());
  iMeshP_Part* junk1 = &part_ids[0];
  int junk2 = part_ids.size(), junk3;
  iMeshP_getPartIdsFromPartHandlesArr( imesh, prtn, &parts[0], parts.size(),
                                       &junk1, &junk2, &junk3, &ierr );
  PCHECK;
  assert( junk1 == &part_ids[0] );
  assert( junk2 == (int)part_ids.size() );
  assert( junk3 == (int)parts.size() );
  
    // build list of {vtx_id, part_id, handle} tuples to send
    // also build list of local vertex handles
  std::vector<iBase_EntityHandle> local_data, local_vertices;
  for (size_t i = 0; i < parts.size(); ++i) {
      // get vertices
    std::vector<iBase_EntityHandle> quads, verts;
    ierr = get_part_quads_and_verts( imesh, parts[i], quads, verts );
    if (ierr)
      break;
    
      // add all vertices to local_data
    for (size_t j = 0; j < verts.size(); ++j) {
      int tag;
      ierr = vertex_tag( imesh, verts[j], tag );
      if (ierr)
        break;
      long tmp_h = tag;
      local_data.push_back((iBase_EntityHandle)tmp_h);
      tmp_h = part_ids[i];
      local_data.push_back((iBase_EntityHandle)tmp_h);
      local_data.push_back( verts[j] );
    }
    if (ierr)
      break;
      
    std::copy( verts.begin(), verts.end(), std::back_inserter(local_vertices) );
  }
  
    // build list of local vertices
  std::sort( local_vertices.begin(), local_vertices.end() );
  local_vertices.erase( std::unique( local_vertices.begin(), local_vertices.end() ), local_vertices.end() );
  std::vector<int> local_vtx_tags(local_vertices.size());
  CHKERR;
  for (size_t i = 0; i < local_vertices.size(); ++i) {
    ierr = vertex_tag( imesh, local_vertices[i], local_vtx_tags[i] );
    if (ierr)
      break;
  }
  CHKERR;
  
    // communicate data
  std::vector<int> gcounts(size), gdisp(size);
  int local_data_size = local_data.size();
  ierr = MPI_Allgather( &local_data_size, 1, MPI_INT, &gcounts[0], 1, MPI_INT, MPI_COMM_WORLD );
  CHKERR;
  gdisp[0] = 0;
  for (int i = 1; i < size; ++i)
    gdisp[i] = gdisp[i-1]+gcounts[i-1];
  std::vector<iBase_EntityHandle> global_data( gdisp[size-1]+gcounts[size-1] );
  ierr = MPI_Allgatherv( &local_data[0], local_data_size, type, 
                         &global_data[0], &gcounts[0], &gdisp[0], type, MPI_COMM_WORLD );
  CHKERR;
  
    // arrange global data in a more useful way
  std::map<int,VtxCopyData> vtx_sharing;
  assert( global_data.size() % 3 == 0 );
  for (size_t i = 0; i < global_data.size(); i += 3) {
    int tag =                  (int)(size_t)global_data[i  ];
    iMeshP_Part part = (iMeshP_Part)(size_t)global_data[i+1];
    iBase_EntityHandle handle =             global_data[i+2];
    vtx_sharing[tag].parts.push_back( part );
    vtx_sharing[tag].handles.push_back( handle );
  }
  
    // test iMeshP_getCopies for each local vertex
  int num_error = 0, num_incorrect = 0, junk4;
  for (size_t i = 0; i < local_vertices.size(); ++i) {
    int num_copies = -1;
    //iMeshP_Part* part_ids = 0;
    iMeshP_Part* ptr_part_ids = 0; // Use ptr_part_ids to avoid shadowing std::vector<iMeshP_Part> part_ids
    iBase_EntityHandle* copies = 0;
    junk2 = junk3 = junk4 = 0;
    iMeshP_getCopies( imesh, prtn, local_vertices[i],
                      &ptr_part_ids, &junk2, &num_copies,
                      &copies, &junk3, &junk4, &ierr );
    if (iBase_SUCCESS != ierr) {
      ++num_error;
      continue;
    }
    assert( junk4 == num_copies );
    
    VtxCopyData& expected = vtx_sharing[local_vtx_tags[i]];
    if (num_copies != (int)expected.parts.size())
      ++num_incorrect;
    else for (size_t j = 0; j < expected.parts.size(); ++j) {
      int idx = std::find( ptr_part_ids, ptr_part_ids + num_copies, expected.parts[j] ) - ptr_part_ids;
      if (idx == num_copies || copies[idx] != expected.handles[j]) {
        ++num_incorrect;
        break;
      }
    }
    free(ptr_part_ids);
    free(copies);
  }
  ASSERT( 0 == num_error );
  ASSERT( 0 == num_incorrect );
   
    // test iMeshP_getCopyOnPart for each local vertex
  num_error = num_incorrect = 0;
  for (size_t i = 0; i < local_vertices.size(); ++i) {
    VtxCopyData& expected = vtx_sharing[local_vtx_tags[i]];
    for (size_t j = 0; j < expected.parts.size(); ++j) {
      iBase_EntityHandle copy;
      iMeshP_getCopyOnPart( imesh, prtn, local_vertices[i], expected.parts[j], &copy, &ierr );
      if (iBase_SUCCESS != ierr)
        ++num_error;
      else if (expected.handles[j] != copy)
        ++num_incorrect;
    }
  }
  ASSERT( 0 == num_error );
  ASSERT( 0 == num_incorrect );

    // test iMeshP_getOwnerCopy for each local vertex
  num_error = num_incorrect = 0;
  for (size_t i = 0; i < local_vertices.size(); ++i) {
    VtxCopyData& expected = vtx_sharing[local_vtx_tags[i]];
    iMeshP_Part owner_id = 0;
    iMeshP_getEntOwnerPart( imesh, prtn, local_vertices[i], &owner_id, &ierr );
    if (iBase_SUCCESS != ierr) 
      continue; // not testing getEntOwnerPart here
    
    size_t idx = std::find( expected.parts.begin(), expected.parts.end(), owner_id )
                   - expected.parts.begin();
    if (idx == expected.parts.size())
      continue; // not testing getEntOwnerPart here
    
    iMeshP_Part owner_id_2 = 0;
    iBase_EntityHandle copy = 0;
    iMeshP_getOwnerCopy( imesh, prtn, local_vertices[i], &owner_id_2, &copy, &ierr );
    if (iBase_SUCCESS != ierr)
      ++num_error;
    else if (owner_id_2 != owner_id && copy != expected.handles[idx])
      ++num_incorrect;
  }
  ASSERT( 0 == num_error );
  ASSERT( 0 == num_incorrect );
  
  return iBase_SUCCESS;
}


int get_num_adj_quads( iMesh_Instance imesh, iBase_EntityHandle vtx, int& num )
{
  iBase_EntityHandle* list = 0;
  int ierr, junk = 0;
  iMesh_getEntAdj( imesh, vtx, iBase_FACE, &list, &junk, &num, &ierr );
  if (iBase_SUCCESS == ierr)
    free(list);
  return ierr;
}

int get_adj( iMesh_Instance imesh, 
             iBase_EntityHandle ent, 
             int type,
             std::vector<iBase_EntityHandle>& adj )
{
  iBase_EntityHandle* list = 0;
  int ierr, num, junk = 0;
  iMesh_getEntAdj( imesh, ent, type, &list, &junk, &num, &ierr );
  if (iBase_SUCCESS == ierr) {
    std::copy( list, list+num, std::back_inserter(adj) );
    free( list );
  }
  return ierr;
}

// assume regular quad mesh
int get_boundary_vertices( iMesh_Instance imesh, std::vector<iBase_EntityHandle>& bdry )
{
  int ierr, n;
  iBase_EntitySetHandle root;
  iMesh_getRootSet( imesh, &root, &ierr );
  CHKERR;
  std::vector<iBase_EntityHandle> all_verts;
  ierr = get_entities( imesh, root, iBase_VERTEX, iMesh_POINT, all_verts );
  CHKERR;
  bdry.clear();
  for (size_t i = 0; i < all_verts.size(); ++i) {
    ierr = get_num_adj_quads( imesh, all_verts[i], n );
    CHKERR;
    if (n != 4)
      bdry.push_back( all_verts[i] );
  }
  return iBase_SUCCESS;
}

int check_one_layer( iMesh_Instance imesh, iBase_EntityHandle vtx,
                     const std::vector<iBase_EntityHandle>& sorted_vertices )
{
  int ierr;
  if (std::binary_search( sorted_vertices.begin(), sorted_vertices.end(), vtx ))
    return iBase_SUCCESS;
  std::vector<iBase_EntityHandle> quads, verts;
  ierr = get_adj( imesh, vtx, iBase_FACE, quads );
  CHKERR;
  for (size_t i = 0; i < quads.size(); ++i) {
    verts.clear();
    ierr = get_adj( imesh, quads[i], iBase_VERTEX, verts );
    CHKERR;
    for (size_t j = 0; j < verts.size(); ++j) {
      if (std::binary_search( sorted_vertices.begin(), sorted_vertices.end(), verts[j] ))
        return iBase_SUCCESS;
    }
  }
  
  return iBase_FAILURE;
}
  
// get number of adjacent quads to each vertex, both on the local
// processor and in the entire mesh
int get_num_adj_all( iMesh_Instance imesh, 
                     const std::vector<iBase_EntityHandle>& verts,
                     std::vector<int>& num_local_adj,
                     std::vector<int>& num_all_adj )
{
  int ierr, size;
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  std::vector<int> vtx_tags(verts.size());
  num_local_adj.resize( verts.size() );
  for (size_t i = 0; i < verts.size(); ++i) {
    ierr = get_num_adj_quads( imesh, verts[i], num_local_adj[i] );
    CHKERR;
    ierr = vertex_tag( imesh, verts[i], vtx_tags[i] );
    CHKERR;
  }
  
  std::vector<int> counts(size), displ(size);
  int num_vtx = verts.size();
  ierr = MPI_Allgather( &num_vtx, 1, MPI_INT, &counts[0], 1, MPI_INT, MPI_COMM_WORLD );
  CHKERR;
  displ[0] = 0;
  for (int i = 1; i < size; ++i)
    displ[i] = displ[i-1]+counts[i-1];
  int total = displ[size-1]+counts[size-1];
  std::vector<int> all_tags(total), all_adj_counts(total);
  ierr = MPI_Allgatherv( &vtx_tags[0], vtx_tags.size(), MPI_INT, &all_tags[0], &counts[0], &displ[0], MPI_INT, MPI_COMM_WORLD );
  CHKERR;
  ierr = MPI_Allgatherv( &num_local_adj[0], num_local_adj.size(), MPI_INT, &all_adj_counts[0], &counts[0], &displ[0], MPI_INT, MPI_COMM_WORLD );
  CHKERR;
  
  num_all_adj.clear();
  num_all_adj.resize(total,0);
  for (int i = 0; i < total; ++i) {
    std::vector<int>::iterator it = std::find( vtx_tags.begin(), vtx_tags.end(), all_tags[i] );
    if (it == vtx_tags.end())
      continue;
    int idx = it - vtx_tags.begin();
    num_all_adj[idx] += all_adj_counts[i];
  }
  
  return iBase_SUCCESS;
}


/**\brief Test creation of ghost entities
 *
 * Test:
 * - iMeshP_createGhostEntsAll
 */
int test_create_ghost_ents( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& /* map */)
{
  int ierr;
  
    // get boundary vertices
  std::vector<iBase_EntityHandle> bdry;
  ierr = get_boundary_vertices( imesh, bdry );
  PCHECK;
    // get counts of adjacent entities
  std::vector<int> num_local_adj, num_global_adj;
  ierr = get_num_adj_all( imesh, bdry, num_local_adj, num_global_adj );
  PCHECK;
    // create one layer of ghost entities
  iMeshP_createGhostEntsAll( imesh, prtn, iBase_FACE, iBase_VERTEX, 1, 0, &ierr );
  PCHECK;
    // check that each vertex has the correct number of adjacent entities
  int num_incorrect = 0;
  for (size_t i = 0; i < bdry.size(); ++i) {
    int n;
    ierr = get_num_adj_quads( imesh, bdry[i], n );
    if (iBase_SUCCESS != ierr || num_global_adj[i] != n)
      ++num_incorrect;
  }
  ASSERT( 0 == num_incorrect );
    // get new the new boundary
  std::vector<iBase_EntityHandle> new_bdry;
  ierr = get_boundary_vertices( imesh, new_bdry );
  PCHECK;
    // check that each vertex on the new boundary is separated by
    // at most one layer from the old boundary
  std::sort( bdry.begin(), bdry.end() );
  num_incorrect = 0;
  for (size_t i = 0; i < new_bdry.size(); ++i) {
    ierr = check_one_layer( imesh, new_bdry[i], bdry );
    if (ierr) 
      ++num_incorrect;
  }
  ASSERT( 0 == num_incorrect );
    // make another layer of ghost entiites
  bdry.swap( new_bdry );
  new_bdry.clear();
  ierr = get_num_adj_all( imesh, bdry, num_local_adj, num_global_adj );
  PCHECK;
  iMeshP_createGhostEntsAll( imesh, prtn, iBase_FACE, iBase_VERTEX, 2, 0, &ierr );
  PCHECK;
    // check that each vertex has the correct number of adjacent entities
  num_incorrect = 0;
  for (size_t i = 0; i < bdry.size(); ++i) {
    int n;
    ierr = get_num_adj_quads( imesh, bdry[i], n );
    if (iBase_SUCCESS != ierr || num_global_adj[i] != n)
      ++num_incorrect;
  }
    // check that each vertex on the new boundary is separated by
    // at most one layer from the old boundary
  std::sort( bdry.begin(), bdry.end() );
  num_incorrect = 0;
  for (size_t i = 0; i < new_bdry.size(); ++i) {
    ierr = check_one_layer( imesh, new_bdry[i], bdry );
    if (ierr) 
      ++num_incorrect;
  }
  ASSERT( 0 == num_incorrect );
  
  return iBase_SUCCESS;
}  

/**\brief Test exchange entities
 *
 * Test:
 * - iMeshP_exchEntArrToPartsAll
 */
int test_exchange_ents( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& map )
{
  int ierr, rank, size;
  int num_err = 0;
  iMeshP_RequestHandle request;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  
  std::vector<iBase_EntityHandle> all_elems;
  std::vector<iMeshP_Part> all_ids;
  std::vector<iBase_EntityHandle> quads;
  
  // get local part handles and part ids
  std::vector<iMeshP_PartHandle> local_handles;
  std::vector<iMeshP_Part> local_ids;
  ierr = get_local_parts( imesh, prtn, local_handles, &local_ids );
  PCHECK;

  // get loacal quads before exchange
  quads.clear();
  ierr = get_entities( imesh, local_handles[0], iBase_FACE, iMesh_QUADRILATERAL, quads );
  CHKERR;
  int n_quads = quads.size();

  // send all elements in local processor to all other processors
  for (size_t i = 0; i < map.get_parts().size(); ++i) {
    if (map.get_parts()[i] == (unsigned int) rank) continue; // skip own rank
    
    for (int j = 0; j < n_quads; j++) {
      all_elems.push_back(quads[j]);
      all_ids.push_back(map.get_parts()[i]);
    }
  }
  
  // exchange entities
  iMeshP_exchEntArrToPartsAll(imesh, prtn, &all_elems[0], all_elems.size(),
                              &all_ids[0], 0, 0, &request, &ierr);
  if (iBase_SUCCESS != ierr) ++num_err;
  
  // get local quads after exchange
  quads.clear();
  ierr = get_entities( imesh, local_handles[0], iBase_FACE, iMesh_QUADRILATERAL, quads );
  CHKERR;

  // # of elements should be # of quads * # of processors
  ASSERT(quads.size() == (unsigned int) n_quads*size);
  
  ASSERT(0 == num_err);
  
  return iBase_SUCCESS;
} 

/**\brief Test commuinication of tag data
 *
 * Test:
 * - iMeshP_pushTags
 * - iMeshP_pushTagsEnt
 */
int test_push_tag_data_common( iMesh_Instance imesh, 
                               iMeshP_PartitionHandle prtn, 
                               int num_ghost_layers )
{
  const char* src_name = "test_src";
  const char* dst_name = "test_dst";
  int ierr, rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  
  if (num_ghost_layers) {
    iMeshP_createGhostEntsAll( imesh, prtn, iBase_FACE, iBase_VERTEX, num_ghost_layers, 0, &ierr );
    PCHECK;
  }
  
  iBase_TagHandle src_tag, dst_tag;
  iMesh_createTag( imesh, src_name, 1, iBase_INTEGER, &src_tag, &ierr, strlen(src_name) );
  CHKERR;
  iMesh_createTag( imesh, dst_name, 1, iBase_INTEGER, &dst_tag, &ierr, strlen(dst_name) );
  CHKERR;
  
  iBase_EntitySetHandle root;
  iMesh_getRootSet( imesh, &root, &ierr ); CHKERR;
  
  std::vector<iBase_EntityHandle> verts;
  ierr = get_entities( imesh, root, iBase_VERTEX, iMesh_POINT, verts );
  CHKERR;
  
    // test iMeshP_pushTags
    // each processor writes its rank on all vertices
    // after push, each vertex should be tagged with the rank of its owner
    
  std::vector<int> tag_vals( verts.size(), rank );
  iMesh_setIntArrData( imesh, &verts[0], verts.size(), src_tag, &tag_vals[0], tag_vals.size(), &ierr );
  CHKERR;
  
  iMeshP_pushTags( imesh, prtn, src_tag, dst_tag, iBase_VERTEX, iMesh_POINT, &ierr );
  PCHECK;
  
  tag_vals.clear();
  tag_vals.resize( verts.size(), -1 );
iBase_TagHandle id_tag;
iMesh_getTagHandle( imesh, "GLOBAL_ID", &id_tag, &ierr, strlen("GLOBAL_ID") );
std::vector<int> ids(verts.size());
int* junk1 = &ids[0], junk2 = ids.size(), junk3;
iMesh_getIntArrData( imesh, &verts[0], verts.size(), id_tag, &junk1, &junk2, &junk3, &ierr );
PCHECK;
int errcount = 0;
for (size_t i = 0; i < verts.size(); ++i) {
  iMesh_getIntData( imesh, verts[i], dst_tag, &tag_vals[i], &ierr );
  if (ierr != iBase_SUCCESS) {
    std::cerr << "Rank " << rank << " : getIntData failed for vertex " << ids[i] << std::endl;
    std::cerr.flush();
    ++errcount;
  }
}
ASSERT(0 == errcount);

//  int *junk1 = &tag_vals[0], junk2 = tag_vals.size(), junk3;
//  iMesh_getIntArrData( imesh, &verts[0], verts.size(), dst_tag, &junk1, &junk2, &junk3, &ierr );
//  PCHECK;
//  assert( junk1 == &tag_vals[0] );
//  assert( junk2 == (int)tag_vals.size() );
//  assert( junk3 == (int)verts.size() );
  
  std::vector<int> expected( verts.size() );
  std::vector<iMeshP_Part> parts( verts.size() );
  iMeshP_Part* junk4 = &parts[0];
  junk2 = parts.size();
  iMeshP_getEntOwnerPartArr( imesh, prtn, &verts[0], verts.size(), &junk4, &junk2, &junk3, &ierr );
  PCHECK;
  assert(junk4 == &parts[0]);
  assert(junk2 == (int)parts.size());
  assert(junk3 == (int)verts.size());
  junk1 = &expected[0];
  junk2 = expected.size();
  iMeshP_getRankOfPartArr( imesh, prtn, &parts[0], parts.size(), &junk1, &junk2, &junk3, &ierr );
  PCHECK;
  assert(junk1 == &expected[0]);
  assert(junk2 == (int)expected.size());
  assert(junk3 == (int)parts.size());
  
  ASSERT( tag_vals == expected );
  
  
  
    // test iMeshP_pushTagsEnt
    // write -1 on all vertices
    // For each vertex owned by this processor and shared with more than
    // two others, write the rank of the owning processor.
  
  tag_vals.clear();
  tag_vals.resize( verts.size(), -1 );
  iMesh_setIntArrData( imesh, &verts[0], verts.size(), src_tag, &tag_vals[0], tag_vals.size(), &ierr );
  PCHECK;
  tag_vals.resize( verts.size(), -1 );
  iMesh_setIntArrData( imesh, &verts[0], verts.size(), dst_tag, &tag_vals[0], tag_vals.size(), &ierr );
  PCHECK;
  
  std::vector<iBase_EntityHandle> some;
  for (size_t i = 0; i < verts.size(); ++i) {
    int num;
    iMeshP_getNumCopies( imesh, prtn, verts[i], &num, &ierr );
    if (iBase_SUCCESS != ierr)
      break;
    if (num > 2)
      some.push_back( verts[i] );
    else 
      expected[i] = -1;
  }

  tag_vals.clear();
  tag_vals.resize( some.size(), rank );
  iMesh_setIntArrData( imesh, &some[0], some.size(), src_tag, &tag_vals[0], tag_vals.size(), &ierr );
  PCHECK;
  
  iMeshP_pushTagsEnt( imesh, prtn, src_tag, dst_tag, &some[0], some.size(), &ierr );
  PCHECK;
  
  tag_vals.clear();
  tag_vals.resize( verts.size(), -1 );
  junk1 = &tag_vals[0];
  junk2 = tag_vals.size();
  iMesh_getIntArrData( imesh, &verts[0], verts.size(), dst_tag, &junk1, &junk2, &junk3, &ierr );
  CHKERR;
  assert( junk1 == &tag_vals[0] );
  assert( junk2 == (int)tag_vals.size() );
  assert( junk3 == (int)verts.size() );
  
  ASSERT( tag_vals == expected );
  return iBase_SUCCESS;
}

/**\brief Test commuinication of tag data
 *
 * Test:
 * - iMeshP_pushTags
 * - iMeshP_pushTagsEnt
 */
int test_push_tag_data_iface( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& )
{
  return test_push_tag_data_common( imesh, prtn, 0 );
}

/**\brief Test commuinication of tag data
 *
 * Test:
 * - iMeshP_pushTags
 * - iMeshP_pushTagsEnt
 */
int test_push_tag_data_ghost( iMesh_Instance imesh, iMeshP_PartitionHandle prtn, const PartMap& )
{
  return test_push_tag_data_common( imesh, prtn, 1 );
}


/**************************************************************************
                          PartMap class
 **************************************************************************/

 


int PartMap::build_map( iMesh_Instance imesh,
                        iMeshP_PartitionHandle prtn,
                        int num_expected_parts )
{
  int ierr, rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  
    // get local parts
  std::vector<iMeshP_PartHandle> local_parts;
  std::vector<iMeshP_Part> imesh_ids;
  ierr = get_local_parts( imesh, prtn, local_parts, &imesh_ids );
  CHKERR;
  
    // get logical ids for local parts
  std::vector<int> local_ids(local_parts.size());
  for (size_t i = 0; i < local_parts.size(); ++i) {
    ierr = part_from_coords( imesh, local_parts[i], local_ids[i] );
    CHKERR;
  }
  
    // get total number of parts
  int num_global = 0, num_local = local_parts.size();
  ierr = MPI_Allreduce( &num_local, &num_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
  CHKERR;
  if (num_global != num_expected_parts) {
    std::cerr << "Invalid/unexpected global part count at " __FILE__ ":" 
              << __LINE__ << " (proc " << rank << "): " << std::endl
              << "  Expected: " << num_expected_parts << std::endl
              << "  Actual:   " << num_global << std::endl;
    return 1;
  }
  
    // get counts and displacements for Allgatherv calls
  std::vector<int> dspls(size), counts(size);
  ierr = MPI_Allgather( &num_local, 1, MPI_INT, &counts[0], 1, MPI_INT, MPI_COMM_WORLD );
  CHKERR;
  dspls[0] = 0;
  for (int i = 1; i < size; ++i)
    dspls[i] = dspls[i-1] + counts[i-1];
 
    // gather iMeshP_Part list from each processor
  std::vector<unsigned> global_part_ids(num_expected_parts);
  assert(sizeof(iMeshP_Part) == sizeof(int));
  ierr = MPI_Allgatherv( &imesh_ids[0], num_local, MPI_UNSIGNED,
                         &global_part_ids[0], &counts[0], &dspls[0], MPI_UNSIGNED,
                         MPI_COMM_WORLD );
  CHKERR;
  
    // gather local ids from each processor
  std::vector<int> global_id_list(num_expected_parts);
  ierr = MPI_Allgatherv( &local_ids[0], num_local, MPI_INT,
                         &global_id_list[0], &counts[0], &dspls[0], MPI_INT,
                         MPI_COMM_WORLD );
  CHKERR;
  
    // build owner list
  std::vector<int> global_owners(num_expected_parts);
  for (int i = 0; i < size; ++i) 
    for (int j = 0; j < counts[i]; ++j)
      global_owners[dspls[i]+j] = i;
      
      
    // populate member lists
  sortedPartList = global_part_ids;
  std::sort( sortedPartList.begin(), sortedPartList.end() );
  partLocalIds.resize( num_expected_parts );
  partRanks.resize( num_expected_parts );
  for (int i = 0; i < num_expected_parts; ++i) {
    int idx = std::lower_bound( sortedPartList.begin(), sortedPartList.end(), global_part_ids[i] ) - sortedPartList.begin();
    partLocalIds[idx] = global_id_list[i];
    partRanks[idx] = global_owners[i];
  }
  
    // do some consistency checking
  if (std::unique( sortedPartList.begin(), sortedPartList.end() ) != sortedPartList.end()) {
    if (rank == 0) {
      std::cerr << "ERROR: Duplicate iMeshP_Part values detected at " __FILE__ ":" << __LINE__ << std::endl;
    }
    return 1;
  }
  
    // build revesre local id map and check for duplicates
  localIdReverseMap.clear();
  localIdReverseMap.resize(num_expected_parts, -1);
  for (int i = 0; i < num_expected_parts; ++i) {
    int idx = partLocalIds[i];
    if (localIdReverseMap[idx] != -1) {
      if (rank == 0) {
        std::cerr << "ERROR: Part mesh has been duplicated in multiple parts." << std::endl
                  << "  Detected at " __FILE__ ":" << __LINE__ << std::endl
                  << "  See PartMap::part_from_coords" << std::endl;
      }
      return 1;
    }
    if (idx >= num_expected_parts) {
      if (rank == 0) {
        std::cerr << "ERROR: Part mesh invalid/incorrect mesh." << std::endl
                  << "  Detected at " __FILE__ ":" << __LINE__ << std::endl
                  << "  See PartMap::part_from_coords" << std::endl;
      }
      return 1;
    }
      
    localIdReverseMap[idx] = i;
  }
  
  return 0;
}


void PartMap::part_id_from_rank( int rank, std::vector<iMeshP_Part>& parts ) const
{
  for (size_t i = 0; i < sortedPartList.size(); ++i)
    if (partRanks[i] == rank)
      parts.push_back( sortedPartList[i] ); 
}
  
void PartMap::local_id_from_rank( int rank, std::vector<int>& ids ) const
{
  for (size_t i = 0; i < sortedPartList.size(); ++i)
    if (partRanks[i] == rank)
      ids.push_back( partLocalIds[i] ); 
}
  

int PartMap::part_from_coords( iMesh_Instance imesh, iMeshP_PartHandle part, int& id )
{
  int ierr, rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    // get elements
  const int num_elem = 4;
  iBase_EntityHandle array[num_elem];
  iBase_EntityHandle* ptr = array;
  int junk1 = num_elem, n = -1;
  iMesh_getEntities( imesh, part, iBase_FACE, iMesh_QUADRILATERAL, &ptr, &junk1, &n, &ierr );
  CHKERR;
  assert( ptr == array );
  assert( junk1 == num_elem );
  if (n != num_elem) {
    std::cerr << "Internal error at " __FILE__ ":" << __LINE__  << " (proc " 
              << rank << "): Expected all parts to have " << num_elem 
              << " elements.  Found one with " << n << std::endl;
    return 1;
  }
  
    // get vertices
  iBase_EntityHandle adj_array[4*num_elem];
  int junk2, junk3, offset_array[5];
  ptr = adj_array;
  junk1 = sizeof(adj_array)/sizeof(adj_array[0]);
  junk2 = sizeof(offset_array)/sizeof(offset_array[0]);
  int* ptr2 = offset_array;
  iMesh_getEntArrAdj( imesh, array, num_elem, iBase_VERTEX,
                      &ptr, &junk1, &n,
                      &ptr2, &junk2, &junk3,
                      &ierr );
  CHKERR;
  assert( ptr == adj_array );
  assert( ptr2 == offset_array );
  assert( junk1 == sizeof(adj_array)/sizeof(adj_array[0]) );
  assert( junk2 == sizeof(offset_array)/sizeof(offset_array[0]) );
  assert( n == 4*num_elem );
  assert( offset_array[0] == 0 );
  for (int i = 1; i < junk3; ++i)
    assert( offset_array[i] - offset_array[i-1] == 4 );
  
    // find center vertex
  iBase_EntityHandle vtx;
  bool all_match;
  for (int i = 0; i < 4; ++i) {
    vtx = adj_array[i];
    all_match = true;
    for (int j = 1; j < 4; ++j) {
      iBase_EntityHandle* mvtx = adj_array + 4*j;
      int k;
      for (k = 0; k < 4; ++k)
        if (mvtx[k] == vtx)
          break;
      if (k == 4)
        all_match = false;
    }
    if (all_match)
      break;
  }
  assert(all_match);
  
    // get center vertex coordinates
  double x, y, z;
  iMesh_getVtxCoord( imesh, vtx, &x, &y, &z, &ierr );
  CHKERR;
  assert( 0.0 == z );
  const int xi = ((int)round(x) - 1)/2;
  const int yi = ((int)round(y) - 1)/2;
  assert (xi >= 0);
  assert (yi >= 0);
  assert( fabs(x - 2*xi - 1) < 1e-12 );
  assert( fabs(y - 2*yi - 1) < 1e-12 );
  
  id = 2*xi + yi;
  return 0;
}
