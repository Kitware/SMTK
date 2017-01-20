#include "TestRunner.hpp"
#include "iMesh.h"
#include "iMesh_extensions.h"
#include "MBiMesh.hpp"
#include "moab/Core.hpp"
#include <algorithm>

void test_tag_iterate();
void test_step_iter();

int main( int argc, char* argv[] )
{
  REGISTER_TEST( test_tag_iterate );
  REGISTER_TEST( test_step_iter );

  return RUN_TESTS( argc, argv ); 
}

void test_tag_iterate()
{
  iMesh_Instance mesh;
  int err;
  iMesh_newMesh("", &mesh, &err, 0);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iBase_EntitySetHandle root_set, entset;
  iMesh_getRootSet(mesh, &root_set, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iBase_EntityHandle *verts = 0;
  int verts_alloc = 0, verts_size = 0;
  double coords[] = { 0,0,0, 1,1,1, 2,2,2, 3,3,3, 4,4,4, 5,5,5 };
  iMesh_createVtxArr(mesh,6,iBase_INTERLEAVED,coords,18,&verts,&verts_alloc,
                     &verts_size,&err);
  CHECK_EQUAL( iBase_SUCCESS, err );

    /* create an entity set with two subranges */
  iMesh_createEntSet( mesh, 0, &entset, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_addEntArrToSet( mesh, verts, 2, entset, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_addEntArrToSet( mesh, &verts[3], 3, entset, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
    
    /* create a dbl tag and set vertices */
  iBase_TagHandle tagh;
  iMesh_createTagWithOptions(mesh, "dum", "moab:TAG_STORAGE_TYPE=DENSE", 1, iBase_DOUBLE, &tagh, &err, 3, 27);
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_setDblArrData(mesh, verts, 6, tagh, coords+3, 6, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

    /* get an iterator over the root set, and check tag iterator for that */
  iBase_EntityArrIterator iter;
  int count, atend;
  double *data;
  iMesh_initEntArrIter( mesh, root_set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES,
                        6, 0, &iter, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_tagIterate(mesh, tagh, iter, &data, &count, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
  if (count != 6) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  
  for (int i = 0; i < 6; i++) {
    if (data[i] != coords[i+3]) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  }
  iMesh_endEntArrIter(mesh, iter, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
  iter = 0; // iMesh_endEntArrIter frees iter
  
    /* get an iterator over the set with two subranges, and check tag iterator for that */
  iMesh_initEntArrIter( mesh, entset, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, 6,
                        0, &iter, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_tagIterate(mesh, tagh, iter, &data, &count, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
  if (count != 2 || data[0] != coords[3] || data[1] != coords[4]) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  iMesh_stepEntArrIter(mesh, iter, 2, &atend, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
    /* shouldn't be at end yet */
  if (atend) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  
  iMesh_tagIterate(mesh, tagh, iter, &data, &count, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
  if (count != 3 || data[0] != coords[6] || data[1] != coords[7]) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  iMesh_stepEntArrIter(mesh, iter, 3, &atend, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
    /* should be at end now */
  if (!atend) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  iMesh_endEntArrIter(mesh, iter, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iMesh_dtor(mesh,&err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  free(verts);
}

void test_step_iter()
{
  iMesh_Instance mesh;
  int err;
  iMesh_newMesh("", &mesh, &err, 0);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iBase_EntitySetHandle root_set;
  iMesh_getRootSet(mesh, &root_set, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iBase_EntityHandle *verts = 0;
  int verts_alloc = 0, verts_size = 0;
  double coords[] = { 0,0,0, 1,1,1, 2,2,2, 3,3,3, 4,4,4, 5,5,5 };
  iMesh_createVtxArr(mesh,6,iBase_INTERLEAVED,coords,18,&verts,&verts_alloc,
                     &verts_size,&err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  /* make a non-array iterator and test stepping over it */
  iBase_EntityIterator iter;
  int atend;
  iMesh_initEntIter( mesh, root_set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES,
                     0, &iter, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_stepEntIter(mesh, iter, 2, &atend, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
    /* shouldn't be at end yet */
  if (atend) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  
  iMesh_stepEntIter(mesh, iter, 4, &atend, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
    /* should be at end now */
  if (!atend) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  iMesh_endEntIter(mesh, iter, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  /* make an array iterator and test stepping over it */
  iBase_EntityArrIterator arr_iter;
  iMesh_initEntArrIter( mesh, root_set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES,
                        6, 0, &arr_iter, &err );
  CHECK_EQUAL( iBase_SUCCESS, err );
  iMesh_stepEntArrIter(mesh, arr_iter, 2, &atend, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
    /* shouldn't be at end yet */
  if (atend) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  
  iMesh_stepEntArrIter(mesh, arr_iter, 4, &atend, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );
    /* should be at end now */
  if (!atend) CHECK_EQUAL( iBase_SUCCESS, iBase_FAILURE );
  iMesh_endEntArrIter(mesh, arr_iter, &err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  iMesh_dtor(mesh,&err);
  CHECK_EQUAL( iBase_SUCCESS, err );

  free(verts);
}
