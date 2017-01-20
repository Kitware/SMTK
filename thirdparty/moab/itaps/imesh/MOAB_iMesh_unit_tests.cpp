#include "TestRunner.hpp"
#include "iMesh.h"
#include "MBiMesh.hpp"
#include "moab/Core.hpp"
#include <algorithm>

iMesh_Instance create_mesh();

void test_getEntArrAdj_conn();
void test_getEntArrAdj_vertex();
void test_getEntArrAdj_up();
void test_getEntArrAdj_down();
void test_getEntArrAdj_invalid_size();
void test_getEntArrAdj_none();
void test_existinterface();
void test_tags_retrieval();
void test_invalid_parallel_option();

int main( int argc, char* argv[] )
{
  REGISTER_TEST( test_getEntArrAdj_conn );
  REGISTER_TEST( test_getEntArrAdj_vertex );
  REGISTER_TEST( test_getEntArrAdj_up );
  REGISTER_TEST( test_getEntArrAdj_down );
  REGISTER_TEST( test_getEntArrAdj_invalid_size );
  REGISTER_TEST( test_getEntArrAdj_none );
  REGISTER_TEST( test_existinterface );
#ifdef  MOAB_HAVE_HDF5
  REGISTER_TEST( test_tags_retrieval );
#endif
#ifndef MOAB_HAVE_MPI
  REGISTER_TEST( test_invalid_parallel_option );
#endif
  int result = RUN_TESTS( argc, argv );

  // Delete the static iMesh instance defined in create_mesh()
  iMesh_Instance mesh = create_mesh();
  int err;
  iMesh_dtor(mesh, &err);
  CHECK_EQUAL(iBase_SUCCESS, err);

  return result;
}

// INTERVAL x INTERVAL x INTERVAL regular hex mesh with skin faces.
// Vertices are located at even coordinate
// values and adjacent vertices are separated by one unit.  The entire
// grid is in the first octant with the first vertex at the origin.
// Faces are grouped by the side of the grid that they occur in.
// The faces are { -Y, X, Y, -X, -Z, Z }.
const int INTERVALS = 2;
iBase_EntityHandle VERTS[INTERVALS+1][INTERVALS+1][INTERVALS+1];
iBase_EntityHandle HEXES[INTERVALS][INTERVALS][INTERVALS];
iBase_EntityHandle FACES[6][INTERVALS][INTERVALS];

static void HEX_VERTS( int i, int j, int k, iBase_EntityHandle conn[8] ) {
  conn[0] = VERTS[i  ][j  ][k  ];
  conn[1] = VERTS[i+1][j  ][k  ];
  conn[2] = VERTS[i+1][j+1][k  ];
  conn[3] = VERTS[i  ][j+1][k  ];
  conn[4] = VERTS[i  ][j  ][k+1];
  conn[5] = VERTS[i+1][j  ][k+1];
  conn[6] = VERTS[i+1][j+1][k+1];
  conn[7] = VERTS[i  ][j+1][k+1];
}

static void QUAD_VERTS( int f, int i, int j, iBase_EntityHandle conn[4] ) {
  switch (f) {
    case 0: case 2:
      conn[0] = VERTS[i  ][INTERVALS*(f/2)][j  ];
      conn[1] = VERTS[i+1][INTERVALS*(f/2)][j  ];
      conn[2] = VERTS[i+1][INTERVALS*(f/2)][j+1];
      conn[3] = VERTS[i  ][INTERVALS*(f/2)][j+1];
      break;
    case 1: case 3:
      conn[0] = VERTS[INTERVALS*(1/f)][i  ][j  ];
      conn[1] = VERTS[INTERVALS*(1/f)][i+1][j  ];
      conn[2] = VERTS[INTERVALS*(1/f)][i+1][j+1];
      conn[3] = VERTS[INTERVALS*(1/f)][i  ][j+1];
      break;
    case 4: case 5:
      conn[0] = VERTS[i  ][j  ][INTERVALS*(f-4)];
      conn[1] = VERTS[i+1][j  ][INTERVALS*(f-4)];
      conn[2] = VERTS[i+1][j+1][INTERVALS*(f-4)];
      conn[3] = VERTS[i  ][j+1][INTERVALS*(f-4)];
      break;
    default:
      CHECK(false);
  }
}


iMesh_Instance create_mesh() 
{
  static iMesh_Instance instance = 0;
  if (instance)
    return instance;
  
  int err;
  iMesh_Instance tmp;
  iMesh_newMesh( 0, &tmp, &err, 0 );
  CHECK_EQUAL( iBase_SUCCESS, err );
  
  for (int i = 0; i < INTERVALS+1; ++i)
    for (int j = 0; j < INTERVALS+1; ++j)
      for (int k = 0; k < INTERVALS+1; ++k) {
        iMesh_createVtx( tmp, i, j, k, &VERTS[i][j][k], &err );
        CHECK_EQUAL( iBase_SUCCESS, err );
      }
  
  int status;
  iBase_EntityHandle conn[8];
  for (int i = 0; i < INTERVALS; ++i)
    for (int j = 0; j < INTERVALS; ++j)
      for (int k = 0; k < INTERVALS; ++k) {
        HEX_VERTS(i,j,k,conn);
        iMesh_createEnt( tmp, iMesh_HEXAHEDRON, conn, 8, &HEXES[i][j][k], &status, &err );
        CHECK_EQUAL( iBase_SUCCESS, err );
        CHECK_EQUAL( iBase_NEW, status );
      }
  
  
  for (int f = 0; f < 6; ++f) 
    for (int i = 0; i < INTERVALS; ++i) 
      for (int j = 0; j < INTERVALS; ++j) {
        QUAD_VERTS(f,i,j,conn);
        iMesh_createEnt( tmp, iMesh_QUADRILATERAL, conn, 4, &FACES[f][i][j], &status, &err );
        CHECK_EQUAL( iBase_SUCCESS, err );
      }
  
  return (instance = tmp);
}

void test_getEntArrAdj_conn()
{
  iMesh_Instance mesh = create_mesh();
  int err;
  
    // test hex vertices
  for (int i = 0; i < INTERVALS; ++i) {
    for (int j = 0; j < INTERVALS; ++j) {
      iBase_EntityHandle adj[8*INTERVALS];
      int off[INTERVALS+1];
      int adj_alloc = sizeof(adj)/sizeof(adj[0]);
      int off_alloc = sizeof(off)/sizeof(off[0]);
      int adj_size = -1, off_size = -1;
      iBase_EntityHandle* adj_ptr = adj;
      int* off_ptr = off;
      iMesh_getEntArrAdj( mesh, HEXES[i][j], INTERVALS, iBase_VERTEX,
                          &adj_ptr, &adj_alloc, &adj_size, 
                          &off_ptr, &off_alloc, &off_size,
                          &err );
      CHECK_EQUAL( &adj[0], adj_ptr );
      CHECK_EQUAL( &off[0], off_ptr );
      CHECK_EQUAL( iBase_SUCCESS, err );
      CHECK_EQUAL( 8*INTERVALS, adj_size );
      CHECK_EQUAL( 8*INTERVALS, adj_alloc );
      CHECK_EQUAL( INTERVALS+1, off_size );
      CHECK_EQUAL( INTERVALS+1, off_alloc );
      for (int k = 0; k < INTERVALS; ++k) {
        CHECK_EQUAL( 8*k, off[k] );
        iBase_EntityHandle conn[8];
        HEX_VERTS( i, j, k, conn );
        CHECK_ARRAYS_EQUAL( conn, 8, adj + off[k], off[k+1]-off[k] );
      }
    }
  }
  
    // test quad vertices for one side of mesh
  const int f = 0;
  for (int i = 0; i < INTERVALS; ++i) {
    iBase_EntityHandle adj[4*INTERVALS];
    int off[INTERVALS+1];
    int adj_alloc = sizeof(adj)/sizeof(adj[0]);
    int off_alloc = sizeof(off)/sizeof(off[0]);
    int adj_size = -1, off_size = -1;
      iBase_EntityHandle* adj_ptr = adj;
      int* off_ptr = off;
    iMesh_getEntArrAdj( mesh, FACES[f][i], INTERVALS, iBase_VERTEX,
                        &adj_ptr, &adj_alloc, &adj_size, 
                        &off_ptr, &off_alloc, &off_size,
                        &err );
    CHECK_EQUAL( &adj[0], adj_ptr );
    CHECK_EQUAL( &off[0], off_ptr );
    CHECK_EQUAL( iBase_SUCCESS, err );
    CHECK_EQUAL( 4*INTERVALS, adj_size );
    CHECK_EQUAL( 4*INTERVALS, adj_alloc );
    CHECK_EQUAL( INTERVALS+1, off_size );
    CHECK_EQUAL( INTERVALS+1, off_alloc );
    for (int k = 0; k < INTERVALS; ++k) {
      CHECK_EQUAL( 4*k, off[k] );
      iBase_EntityHandle conn[4];
      QUAD_VERTS( f, i, k, conn );
      CHECK_ARRAYS_EQUAL( conn, 4, adj + off[k], off[k+1]-off[k] );
    }
  }
}

void test_getEntArrAdj_vertex()
{
  iMesh_Instance mesh = create_mesh();
  int err;
  
  // get hexes adjacent to row of vertices at x=0,y=0;
  iBase_EntityHandle *adj = 0;
  int *off = 0;
  int adj_alloc = 0, off_alloc = 0;
  int adj_size = -1, off_size = -1;
  iMesh_getEntArrAdj( mesh, VERTS[0][0], INTERVALS+1, iBase_REGION,
                      &adj, &adj_alloc, &adj_size,
                      &off, &off_alloc, &off_size,
                      &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  CHECK( 0 != adj );
  CHECK( 0 != off );
  CHECK_EQUAL( 2*INTERVALS, adj_size ); // INTERVALS+1 verts, end ones with one hex, others with two
  CHECK_EQUAL( INTERVALS+2, off_size ); // one more than number of input handles
  CHECK( adj_alloc >= adj_size );
  CHECK( off_alloc >= off_size );
  
    // first and last vertices should have one adjacent hex
  CHECK_EQUAL( 1, off[1] - off[0] );
  CHECK_EQUAL( HEXES[0][0][0], adj[off[0]] );
  CHECK_EQUAL( 1, off[INTERVALS+1] - off[INTERVALS] );
  CHECK_EQUAL( HEXES[0][0][INTERVALS-1], adj[off[INTERVALS]] );
    // middle ones should have two adjacent hexes
  for (int i = 1; i < INTERVALS; ++i) {
    CHECK_EQUAL( 2, off[i+1] - off[i] );
    CHECK_EQUAL( HEXES[0][0][i-1], adj[off[i]  ] );
    CHECK_EQUAL( HEXES[0][0][i  ], adj[off[i]+1] );
  }
  
  free(adj);
  free(off);
}

void test_getEntArrAdj_up()
{
  iMesh_Instance mesh = create_mesh();
  int err;
  
  // get hexes adjacent to a row of faces in the z=0 plane
  iBase_EntityHandle *adj = 0;
  int *off = 0;
  int adj_alloc = 0, off_alloc = 0;
  int adj_size = -1, off_size = -1;
  iMesh_getEntArrAdj( mesh, FACES[4][0], INTERVALS, iBase_REGION,
                      &adj, &adj_alloc, &adj_size,
                      &off, &off_alloc, &off_size,
                      &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  CHECK( 0 != adj );
  CHECK( 0 != off );
  CHECK_EQUAL( INTERVALS, adj_size ); // one hex adjacent to each skin face
  CHECK_EQUAL( INTERVALS+1, off_size ); // one more than number of input handles
  CHECK( adj_alloc >= adj_size );
  CHECK( off_alloc >= off_size );
  
  for (int i = 0; i < INTERVALS; ++i) {
    CHECK_EQUAL( 1, off[i+1] - off[i] );
    CHECK_EQUAL( HEXES[0][i][0], adj[off[i]] );
  }
  
  free(adj);
  free(off);
}

void test_getEntArrAdj_down()
{
  iMesh_Instance mesh = create_mesh();
  int err;
  
  // get quads adjacent to a edge-row of hexes
  iBase_EntityHandle *adj = 0;
  int *off = 0;
  int adj_alloc = 0, off_alloc = 0;
  int adj_size = -1, off_size = -1;
  iMesh_getEntArrAdj( mesh, HEXES[0][0], INTERVALS, iBase_FACE,
                      &adj, &adj_alloc, &adj_size,
                      &off, &off_alloc, &off_size,
                      &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  CHECK( 0 != adj );
  CHECK( 0 != off );
  CHECK_EQUAL( 2*INTERVALS+2, adj_size ); // corner hexes adj to 3 faces, others adj to 2
  CHECK_EQUAL( INTERVALS+1, off_size ); // one more than number of input handles
  CHECK( adj_alloc >= adj_size );
  CHECK( off_alloc >= off_size );
  
    // first (corner) hex should have three adjacent faces
  CHECK_EQUAL( 3, off[1] - off[0] );
  iBase_EntityHandle exp[3] = { FACES[0][0][0],
                                FACES[3][0][0],
                                FACES[4][0][0] };
  iBase_EntityHandle act[3];
  std::copy( adj + off[0], adj+off[1], act );
  std::sort( exp, exp+3 );
  std::sort( act, act+3 );
  CHECK_ARRAYS_EQUAL( exp, 3, act, 3 );
  
    // last (corner) hex should have three adjacent faces
  CHECK_EQUAL( 3, off[INTERVALS] - off[INTERVALS-1] );
  iBase_EntityHandle exp2[3] = { FACES[0][0][INTERVALS-1],
                                 FACES[3][0][INTERVALS-1],
                                 FACES[5][0][0] };
  std::copy( adj + off[INTERVALS-1], adj+off[INTERVALS], act );
  std::sort( exp2, exp2+3 );
  std::sort( act, act+3 );
  CHECK_ARRAYS_EQUAL( exp2, 3, act, 3 );
  
    // all middle hexes should have two adjacent faces
  // FixME: This loop is never executed (INTERVALS is 2)
  /*
  for (int i = 1; i < INTERVALS-1; ++i) {
    iBase_EntityHandle e1, e2, a1, a2;
    e1 = FACES[0][0][i];
    e2 = FACES[3][0][i];
    if (e1 > e2) std::swap(e1,e2);
    
    CHECK_EQUAL( 2, off[i+1] - off[i] );
    a1 = adj[off[i]  ];
    a2 = adj[off[i]+1];
    if (a1 > a2) std::swap(a1,a2);
    
    CHECK_EQUAL( e1, a1 );
    CHECK_EQUAL( e2, a2 );
  }
  */
  
  free(adj);
  free(off);
}

void test_getEntArrAdj_invalid_size()
{
  iMesh_Instance mesh = create_mesh();
  int err = -1;
  
  const int SPECIAL1 = 0xDeadBeef;
  const int SPECIAL2 = 0xCafe5;
  const int SPECIAL3 = 0xbabb1e;
  
    // test a downward query
  volatile int marker1 = SPECIAL1;
  iBase_EntityHandle adj1[8*INTERVALS-1]; // one too small
  volatile int marker2 = SPECIAL2;
  int off1[INTERVALS+1];
  int adj1_alloc = sizeof(adj1)/sizeof(adj1[0]);
  int off1_alloc = sizeof(off1)/sizeof(off1[0]);
  int adj_size, off_size;
  iBase_EntityHandle* adj_ptr = adj1;
  int* off_ptr = off1;
  iMesh_getEntArrAdj( mesh, HEXES[0][0], INTERVALS, iBase_VERTEX,
                      &adj_ptr, &adj1_alloc, &adj_size, 
                      &off_ptr, &off1_alloc, &off_size,
                      &err );
  CHECK_EQUAL( &adj1[0], adj_ptr );
  CHECK_EQUAL( &off1[0], off_ptr );
    // first ensure no stack corruption from writing off end of array
  CHECK_EQUAL( SPECIAL1, marker1 );
  CHECK_EQUAL( SPECIAL2, marker2 );
    // now verify that it correctly failed
  CHECK_EQUAL( iBase_BAD_ARRAY_SIZE, err );
  
    // now test an upwards query
  volatile int marker3 = SPECIAL3;
  int off2[INTERVALS];
  volatile int marker4 = SPECIAL1;
  int off2_alloc = sizeof(off2)/sizeof(off2[0]);
  err = iBase_SUCCESS;
  adj_ptr = adj1;
  off_ptr = off2;
  iMesh_getEntArrAdj( mesh, VERTS[0][0], INTERVALS+1, iBase_REGION,
                      &adj_ptr, &adj1_alloc, &adj_size, 
                      &off_ptr, &off2_alloc, &off_size,
                      &err );
    // first ensure no stack corruption from writing off end of array
  CHECK_EQUAL( &adj1[0], adj_ptr );
  CHECK_EQUAL( &off2[0], off_ptr );
  CHECK_EQUAL( SPECIAL3, marker3 );
  CHECK_EQUAL( SPECIAL1, marker4 );
    // now verify that it correctly failed
  CHECK_EQUAL( iBase_BAD_ARRAY_SIZE, err );
}

void test_getEntArrAdj_none()
{
  iMesh_Instance mesh = create_mesh();
  int err = -1;

  iBase_EntityHandle* adj = 0;
  int* off = 0;
  int adj_alloc = 0, off_alloc = 0;
  int adj_size = -1, off_size = -1;
  iMesh_getEntArrAdj( mesh, NULL, 0, iBase_REGION, 
                      &adj, &adj_alloc, &adj_size,
                      &off, &off_alloc, &off_size,
                      &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  CHECK_EQUAL( 0, adj_alloc );
  CHECK_EQUAL( 0, adj_size );
  CHECK_EQUAL( 1, off_size );
  CHECK( off_alloc >= 1 );
  CHECK_EQUAL( 0, off[0] );  
  
  free(off);
}

void test_existinterface()
{
    // test construction of an imesh instance from a core instance
  moab::Core *core = new moab::Core();
  MBiMesh *mesh = new MBiMesh(core);
  iMesh_Instance imesh = reinterpret_cast<iMesh_Instance>(mesh);
  
    // make sure we can call imesh functions
  int dim, err;
  iMesh_getGeometricDimension(imesh, &dim, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

    // now make sure we can delete the instance without it deleting the MOAB instance
  iMesh_dtor(imesh, &err);
  CHECK_EQUAL(err, iBase_SUCCESS);
  
  ErrorCode rval = core->get_number_entities_by_dimension(0, 0, dim);
  CHECK_EQUAL(moab::MB_SUCCESS, rval);

    // finally, delete the MOAB instance
  delete core;
}

void test_tags_retrieval()
{
  iMesh_Instance mesh;
  int err;
  iMesh_newMesh("", &mesh, &err, 0);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iBase_EntitySetHandle root_set;
  iMesh_getRootSet(mesh, &root_set, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  // open a file with var len tags (sense tags)
  // they should be filtered out
  std::string filename = STRINGIFY(MESHDIR) "/PB.h5m";

  iMesh_load(mesh, root_set, filename.c_str(), NULL, &err, filename.length(), 0);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iBase_EntitySetHandle* contained_set_handles = NULL;
  int contained_set_handles_allocated = 0;
  int contained_set_handles_size;
  // get all entity sets
  iMesh_getEntSets(mesh, root_set, 1,
      &contained_set_handles,
      &contained_set_handles_allocated,
      &contained_set_handles_size,
      &err  );
  CHECK_EQUAL( iBase_SUCCESS, err );
  // get tags for all sets
  for (int i=0; i< contained_set_handles_size; i++)
  {
    iBase_TagHandle* tag_handles = NULL;
    int tag_handles_allocated=0;
    int tag_handles_size;
    iMesh_getAllEntSetTags (mesh,
        contained_set_handles[i],
        &tag_handles,
        &tag_handles_allocated,
        &tag_handles_size, &err);
    CHECK_EQUAL( iBase_SUCCESS, err );

    for (int j=0; j<tag_handles_size; j++)
    {
      int tagSize;
      iMesh_getTagSizeValues(mesh, tag_handles[j], &tagSize, &err);
      CHECK_EQUAL( iBase_SUCCESS, err );

    }
    free(tag_handles);
  }
  free (contained_set_handles);

  // Delete the iMesh instance
  iMesh_dtor(mesh, &err);
  CHECK_EQUAL(iBase_SUCCESS, err);

  return;
}

void test_invalid_parallel_option()
{
  iMesh_Instance mesh;
  int err;
  iMesh_newMesh("moab:PARALLEL", &mesh, &err, 13);
  CHECK_EQUAL(iBase_NOT_SUPPORTED, err);

  iMesh_dtor(mesh, &err);
  CHECK_EQUAL(iBase_SUCCESS, err);
}
