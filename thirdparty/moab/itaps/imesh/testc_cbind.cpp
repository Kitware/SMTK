/**
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2004 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

/** TSTT Mesh Interface Unit Test
 * 
 * This program tests TSTT mesh interface functions through the SIDL interface.
 * In a nutshell, the test creates (or accesses) an implementation instance,
 * tells it to load a file (specified on the command line), then calls a
 * number of functions evaluating that mesh.  This test also tests mesh, set
 * and tag creation and evaluation functions.
 *
 * Usage: testcxx <mesh_file_name>
 *
 * Compiling
 * ---------
 * 1. Build your server (your implementation of the TSTT mesh interface) under 
 *    SIDL/Babel and put it in a library in the Babel-generated server directory, 
 *    ${SERVER_DIR}/libimpl.so.  Any include files needed to instantiate this 
 *    server from applications should also be in that directory.  See note a) below.
 * 2. Change the definition of IMPLEMENTATION_CLASS below to be the 
 *    namespace-qualified name of your implementation class.  This definition is
 *    used in the main to instantiate the server using the SIDL _create function, e.g.
 *    .   TSTTB::Mesh mesh = IMPLEMENTATION_CLASS._create();
 *    (we put the definition of IMPLEMENTATION_CLASS at the top of this file so that
 *    you don't have to understand the rest of the source code to use this test).  See
 *    note b) below.
 * 3. Include the file(s) needed which declare the implementation's namespace and
 *    class
 * 4. Compile this test file to an object file:
 *        g++ -fpic -I. -I${BABEL}/include -I${SERVER_DIR} -c testcxx.cpp
 * 5. Link the application:
 *        g++ -o testcxx testcxx.o -ltest -L${BABEL_LIBS} -lsidl
 * 
 * Notes 
 * -----
 * a) The files don't absolutely need to be arranged as described above.  For
 *    example, some projects put their custom implementation files somewhere besides
 *    ${SERVER_DIR}, to keep them separate from Babel-generated files.
 * b) This test assumes interface instances are created using a direct call to the
 *    SIDL _create() function for that class.  Another way to do this would be to
 *    call a factory linked into the test.  To change the method used to construct
 *    the interface instance for this test, see the use of IMPLEMENTATION_CLASS towards
 *    the end of this file.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iMesh.h"
#include "iMesh_extensions.h"
#include "moab/Types.hpp"
using namespace moab;
extern enum iBase_ErrorType iBase_ERROR_MAP[MB_FAILURE + 1];


#define FALSE 0
#define TRUE 1

#define DEFAULT_TEST_FILE brick.vtk

#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
#ifdef SRCDIR
#  define DEFAULT_INPUT_FILE STRINGIFY(SRCDIR/DEFAULT_TEST_FILE)
#else
#  define DEFAULT_INPUT_FILE STRINGIFY(DEFAULT_TEST_FILE)
#endif

static bool assert_pass;
#define ASSERT(COND) \
  assert_pass = true; \
  if (!(COND)) { \
    PRINT_ASSERT_FAILURE(#COND, __FILE__, __LINE__); \
    assert_pass = false; \
  }

void PRINT_ASSERT_FAILURE(const char* cond, const char* file, int line)
{
  printf("Condition: %s\n", cond );
  printf(" failed at %s line %d\n", file, line );
}

#define CHK(err) if (err != iBase_SUCCESS) \
  do {printf("%s:%d ITAPS error %d\n", __FILE__, __LINE__, err); return 0;} while (0)

static iBase_EntitySetHandle root_set;

/*!
  prints out a result string based on the value of error_code
*/
void handle_error_code(const int result,
                       int *number_failed,
                       int * /*number_not_implemented*/,
                       int *number_successful)
{
  if (result) {
    printf("Success\n");
    (*number_successful)++;
  }
  else {
    printf("Failure\n");
    (*number_failed)++;
  }
}

/*!
  @test 
  Load Mesh
  @li Load a mesh file
*/
int load_mesh_test(const char *filename, iMesh_Instance mesh)
{
    /* load a mesh */
  int result;
  iMesh_load(mesh, root_set, filename, NULL, &result, strlen(filename), 0);
  if (iBase_SUCCESS != result) {
    printf("ERROR : can not load a mesh from file %s\n", filename);
    return FALSE;
  }

  return TRUE;
}

#define TEST_ERROR_CODE( A, B ) \
  if (iBase_ERROR_MAP[(A)] != (B)) { \
    printf("ERROR: Invalid mapping for MOAB error code %s\n", #A); \
    printf("       Expected %d, actual is %d\n", (int)iBase_ERROR_MAP[(A)], (int)(B) ); \
    return FALSE; \
  }

int error_code_test(iMesh_Instance /*mesh*/) {
  TEST_ERROR_CODE( MB_SUCCESS,                  iBase_SUCCESS )
  TEST_ERROR_CODE( MB_TYPE_OUT_OF_RANGE,        iBase_INVALID_ENTITY_TYPE )
  TEST_ERROR_CODE( MB_MEMORY_ALLOCATION_FAILED, iBase_MEMORY_ALLOCATION_FAILED )
  TEST_ERROR_CODE( MB_ENTITY_NOT_FOUND,         iBase_INVALID_ENTITY_HANDLE )
  TEST_ERROR_CODE( MB_TAG_NOT_FOUND,            iBase_TAG_NOT_FOUND )
  TEST_ERROR_CODE( MB_FILE_DOES_NOT_EXIST,      iBase_FILE_NOT_FOUND )
  TEST_ERROR_CODE( MB_UNSUPPORTED_OPERATION,    iBase_NOT_SUPPORTED )
  TEST_ERROR_CODE( MB_UNHANDLED_OPTION,         iBase_INVALID_ARGUMENT )
  TEST_ERROR_CODE( MB_FAILURE,                  iBase_FAILURE )
  return TRUE;
}

/*!
  @test
  TSTT topology dimension Test
  @li Check 2d topology dimensions
*/
int topology_dimension_test(iMesh_Instance mesh)
{
  iBase_EntityHandle *faces = NULL;
  int faces_alloc = 0, faces_size;
  int result, i;
  int *dimensions = NULL;
  int dimensions_alloc = 0, dimensions_size;

    /* first get 2D entities */
  iMesh_getEntities(mesh, root_set, iBase_FACE, 
                    iMesh_ALL_TOPOLOGIES, 
                    &faces, &faces_alloc, &faces_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get faces in entity_sets_test.\n");
    return FALSE;
  }

    /* get dimensions of faces */
  iMesh_getEntArrType(mesh, faces, faces_size, 
                      &dimensions, &dimensions_alloc, &dimensions_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get dimensions of faces in topology_test.\n");
    free(faces);
    return FALSE;
  }

  if (dimensions_size != faces_size) {
    printf("Didn't get the right number of types in topology_test.\n");
    free(faces);
    free(dimensions);
    return FALSE;
  }
    
    /* make sure all elements are 2D */
  for (i = 0; i < faces_size; i++) {
    if (dimensions[i] != iBase_FACE) {
      free(faces);
      free(dimensions);
      return FALSE;
    }
  }

  free(faces);
  free(dimensions);

  return TRUE;
}

/*!
  @test
  TSTT topology adjacency Test
  @li Check topology information
  @li Check adjacency
  @li Get interior and exterior elements
*/
/* make each topological entity vectors, check their topology */
/* types, get interior and exterior faces of hexes */
int topology_adjacency_test(iMesh_Instance mesh)
{
  int result, i, j, entities_alloc, entities_size, *topologies;
  int topologies_alloc, topologies_size;
  iBase_EntityHandle *entities, *entity_vectors[iMesh_ALL_TOPOLOGIES] = {NULL};
  int entity_vectors_sizes[iMesh_ALL_TOPOLOGIES] = {0};
  int top_j, num_tops, region_type, num_region;
  iBase_EntityHandle *adj_faces = NULL;
  int adj_faces_alloc = 0, adj_faces_size;
  int *face_offsets = NULL;
  int face_offsets_alloc = 0, face_offsets_size, face_loaded;
  iBase_EntityHandle *adj_regions = NULL;
  int adj_regions_alloc = 0, adj_regions_size;
  int *region_offsets = NULL;
  int region_offsets_alloc = 0, region_offsets_size;
  iBase_EntityHandle *interior, *exterior;
  int num_int = 0, num_ext = 0, found, next_offset, iter;
  int num_faces_per_region;;

    /* fill the vectors of each topology entities */
    /* like lines vector, polygon vector, triangle vector, */
    /* quadrilateral, polyhedrron, tet, hex, prism, pyramid, */
    /* septahedron vectors */
  for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++) {
    entities = NULL;
    entities_alloc = 0;
    iMesh_getEntities(mesh, root_set, iBase_ALL_TYPES,
                      i, &entities, &entities_alloc, 
                      &entities_size, &result);
    if (iBase_SUCCESS != result) {
      printf("Failed to get entities in adjacencies_test.\n");
      return FALSE;
    }

    if (entities_alloc > 0) {
      topologies = NULL;
      topologies_alloc = 0;
      iMesh_getEntArrTopo(mesh, entities, 
                          entities_alloc, 
                          &topologies, &topologies_alloc, &topologies_size, &result);
      if (iBase_SUCCESS != result) {
        printf("Failed to get topologies in adjacencies_test.\n");
        return FALSE;
      }  

      if (topologies_size != entities_size) {
        printf("Didn't get the right number of topologies "
               "in topology_adjacency_test.\n");
        free(topologies);
        free(entities);
        return FALSE;
      }
    
        /* put entities into vectors of each topology */
      entity_vectors[i] = (iBase_EntityHandle*) 
        malloc(entities_size*sizeof(iBase_EntityHandle));

      for (j = 0; j < entities_size; j++) {
        if (topologies[j] < iMesh_POINT ||
            topologies[j] >= iMesh_ALL_TOPOLOGIES)
          printf("Didn't find entity type for this topology.");
        else {
          entity_vectors[i][entity_vectors_sizes[i]] = entities[j];
          entity_vectors_sizes[i]++;
        }
      }

      free(topologies);
    }

    free(entities);
  }

    /* check number of entities for each topology */
  for (top_j = 0; top_j < iMesh_ALL_TOPOLOGIES; top_j++) {
    num_tops = 0;
    iMesh_getNumOfTopo(mesh, root_set, top_j, &num_tops, &result);
    if (iBase_SUCCESS != result) {
      printf("Failed to get number of topologies in adjacencies_test.\n");
      for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
        free(entity_vectors[i]);
      return FALSE;
    }
    
    if (entity_vectors_sizes[top_j] != num_tops) {
      printf("Topology count mismatch.\n");
      for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
        free(entity_vectors[i]);
      return FALSE;
    }
  }

    /* change if 3d topology entities are added or removed */
  for (region_type = iMesh_TETRAHEDRON;
       region_type < iMesh_ALL_TOPOLOGIES; region_type++) {
      /* get all adjacent faces of regions  */
    iBase_EntityHandle *region_vector =
      entity_vectors[region_type];
    
    num_region = entity_vectors_sizes[region_type];
    
    if (num_region > 0) {
      adj_faces = NULL;
      adj_faces_alloc = 0;
      face_offsets = NULL;
      face_offsets_alloc = 0;
      
      iMesh_getEntArrAdj(mesh, region_vector, num_region, 
                         iBase_FACE,
                         &adj_faces, &adj_faces_alloc, &adj_faces_size, 
                         &face_offsets, &face_offsets_alloc, 
                         &face_offsets_size, &result);
      if (iBase_SUCCESS != result) {
        printf("Failed to get adjacent faces of regions in adjacencies_test.\n");
        for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
          free(entity_vectors[i]);
        return FALSE;
      }

      if (num_region + 1 != face_offsets_size) {
        printf("Number of offsets didn't agree with number of "
               "regions in topology_adjacency_test.\n");
        free(face_offsets);
        free(adj_faces);
        for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
          free(entity_vectors[i]);
        return FALSE;
      }

      face_loaded = FALSE;

        /* check # of faces really loaded */
      if (region_type == iMesh_TETRAHEDRON) {
        if (adj_faces_size == 4 * num_region)
          face_loaded = TRUE;
      }
      else if (region_type == iMesh_HEXAHEDRON) {
        if (adj_faces_size == 6 * num_region)
          face_loaded = TRUE;
      }
      else if (region_type == iMesh_PRISM) {
        if (adj_faces_size == 5 * num_region)
          face_loaded = TRUE;
      }
      else if (region_type == iMesh_PYRAMID) {
        if (adj_faces_size == 5 * num_region)
          face_loaded = TRUE;
      }
      else if (region_type == iMesh_SEPTAHEDRON) {
        if (adj_faces_size == 7 * num_region)
          face_loaded = TRUE;
      }
      else
        face_loaded = FALSE;

        /* get all adjacent regions of adjacent faces */
      adj_regions = NULL;
      adj_regions_alloc = 0;
      region_offsets = NULL;
      region_offsets_alloc = 0;
      iMesh_getEntArrAdj(mesh, adj_faces, adj_faces_size, iBase_REGION,
                         &adj_regions, &adj_regions_alloc, 
                         &adj_regions_size,
                         &region_offsets, &region_offsets_alloc,
                         &region_offsets_size, &result);
      if (iBase_SUCCESS != result) {
        printf("Failed to get regions from faces in adjacencies_test.\n");
        free(face_offsets);
        free(adj_faces);
        for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
          free(entity_vectors[i]);
        return FALSE;
      }

      if (adj_faces_size + 1 != region_offsets_size) {
        printf("Number of offsets didn't agree with number of faces in topology_adjacency_test.\n");
        free(face_offsets);
        free(region_offsets);
        free(adj_faces);
        free(adj_regions);
        for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
          free(entity_vectors[i]);
        return FALSE;
      }

      interior = (iBase_EntityHandle*)malloc(adj_faces_size*sizeof(iBase_EntityHandle)),
      exterior = (iBase_EntityHandle*)malloc(adj_faces_size*sizeof(iBase_EntityHandle));
      num_int = 0;
      num_ext = 0;

        /* find the interior faces having two adjacent regions */
      for (i = 0; i < adj_faces_size; i++) {
        next_offset = 0;
        if (i == adj_faces_size-1) next_offset = adj_regions_size;
        else next_offset = region_offsets[i + 1];

        if (next_offset - region_offsets[i] == 2) {
          found = FALSE;
          for (iter = 0; iter < num_int; iter++) {
            if (interior[iter] == adj_faces[i]) {
              found = TRUE;
              break;
            }
          }
          if (!found)
            interior[num_int++] = adj_faces[i];
        }
      }

        /* now remove any interior faces from the previous adjacent faces list */
        /* and we should be left with exterior faces */
      for (i = 0; i < adj_faces_size; i++) {
        found = FALSE;
        for (iter = 0; iter < num_int; iter++) {
          if (interior[iter] == adj_faces[i]) {
            found = TRUE;
            break;
          }
        }
        if (!found)
          exterior[num_ext++] = adj_faces[i];
      }

      num_faces_per_region = face_offsets[1] - face_offsets[0];

        /* check # of exterior and interior faces */
        /* should be #exterior_faces + 2 * #interior_faces = #faces_per_region */
        /* * #regions */
      if (face_loaded) {
        if (num_ext + 2*num_int !=
            num_faces_per_region*num_region) {
          printf("exterior/interior failure: %d ext, %d int, %d regions, %d faces per\n",
                 num_ext, num_int, num_region, num_faces_per_region);
          free(face_offsets);
          free(region_offsets);
          free(adj_faces);
          free(adj_regions);
          free(interior);
          free(exterior);
          for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
            free(entity_vectors[i]);
          return FALSE;
        }
      }

      free(face_offsets);
      free(region_offsets);
      free(adj_faces);
      free(adj_regions);
      free(interior);
      free(exterior);
    }
  }

  for (i = 0; i < iMesh_ALL_TOPOLOGIES; i++)
    free(entity_vectors[i]);

  return TRUE;
}

int qsort_comp_handles( const void* h1, const void* h2 )
{
  return *(char**)h1 - *(char**)h2;
}

/*!
  @test
  TSTT EntityType Connectivity Test
  @li Get coordinates for all type enities
*/

int entity_connectivity_test(iMesh_Instance mesh)
{
  int type, result;
  int *offsets, offsets_alloc, offsets_size;
  int *indices, indices_alloc, indices_size;
  iBase_EntityHandle *entities, *adj_ents, *entities2, *sorted;
  int entities_alloc, entities_size, adj_ents_alloc, adj_ents_size;
  int entities2_alloc, entities2_size;
  iBase_EntityHandle adj_ents2[27], *adj_ents2_ptr = adj_ents2;
  int adj_ents2_alloc = 27, adj_ents2_size, i, size;

  for (type = iBase_EDGE; type < iBase_ALL_TYPES; type++) {
    entities = NULL; entities_alloc = 0;
    adj_ents = NULL; adj_ents_alloc = 0;
    indices = NULL; indices_alloc = 0;
    offsets = NULL; offsets_alloc = 0;
    iMesh_getAdjEntIndices( mesh, root_set, 
                            type, iMesh_ALL_TOPOLOGIES, iBase_VERTEX,
                            &entities, &entities_alloc, &entities_size,
                            &adj_ents, &adj_ents_alloc, &adj_ents_size,
                            &indices, &indices_alloc, &indices_size,
                            &offsets, &offsets_alloc, &offsets_size,
                            &result );
    if (iBase_SUCCESS != result) {
      printf("Failed to get indices of vertices in connectivity_test, type=%d.\n", type);
      return FALSE;
    }

    if (entities_alloc != entities_size) {
      printf("Number of entities didn't agree with array size in connectivity_test.\n");
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      return FALSE;
    }

    if (offsets_alloc != offsets_size) {
      printf("Number of offsets didn't agree with array size in connectivity_test.\n");
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      return FALSE;
    }

    if (indices_alloc != indices_size) {
      printf("Number of indices didn't agree with array size in connectivity_test.\n");
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      return FALSE;
    }

    if (adj_ents_alloc != adj_ents_size) {
      printf("Number of adjacent entities didn't agree with array size in connectivity_test.\n");
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      return FALSE;
    }
    
    if (offsets_size != entities_size + 1) {
      printf("Invalid/inconsistent offset size from iMesh_getAdjEntIndices.\n");
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      return FALSE;
    }

      /* check that results are valid */
    for (i = 0; i < entities_size; ++i) {
      ASSERT( offsets[i] < offsets[i + 1] && offsets[i] < indices_size );
      if (!assert_pass) {
        free(entities);
        free(adj_ents);
        free(indices);
        free(offsets);
        return FALSE;
      }
    }

    for (i = 0; i < indices_size; ++i) {
      ASSERT( indices[i] >= 0 && indices[i] < adj_ents_size );
      if (!assert_pass) {
        free(entities);
        free(adj_ents);
        free(indices);
        free(offsets);
        return FALSE;
      }
    }
    
      /* compare initial entity list against result of iMesh_getEntities */
    entities2 = NULL; entities2_alloc = 0;
    iMesh_getEntities( mesh, root_set, 
                       type, iMesh_ALL_TOPOLOGIES,
                       &entities2, &entities2_alloc, &entities2_size,
                       &result );
    ASSERT( iBase_SUCCESS == result );
    if (!assert_pass) {
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      return FALSE;
    }

    size = sizeof(iBase_EntityHandle)*entities_size;
    sorted = (iBase_EntityHandle*)malloc(size);
    memcpy( sorted, entities, size );
    qsort( sorted, entities_size, sizeof(iBase_EntityHandle), &qsort_comp_handles );
    qsort( entities2, entities2_size, sizeof(iBase_EntityHandle), &qsort_comp_handles );

    ASSERT( entities_size == entities2_size && !memcmp(sorted, entities2, size) );
    if (!assert_pass) {
      free(entities);
      free(adj_ents);
      free(indices);
      free(offsets);
      free(entities2);
      free(sorted);
      return FALSE;
    }

    free(entities2);
    free(sorted);

      /* compare results against output of iMesh_getEntAdj */
    for (i = 0; i < entities_size; ++i) {
      iMesh_getEntAdj( mesh, entities[i], iBase_VERTEX,
                       &adj_ents2_ptr, &adj_ents2_alloc, &adj_ents2_size,
                       &result );
      ASSERT( iBase_SUCCESS == result );
      if (!assert_pass) {
        free(entities);
        free(adj_ents);
        free(indices);
        free(offsets);
        return FALSE;
      }

      ASSERT( adj_ents2_ptr == adj_ents2 ); /* shouldn't change */
      if (!assert_pass) {
        free(entities);
        free(adj_ents);
        free(indices);
        free(offsets);
        return FALSE;
      }

      ASSERT( adj_ents2_alloc == 27 ); /* shouldn't change */
      if (!assert_pass) {
        free(entities);
        free(adj_ents);
        free(indices);
        free(offsets);
        return FALSE;
      }

      /* compare results */
      size = offsets[i + 1] - offsets[i];
      ASSERT( size == adj_ents2_size );
      while (--size >= 0) {
        ASSERT( adj_ents2[size] == adj_ents[indices[offsets[i] + size]] );
        if (!assert_pass) {
          free(entities);
          free(adj_ents);
          free(indices);
          free(offsets);
          return FALSE;
        }
      }
    }

    free(entities);
    free(adj_ents);
    free(indices);
    free(offsets);

/*
    offsets = NULL;
    offsets_alloc = 0;
    entities = NULL;
    entities_alloc = 0;

    iMesh_getAdjEntities(mesh, root_set, type, 
                         iMesh_ALL_TOPOLOGIES, iBase_VERTEX, 
                         &entities, &entities_alloc, &entities_size,
                         &offsets, &offsets_alloc, &offsets_size,
                         &result);
    if (iBase_SUCCESS != result) {
      printf("Failed to get indices of adjacent entity vertices in connectivity_test.\n");
      return FALSE;
    }

    if (entities_alloc != entities_size ||
        offsets_alloc != offsets_size) {
      printf("Number of elements didn't agree with array size for an array in connectivity_test.\n");
      return FALSE;
    }

    free(offsets);
    free(entities);
*/
  }

  return TRUE;
}

/*!
  @test
  TSTT entity sets sub test
  @li Check entity sets
*/

/* helper function used to report errors in # sets */
int check_esets(iMesh_Instance mesh, const int num_sets);

int entity_sets_subtest(iMesh_Instance mesh, int is_list,
                         int /*num_iter*/)
{
  int i, num_type = iBase_ALL_TYPES - iBase_VERTEX;
  //int num_all_entities_super = 0;
  iBase_EntitySetHandle es_array[iBase_ALL_TYPES - iBase_VERTEX];
  int number_array[iBase_ALL_TYPES - iBase_VERTEX];
  int ent_type = iBase_VERTEX;
  iBase_EntityHandle *entities = NULL;
  int entities_alloc = 0, entities_size;
  iBase_EntitySetHandle parent_child, super_set = NULL;
  iBase_EntitySetHandle temp_es1, temp_es2, temp_es3;
  iBase_EntityHandle *edges = NULL, *faces = NULL, 
    *temp_entities1 = NULL, *temp_entities2 = NULL;
  int edges_alloc = 0, faces_alloc = 0, 
    temp_entities1_alloc = 0, temp_entities2_alloc = 0;
  int edges_size, faces_size, temp_entities1_size, temp_entities2_size;
  int *types = NULL;
  int types_alloc = 0, types_size;
  int num_rest, num_regions;
  iBase_EntityHandle *regions = NULL;
  int regions_alloc = 0, regions_size;
  iBase_EntitySetHandle *parents = NULL;
  int parents_alloc = 0, parents_size, temp_numb, is_child;
  iBase_EntitySetHandle *es_array1 = NULL;
  int es_array1_alloc = 0, es_array1_size, num_super;
  iBase_EntityHandle *all_entities = NULL;
  int all_entities_alloc = 0, all_entities_size, k, l;
  iBase_EntityHandle *adj_faces = NULL;
  int adj_faces_alloc = 0, adj_faces_size;
  int *face_offsets = NULL;
  int face_offsets_alloc = 0, face_offsets_size;
  iBase_EntityHandle *hexes = NULL;
  int hexes_alloc = 0, hexes_size;
  iBase_EntitySetHandle hex_set;

    /* get the number of whole mesh */
  int n_whole_mesh = 0;
  int result;
  iMesh_getNumEntSets(mesh, root_set, 1, &n_whole_mesh, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem to get the number of all entity sets in whole mesh.\n");
    return FALSE;
  }

    /* add entities to entitysets by type */
  for (; ent_type < num_type; ent_type++) {
      /* initialize the entityset */
    iMesh_createEntSet(mesh, is_list, &es_array[ent_type], &result);
    if (iBase_SUCCESS != result) {
      printf("Problem creating entityset.\n");
      return FALSE;
    }

      /* get entities by type in total "mesh" */
    entities = NULL;
    entities_alloc = 0;
    iMesh_getEntities(mesh, root_set, ent_type,
                      iMesh_ALL_TOPOLOGIES, 
                      &entities, &entities_alloc, &entities_size, &result);
    if (iBase_SUCCESS != result) {
      printf("Failed to get entities by type in entity_sets_test.\n");
      return FALSE;
    }

    if (entities_alloc != entities_size) {
      printf("Number of entities didn't agree with array size in entity_sets_subtest.\n");
      free(entities);
      return FALSE;
    }

      /* add entities into entity set */
    if (0 != entities_size) {
      iMesh_addEntArrToSet(mesh, entities, entities_size, es_array[ent_type], &result);
      if (iBase_SUCCESS != result) {
        printf("Failed to add entities in entity_sets_test.\n");
        free(entities);
        return FALSE;
      }
    }
    
      /* Check to make sure entity set really has correct number of entities in it */
    iMesh_getNumOfType(mesh, es_array[ent_type], ent_type,
                       number_array + ent_type, &result);
      
    if (iBase_SUCCESS != result) {
      printf("Failed to get number of entities by type in entity_sets_test.\n");
      free(entities);
      return FALSE;
    }  

      /* compare the number of entities by type */
    if (number_array[ent_type] != entities_size)
    {
      printf("Number of entities by type is not correct\n");
      free(entities);
      return FALSE;
    }

      /* add to number of all entities in super set */
    //num_all_entities_super += entities_size;

    free(entities);
  }

  if (!check_esets(mesh, n_whole_mesh + num_type))
    return FALSE;

    /* make a super set having all entitysets */
  super_set = NULL;
  iMesh_createEntSet(mesh, is_list, &super_set, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to create a super set in entity_sets_test.\n");
    return FALSE;
  }

  for (i = 0; i < num_type; i++) {
    iMesh_addEntSet(mesh, es_array[i], super_set, &result);
    if (iBase_SUCCESS != result) {
      printf("Failed to add a set to a super set in entity_sets_test.\n");
      return FALSE;
    }
  }
  
  if (!check_esets(mesh, n_whole_mesh + num_type + 1)) 
    return FALSE;

    /*----------TEST intEAN OPERATIONS----------------*/

  iMesh_createEntSet(mesh, is_list, &temp_es1, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to create a super set in entity_sets_test.\n");
    return FALSE;
  }
  
  if (!check_esets(mesh, n_whole_mesh + num_type + 2)) {
    return FALSE;
  }

    /* Subtract */
    /* add all EDGEs and FACEs to temp_es1 */
    /* get all EDGE entities */
  edges = NULL;
  edges_alloc = 0;
  iMesh_getEntities(mesh, es_array[iBase_EDGE], iBase_EDGE, 
                    iMesh_ALL_TOPOLOGIES, 
                    &edges, &edges_alloc, &edges_size, &result);

  if (iBase_SUCCESS != result) {
    printf("Failed to get edge entities in entity_sets_test.\n");
    return FALSE;
  }

    /* add EDGEs to es1 */
  iMesh_addEntArrToSet(mesh, edges, edges_size, temp_es1, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to add edge entities in entity_sets_test.\n");
    free(edges);
    return FALSE;
  }

    /* get all FACE entities */
  faces = NULL;
  faces_alloc = 0;
  iMesh_getEntities(mesh, es_array[iBase_FACE], iBase_FACE, 
                    iMesh_ALL_TOPOLOGIES, 
                    &faces, &faces_alloc, &faces_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get face entities in entity_sets_test.\n");
    free(edges);
    return FALSE;
  }

    /* add FACEs to es1 */
  iMesh_addEntArrToSet(mesh, faces, faces_size, temp_es1, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to add face entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    return FALSE;
  }

    /* subtract EDGEs */
  
  iMesh_subtract(mesh, temp_es1, es_array[iBase_EDGE], &temp_es2, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to subtract entitysets in entity_sets_test.\n");
    free(edges);
    free(faces);
    return FALSE;
  }

  temp_entities1 = NULL;
  temp_entities1_alloc = 0;
  iMesh_getEntities(mesh, temp_es2, iBase_FACE, iMesh_ALL_TOPOLOGIES, 
                    &temp_entities1, &temp_entities1_alloc, &temp_entities1_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get face entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    return FALSE;
  }

  if (faces_size != temp_entities1_size) {
    printf("not match number of entitysets after subtraction "
           "in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    return FALSE;
  }

    /* check there's nothing but faces in face_es */
  types = NULL;
  types_alloc = 0;
  iMesh_getEntArrType(mesh, temp_entities1, temp_entities1_size,
                      &types, &types_alloc, &types_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get types of entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    return FALSE;
  }
  for (i = 0; i < types_size; i++) {
    if (types[i] != iBase_FACE) {
      printf("wrong entity type for face test in entity_sets_test.\n");
      free(edges);
      free(faces);
      free(temp_entities1);
      free(types);
      return FALSE;
    }
  }

  iMesh_destroyEntSet(mesh, temp_es2, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to destroy temp es2.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }
    
  if (!check_esets(mesh, n_whole_mesh + num_type + 2)) {
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

    /*------------Intersect------------ */

    /* clean out the temp_ms1 */
  iMesh_rmvEntArrFromSet(mesh, faces, faces_size, temp_es1, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to remove face entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

    /* check if it is really cleaned out */
  iMesh_getNumOfType(mesh, temp_es1, iBase_FACE, &num_rest, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get number of entities by type in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

  if (num_rest != 0) {
    printf("failed to remove correctly.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }
  
    /* add EDGEs to temp es1 */
  iMesh_addEntArrToSet(mesh, edges, edges_size, temp_es1, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to add edge entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

    /* add FACEs to temp es1 */
  iMesh_addEntArrToSet(mesh, faces, faces_size, temp_es1, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to add edge entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

    /* intersect temp_es1 with edges meshset  */
    /* temp_ms1 entityset is altered */
  iMesh_intersect(mesh, temp_es1, es_array[iBase_EDGE], &temp_es2, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to intersect in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

  temp_entities2 = NULL;
  temp_entities2_alloc = 0;
  iMesh_getEntities(mesh, temp_es2, iBase_FACE, iMesh_ALL_TOPOLOGIES, 
                    &temp_entities2, &temp_entities2_alloc, 
                    &temp_entities2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get face entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(types);
    return FALSE;
  }

  if (temp_entities2_size != 0) {
    printf("wrong number of faces.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    return FALSE;
  }

  iMesh_destroyEntSet(mesh, temp_es2, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to destroy temp es2.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    return FALSE;
  }
    
  if (!check_esets(mesh, n_whole_mesh + num_type + 2)) {
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    return FALSE;
  }

    /*-------------Unite-------------- */

  iMesh_createEntSet(mesh, is_list, &temp_es2, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to create a temp entityset in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    return FALSE;
  }

  /* get all regions */
  regions = NULL;
  regions_alloc = 0;
  iMesh_getEntities(mesh, es_array[iBase_REGION], 
                    iBase_REGION, 
                    iMesh_ALL_TOPOLOGIES, 
                    &regions, &regions_alloc, &regions_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get region entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    return FALSE;
  }

    /* add REGIONs to temp es2 */
  iMesh_addEntArrToSet(mesh, regions, regions_size, temp_es2, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to add region entities in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

    /* unite temp_es1 and temp_es2 */
  iMesh_unite(mesh, temp_es1, temp_es2, &temp_es3, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to unite in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

    /* perform the check */
  iMesh_getNumOfType(mesh, temp_es3, iBase_REGION, &num_regions, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get number of region entities by type in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }
  
  if (num_regions != number_array[iBase_REGION]) {
    printf("different number of regions in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

  if (!check_esets(mesh, n_whole_mesh + num_type + 4)) {
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

    /*--------Test parent/child stuff in entiysets----------- */

    /* Add 2 meshsets as children to another */
  iMesh_createEntSet(mesh, is_list, &parent_child, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem creating entityset in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

  iMesh_addPrntChld(mesh, es_array[iBase_VERTEX], parent_child, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem add parent in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

    /* check if parent is really added */
  parents = NULL;
  parents_alloc = 0;
  iMesh_getPrnts(mesh, parent_child, 0, 
                 &parents, &parents_alloc, &parents_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem getting parents in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    return FALSE;
  }

  if (parents_size != 1) {
    printf("number of parents is not correct in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }

    /* get the number of child entitysets */
  iMesh_getNumChld(mesh, es_array[iBase_VERTEX], 0, &temp_numb, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem getting number of children in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }

  if (temp_numb != 1) {
    printf("number of children is not correct in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }

    /* parent_child and es_array[iBase_VERTEX] should be related */
  is_child = 0;
  iMesh_isChildOf(mesh, es_array[iBase_VERTEX], parent_child, &is_child, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem checking relation in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }
  if (!is_child) {
    printf("parent_child and es_array[iBase_VERTEX] should be related\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }
    
    /* es_array[iBase_FACE] and es_array[iBase_REGION] are not related */
  is_child = FALSE;
  iMesh_isChildOf(mesh, es_array[iBase_FACE], es_array[iBase_REGION], &is_child, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem checking relation in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }
  if (is_child) {
    printf("es_array[iBase_REGION] and es_array[iBase_FACE] should not be related\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }
  
  if (!check_esets(mesh, n_whole_mesh + num_type + 5)) {
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }

    /*--------test modify and query functions----------------------------- */
  
    /* get all entity sets in super set */
  es_array1 = NULL;
  es_array1_alloc = 0;
  iMesh_getEntSets(mesh, super_set, 0, 
                   &es_array1, &es_array1_alloc, &es_array1_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem to get entity sets in super set.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    return FALSE;
  }

    /* get the number of entity sets in super set */
  iMesh_getNumEntSets(mesh, super_set, 0, &num_super, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem to get the number of all entity sets in super set.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    free(es_array1);
    return FALSE;
  }

    /* the number of entity sets in super set should be same */
  if (num_super != es_array1_size) {
    printf("the number of entity sets in super set should be same.\n");
    return FALSE;
  }

    /* get all entities in super set */
  all_entities = NULL;
  all_entities_alloc = 0;
  iMesh_getEntities(mesh, super_set, iBase_ALL_TYPES,
                    iMesh_ALL_TOPOLOGIES, 
                    &all_entities, &all_entities_alloc, &all_entities_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem to get all entities in super set.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    free(es_array1);
    return FALSE;
  }
  
    /* compare the number of all entities in super set */
    /* NOTE: TEST COMMENTED OUT UNTIL RESOLUTION OF WHETHER GETENTITIES */
    /* SHOULD GET A NUM_HOPS ARGUMENT */
    /*  if (num_all_entities_super != all_entities_size) { */
    /*    printf("number of all entities in super set should be same.\n"); */
    /*    return FALSE; */
    /*  } */

    /* test add, remove and get all entity sets using super set */
    /* check GetAllEntitySets works recursively and dosen't return */
    /* multi sets */
  for (k = 0; k < num_super; k++) {
      /* add entity sets of super set to each entity set of super set */
      /* make multiple child super sets */
    iBase_EntitySetHandle es_k = es_array1[k];
    for (l = 0; l < es_array1_size; l++) {
      iMesh_addEntSet(mesh, es_array1[l], es_k, &result);
      if (iBase_SUCCESS != result) {
        printf("Problem to add entity set to entityset.\n");
        free(edges);
        free(faces);
        free(temp_entities1);
        free(temp_entities2);
        free(types);
        free(regions);
        free(parents);
        free(es_array1);
        free(all_entities);
        return FALSE;
      }
    }

      /* add super set to each entity set */
    iMesh_addEntSet(mesh, super_set, es_k, &result);
    if (iBase_SUCCESS != result) {
      printf("Problem to add super set to entitysets.\n");
      free(edges);
      free(faces);
      free(temp_entities1);
      free(temp_entities2);
      free(types);
      free(regions);
      free(parents);
      free(es_array1);
      free(all_entities);
      return FALSE;
    }

      /* add one entity sets multiple times */
    for (l = 0; l < 3; l++) {
      iMesh_addEntSet(mesh, temp_es1, es_k, &result);
      if (iBase_SUCCESS != result) {
        printf("Problem to add temp set to entitysets.\n");
        free(edges);
        free(faces);
        free(temp_entities1);
        free(temp_entities2);
        free(types);
        free(regions);
        free(parents);
        free(es_array1);
        free(all_entities);
        return FALSE;
      }
    }
  }

    /* get all hexes and get faces of that hexes */
  hexes = NULL;
  hexes_alloc = 0;

  iMesh_getEntities(mesh, root_set, iBase_ALL_TYPES,
                    iMesh_HEXAHEDRON, &hexes, &hexes_alloc, &hexes_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get hexes in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    free(es_array1);
    free(all_entities);
    return FALSE;
  }

    /* get adjacent face of hexes */
  adj_faces = NULL;
  adj_faces_alloc = 0;
  face_offsets = NULL; 
  face_offsets_alloc = 0; 

  iMesh_getEntArrAdj( mesh, hexes, hexes_size, iBase_FACE,
                      &adj_faces, &adj_faces_alloc, &adj_faces_size,
                      &face_offsets, &face_offsets_alloc, &face_offsets_size,
                      &result);
  if (iBase_SUCCESS != result) {
    printf("Problem to get adjacent entities in entitysets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    free(es_array1);
    free(all_entities);
    free(hexes);
    return FALSE;
  }

  iMesh_createEntSet(mesh, FALSE, &hex_set, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem creating entityset in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    free(es_array1);
    free(all_entities);
    free(hexes);
    free(adj_faces);
    free(face_offsets);
    return FALSE;
  }
  
  iMesh_addEntArrToSet(mesh, hexes, hexes_size, hex_set, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to add hexes in entity_sets_test.\n");
    free(edges);
    free(faces);
    free(temp_entities1);
    free(temp_entities2);
    free(types);
    free(regions);
    free(parents);
    free(es_array1);
    free(all_entities);
    free(hexes);
    free(adj_faces);
    free(face_offsets);
    return FALSE;
  }
  
  free(edges);
  free(faces);
  free(temp_entities1);
  free(temp_entities2);
  free(types);
  free(regions);
  free(parents);
  free(es_array1);
  free(all_entities);
  free(hexes);
  free(adj_faces);
  free(face_offsets);

  return TRUE;
}

int check_esets(iMesh_Instance mesh, const int num_sets) 
{
  int entity_sets_size;

  int result;
  iMesh_getNumEntSets(mesh, root_set, 1, &entity_sets_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Problem to get all entity sets in mesh.\n");
    return FALSE;
  }
  if (entity_sets_size != num_sets) {
    printf("the number of entity sets in whole mesh should be %d"
           ", actual number is %d.\n", num_sets, entity_sets_size);
    return FALSE;
  }

  return TRUE;
}

/*!
  @test
  TSTT entity sets Test
  @li Check entity sets
*/
int entity_sets_test(iMesh_Instance mesh)
{
  int iter_num = 0, result;
    /* check  */
  int i;
  for (i = 0; i < 2; i++) {
    iter_num++;

    result = entity_sets_subtest(mesh, i, iter_num);
    if (!result)
      return result;
  }
  
  return TRUE;
}

/*!
  @test 
  Vertex Coordinates
  @li Get coordinates of vertices by 2 different methods then compare them
*/
int vertex_coordinates_test(iMesh_Instance mesh)
{
  iBase_EntityHandle *verts = NULL;
  int verts_alloc = 0, verts_size;
  double *vert_coords = NULL;
  int vert_coords_alloc = 0, vert_coords_size;

    /* check storage order */
  int result;
  int this_order;
  iMesh_getDfltStorage(mesh, &this_order, &result);
  if (iBase_SUCCESS != result) {
    printf("failed to get preferred storage order in vertex_coordinates_test.\n");
    return FALSE;
  }

    /* now get the vertex coordinates from a vertex array */
    /* need to get the vertices in the model */
  verts = NULL;
  verts_alloc = 0;
  iMesh_getEntities(mesh, root_set, iBase_VERTEX, 
                    iMesh_POINT, &verts, &verts_alloc, &verts_size, &result);
  if (iBase_SUCCESS != result) {
    printf("failed to get entities in vertex_coordinates_test.\n");
    return FALSE;
  }

    /* get the coordinates in one array */
  vert_coords = NULL;
  vert_coords_alloc = 0;

  iMesh_getVtxArrCoords(mesh, verts, verts_size, iBase_INTERLEAVED, 
                        &vert_coords, &vert_coords_alloc, &vert_coords_size, &result);
  if (iBase_SUCCESS != result) {
    printf("failed to get vertex cooridinate of entities in vertex_coordinates_test.\n");
    free(verts);
    return FALSE;
  }

  free(verts);
  free(vert_coords);
  
    /* if we get here, this test was successful */
  return TRUE;
}

/*!
  @test
  TSTT Tag Info test
  @li Tests tagCreate, tagDelete, tagGetName, tagGetSize, tagGetHandle
*/
int tag_info_test(iMesh_Instance mesh)
{
  char dum_name[120];
  int dum_name_size = 120;
  int dum_size;
  int error = FALSE;
  iBase_TagHandle dum_handle;
    /* Create a tag */
  int result;
  iBase_TagHandle tag_handle = NULL;
  const char *tag_name = "int_tag";
  int tag_name_size = 8;
  iMesh_createTag(mesh, tag_name, 1, iBase_INTEGER, 
                  &tag_handle, &result, tag_name_size);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag int_tag in vertex_tag_test.");
    return FALSE;
  }

    /* check tag info functions */
  iMesh_getTagName(mesh, tag_handle, dum_name, &result, dum_name_size);
  if (iBase_SUCCESS != result) {
    printf("Couldn't get name of tag just created.\n");
    return FALSE;
  }
  if (strcmp(dum_name, "int_tag")) {
    printf("Tag names didn't match.\n");
    return FALSE;
  }
  
  iMesh_getTagSizeBytes(mesh, tag_handle, &dum_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't get size of tag just created.\n");
    return FALSE;
  }
  if (dum_size != sizeof(int)) {
    printf("Tag sizes didn't match.\n");
    return FALSE;
  }

  iMesh_getTagHandle(mesh, tag_name, &dum_handle, &result, tag_name_size);
  if (iBase_SUCCESS != result) {
    printf("Couldn't get handle of tag just created.\n");
    return FALSE;
  }
  if (dum_handle != tag_handle) {
    printf("Tag handles didn't match.\n");
    return FALSE;
  }

    /* test non-forced version of tagDelete; forced version tested later, */
    /* when we have entities around */
  iMesh_destroyTag(mesh, tag_handle, FALSE, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't delete tag just created.\n");
    return FALSE;
  }

    /* look for that tag, to make sure it got deleted */
  iMesh_getTagHandle(mesh, tag_name, &tag_handle, &result, tag_name_size);
  if (iBase_SUCCESS != result) {
    error = TRUE;
  }
  if (!error) {
    printf("tagGetHandle was able to find handle for deleted tag.\n");
    return FALSE;
  }

  return TRUE;
}

int vertex_int_tag_test(iMesh_Instance mesh, 
                         iBase_EntityHandle *verts, int /*verts_size*/,
                         iBase_TagHandle *int_tag) 
{
  int result;
  iBase_EntityHandle dum_vert = verts[0];
  
  int dum_val = 11, dum_val2;
  void *dum_val2_ptr = &dum_val2;
  int dum_val2_alloc = sizeof(int), dum_val2_size;

    /* create a tag */
  const char *tag_name = "int_tag";
  int tag_name_size = 8;
  iMesh_createTag(mesh, tag_name, 1, iBase_INTEGER, 
                  int_tag, &result, tag_name_size);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag int_tag in vertex_int_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first vertex and retrieve */
  iMesh_setArrData(mesh, &dum_vert, 1, *int_tag, 
                   (const char*)(&dum_val), sizeof(int), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set int tag (val=11) in vertex_int_tag_test.\n");
    return FALSE;
  }

  iMesh_getArrData(mesh, &dum_vert, 1, *int_tag,
                   &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result || dum_val2 != 11) {
    printf("Failed to get int tag (val=11) in vertex_int_tag_test.\n");
    return FALSE;
  }

  if (dum_val2 != 11) {
    
    printf("Value of vertex tag (val=11) wrong.\n");
    return FALSE;
  }

    /* put a value in the last vertex and retrieve */
  dum_val = 12;

  iMesh_setArrData(mesh, &dum_vert, 1, *int_tag, 
                   (char*)(&dum_val), sizeof(int), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set int tag (val=12) in vertex_int_tag_test.\n");
    return FALSE;
  }

  iMesh_getArrData(mesh, &dum_vert, 1, *int_tag,
                   &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get int tag (val=12) in vertex_int_tag_test.\n");
    return FALSE;
  }

  if (dum_val2 != 12) {
    
    printf("Value of vertex tag (val=12) wrong.\n");
    return FALSE;
  }

    /* ok, the int tag test worked */
  return TRUE;
}

int vertex_double_tag_test(iMesh_Instance mesh, 
                            iBase_EntityHandle *verts, int  /*verts_size*/,
                            iBase_TagHandle *double_tag) 
{
  int result;

  iBase_EntityHandle dum_vert = verts[0];
  double dum_val = 1.0e6, dum_val2;
  void *dum_val2_ptr = &dum_val2;
  int dum_val2_alloc = sizeof(double), dum_val2_size;
  
    /* create a tag */
  const char *tag_name = "double_tag";
  int tag_name_size = 11;
  iMesh_createTag(mesh, tag_name, 1, iBase_DOUBLE, 
                  double_tag, &result, tag_name_size);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag double_tag in vertex_double_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first vertex and retrieve */
  iMesh_setArrData(mesh, &dum_vert, 1, *double_tag, 
                   (char*)(&dum_val), sizeof(double), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set double tag (val=1.0e6) in vertex_double_tag_test.\n");
    return FALSE;
  }

  iMesh_getArrData(mesh, &dum_vert, 1, *double_tag,
                   &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get double tag (val=1.0e6) in vertex_double_tag_test.\n");
    return FALSE;
  }

  if (dum_val2 != 1.0e6) {
    printf("Value of vertex tag (val=1.0e6) wrong.\n");
    return FALSE;
  }

    /* put a value in the last vertex and retrieve */
  dum_val = 2.0e9;

  iMesh_setArrData(mesh, &dum_vert, 1, *double_tag, 
                   (char*)(&dum_val), sizeof(double), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set double tag (val=2.0e9) in vertex_double_tag_test.\n");
    return FALSE;
  }

  iMesh_getArrData(mesh, &dum_vert, 1, *double_tag,
                   &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get double tag (val=2.0e9) in vertex_double_tag_test.\n");
    return FALSE;
  }

  if (dum_val2 != 2.0e9) {
    printf("Value of vertex tag (val=2.0e9) wrong.\n");
    return FALSE;
  }

    /* ok, the double tag test worked */
  return TRUE;
}

  /* Add a struct Vertex Tag to the database */
struct TagStruct {
  double test_double;
  int test_int1, test_int2;
};

int vertex_struct_tag_test(iMesh_Instance mesh, 
                            iBase_EntityHandle *verts, int /*verts_size*/,
                            iBase_TagHandle *struct_tag) 
{
  int result;
  iBase_EntityHandle dum_vert = verts[0];
  struct TagStruct dum_struct = {3.0e12, 2, 3}, dum_struct2;
  void *dum_struct2_ptr = &dum_struct2;
  int dum_struct_alloc = sizeof(struct TagStruct), dum_struct_size;

    /* create a tag */
  const char *tag_name = "struct_tag";
  int tag_name_size = 11;
  iMesh_createTag(mesh, tag_name, sizeof(struct TagStruct), 
                  iBase_BYTES, struct_tag, &result, tag_name_size);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag struct_tag in vertex_struct_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first vertex and retrieve */

    /* careful setting the value, since tags are opaque */
  iMesh_setArrData(mesh, &dum_vert, 1, *struct_tag, 
                   (const char*) &dum_struct, sizeof(struct TagStruct), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set struct tag in vertex_struct_tag_test.\n");
    return FALSE;
  }

  iMesh_getArrData(mesh, &dum_vert, 1, *struct_tag,
                   &dum_struct2_ptr, &dum_struct_alloc, &dum_struct_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get struct tag in vertex_struct_tag_test.\n");
    return FALSE;
  }

  if (dum_struct_size != sizeof(struct TagStruct)) {
    printf("Size of vertex struct tag wrong.\n");
    return FALSE;
  }
  if (dum_struct2.test_int1 != dum_struct.test_int1 ||
     dum_struct2.test_int2 != dum_struct.test_int2 ||
     dum_struct2.test_double != dum_struct.test_double) {
    printf("Value of vertex struct tag wrong.\n");
    return FALSE;
  }

    /* ok, the int tag test worked */
  return TRUE;
}

int vertex_tag_delete_test(iMesh_Instance mesh, 
                            iBase_EntityHandle *verts, int /*verts_size*/)
{
  int result;
  int delete_err = FALSE;

    /* test forced, unforced deletion of tags from entities */

    /* test getAlliBase_TagHandles for first entity */
  iBase_TagHandle *all_tags = NULL;
  iBase_EntityHandle dum_entity = verts[0];
  int all_tags_alloc = 0, all_tags_size;
  iMesh_getAllTags(mesh, verts[0], &all_tags, &all_tags_alloc,
                   &all_tags_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't get all tag handles from vertex.\n");
    return FALSE;
  }

  iMesh_rmvArrTag(mesh, &dum_entity, 1, all_tags[0], &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't remove tag from vertex.\n");
    free(all_tags);
    return FALSE;
  }
  
  iMesh_destroyTag(mesh, all_tags[1], FALSE, &result);
  if (iBase_SUCCESS != result) {
    delete_err = TRUE;
  }
  if (!delete_err) {
    printf("Error when unforced-deleting tag in use on a vertex.\n");
    free(all_tags);
    return FALSE;
  }

  iMesh_destroyTag(mesh, all_tags[1], TRUE, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't force-delete a tag in use on a vertex.\n");
    free(all_tags);
    return FALSE;
  }

  free(all_tags);

    /* ok, we're done */
  return TRUE;
}

int vertex_tag_test(iMesh_Instance mesh)
{
  int result;

  iBase_TagHandle int_tag, double_tag, struct_tag;
  int int_err, double_err, struct_err, tag_delete_err;

  int info_err = tag_info_test(mesh);
  
    /* get all the vertices */
  iBase_EntityHandle *verts = NULL;
  int verts_alloc = 0, verts_size;
  iMesh_getEntities(mesh, root_set, iBase_ALL_TYPES, 
                    iMesh_POINT, &verts, &verts_alloc, &verts_size, &result);
  if (iBase_SUCCESS != result) {
    printf("entitysetGetEntities failed in vertex_tag_test.\n");
    return FALSE;
  }
  
    /* vertex int tag */
  int_err = vertex_int_tag_test(mesh, verts, verts_size, &int_tag);
  
    /* vertex double tag */
  double_err = vertex_double_tag_test(mesh, verts, verts_size, 
                                           &double_tag);
  
    /* vertex struct tag */
  struct_err = vertex_struct_tag_test(mesh, verts, verts_size, 
                                           &struct_tag);

  tag_delete_err = vertex_tag_delete_test(mesh, verts, verts_size);

  free(verts);
  
  return (info_err && int_err && double_err && 
          struct_err && tag_delete_err);
}

int entityset_int_tag_test(iMesh_Instance mesh, 
                            iBase_EntitySetHandle *sets, int sets_size,
                            iBase_TagHandle *int_tag) 
{
  int result;
  int dum_val = 11, dum_val2;
  void *dum_val2_ptr = &dum_val2;
  int dum_val2_alloc = sizeof(int), dum_val2_size;

    /* create a tag */
  const char *tag_name = "set_int_tag";
  iMesh_createTag(mesh, tag_name, 1, iBase_INTEGER, 
                  int_tag, &result, 12);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag int_tag in entityset_int_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first set and retrieve */

  iMesh_setEntSetData(mesh, sets[0], *int_tag, 
                      (char*)(&dum_val), sizeof(int), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set int tag (val=11) in entityset_int_tag_test.\n");
    return FALSE;
  }

  dum_val2 = 0;
  iMesh_getEntSetData(mesh, sets[0], *int_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get int tag (val=11) in entityset_int_tag_test.");
    return FALSE;
  }

  if (dum_val2 != 11) {
    printf("Value of entityset tag (val=11) wrong.\n");
    return FALSE;
  }

    /* put a value in the last faces and retrieve */
  dum_val = 12;
  iMesh_setEntSetData(mesh, sets[sets_size - 1], *int_tag,
                      (char*)(&dum_val), sizeof(int), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set int tag (val=12) in entityset_int_tag_test.");
    return FALSE;
  }

  iMesh_getEntSetData(mesh, sets[sets_size - 1], *int_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get int tag (val=12) in entityset_int_tag_test.");
    return FALSE;
  }
  
  if (dum_val2 != 12) {
    printf("Value of entityset tag (val=12) wrong.\n");
    return FALSE;
  }

    /* ok, the int tag test worked */
  return TRUE;
}

int entityset_double_tag_test(iMesh_Instance mesh, 
                               iBase_EntitySetHandle *sets, int sets_size,
                               iBase_TagHandle *double_tag) 
{
  int result;
  double dum_val = 1.0e6, dum_val2;
  void *dum_val2_ptr = &dum_val2;
  int dum_val2_alloc = sizeof(double), dum_val2_size;

    /* create a tag */
  const char *tag_name = "set_double_tag";
  iMesh_createTag(mesh, tag_name, 1, iBase_DOUBLE, 
                  double_tag, &result, 15);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag double_tag in entityset_double_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first set and retrieve */

  iMesh_setEntSetData(mesh, sets[0], *double_tag, 
                      (char*)(&dum_val), sizeof(double), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set double tag (val=11) in entityset_double_tag_test.\n");
    return FALSE;
  }

  dum_val2 = 0;
  iMesh_getEntSetData(mesh, sets[0], *double_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get double tag (val=1.0e6) in entityset_double_tag_test.");
    return FALSE;
  }

  if (dum_val2 != 1.0e6) {
    printf("Value of entityset tag (val=11) wrong.\n");
    return FALSE;
  }

    /* put a value in the last faces and retrieve */
  dum_val = 2.0e9;
  iMesh_setEntSetData(mesh, sets[sets_size-1], *double_tag,
                      (char*)(&dum_val), sizeof(double), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set double tag (val=2.0e9) in entityset_double_tag_test.");
    return FALSE;
  }

  iMesh_getEntSetData(mesh, sets[sets_size-1], *double_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get double tag (val=2.0e9) in entityset_double_tag_test.");
    return FALSE;
  }
  
  if (dum_val2 != 2.0e9) {
    printf("Value of entityset tag (val=2.0e9) wrong.\n");
    return FALSE;
  }

    /* ok, the double tag test worked */
  return TRUE;
}

int entityset_struct_tag_test(iMesh_Instance mesh, 
                               iBase_EntitySetHandle * /*sets*/, int /*sets_size*/,
                               iBase_TagHandle *struct_tag) 
{
  int result;
  struct TagStruct dum_struct = {3.0e12, 2, 3}, dum_struct2;
  void *dum_struct2_ptr = &dum_struct2;
  int dum_struct_alloc = sizeof(struct TagStruct), dum_struct_size;

    /* create a tag */
  const char *tag_name = "set_struct_tag";
  iMesh_createTag(mesh, tag_name, sizeof(struct TagStruct), iBase_BYTES, 
                  struct_tag, &result, 11);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag struct_tag in entityset_struct_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first vertex and retrieve */

    /* careful setting the value, since tags are opaque */
  iMesh_setEntSetData(mesh, root_set, *struct_tag, 
                      (const char*) &dum_struct, sizeof(struct TagStruct), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set struct tag in entityset_struct_tag_test.\n");
    return FALSE;
  }

  iMesh_getEntSetData(mesh, root_set, *struct_tag,
                      &dum_struct2_ptr, &dum_struct_alloc, &dum_struct_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get struct tag in entityset_struct_tag_test.\n");
    return FALSE;
  }

  if (dum_struct_size != sizeof(struct TagStruct)) {
    printf("Size of entityset struct tag wrong.\n");
    return FALSE;
  }
  if (dum_struct2.test_int1 != dum_struct.test_int1 ||
     dum_struct2.test_int2 != dum_struct.test_int2 ||
     dum_struct2.test_double != dum_struct.test_double) {
    printf("Value of entityset struct tag wrong.\n");
    return FALSE;
  }

    /* ok, the int tag test worked */
  return TRUE;
}

int entityset_tag_delete_test(iMesh_Instance mesh, 
                               iBase_EntitySetHandle *sets, int sets_size) 
{
    /* test forced, unforced deletion of tags from entities */
  int result;
  int delete_err = FALSE;

    /* test getAlliBase_TagHandles for first entity */
  iBase_TagHandle *all_tags = NULL;
  if (sets_size < 1)
  {
    printf("no sets.\n");
    return FALSE;
  }
  iBase_EntitySetHandle dum_entity = sets[0];
  int all_tags_alloc = 0, all_tags_size;
  iMesh_getAllEntSetTags(mesh, sets[0], &all_tags, &all_tags_alloc,
             &all_tags_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't get all tag handles from entityset.\n");
    free(all_tags);
    return FALSE;
  }

  iMesh_rmvEntSetTag(mesh, dum_entity, all_tags[0], &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't remove tag from entityset.\n");
    free(all_tags);
    return FALSE;
  }
  
  iMesh_destroyTag(mesh, all_tags[1], FALSE, &result);
  if (iBase_SUCCESS != result) {
    delete_err = TRUE;
  }
  if (!delete_err) {
    printf("Error when unforced-deleting tag in use on a entityset.\n");
    free(all_tags);
    return FALSE;
  }

  iMesh_destroyTag(mesh, all_tags[1], TRUE, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't force-delete a tag in use on a entityset.\n");
    free(all_tags);
    return FALSE;
  }

  free(all_tags);
  
    /* ok, we're done */
  return TRUE;
}

/*!
  @test
  TSTT entityset tag Test
  @li Check entityset tags
*/
int entityset_tag_test(iMesh_Instance mesh)
{
  int result;
  iBase_TagHandle int_tag, double_tag, struct_tag;
  int int_success, double_success, struct_success, tag_delete_success;

    /* get the sets */
  iBase_EntitySetHandle *esets = NULL;
  int esets_alloc = 0, esets_size;
  iMesh_getEntSets(mesh, root_set, 1, &esets, &esets_alloc, &esets_size, &result);
  if (iBase_SUCCESS != result) {
    printf("entitysetGetEntities failed in entityset_tag_test.\n");
    free(esets);
    return FALSE;
  }

     /* entityset int tag */
  int_success = entityset_int_tag_test(mesh, esets, esets_size, 
                                            &int_tag);

    /* entityeset double tag */
  double_success = entityset_double_tag_test(mesh, esets, esets_size, 
                                                  &double_tag);

    /* entityset struct tag */
  struct_success = entityset_struct_tag_test(mesh, esets, esets_size, 
                                                  &struct_tag);

  tag_delete_success = entityset_tag_delete_test(mesh, esets, esets_size);

  free(esets);

  return (int_success && double_success && struct_success
          && tag_delete_success);
}

int mesh_int_tag_test(iMesh_Instance mesh, 
                       iBase_TagHandle *int_tag) 
{
  int result;
  int dum_val = 11, dum_val2;
  void *dum_val2_ptr = &dum_val2;
  int dum_val2_alloc = sizeof(int), dum_val2_size;

    /* create a tag */
  const char *tag_name = "mesh_int_tag";
  iMesh_createTag(mesh, tag_name, 1, iBase_INTEGER, 
                  int_tag, &result, 12);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag int_tag in mesh_int_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first set and retrieve */

  iMesh_setEntSetData(mesh, root_set, *int_tag, 
                      (char*)(&dum_val), sizeof(int), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set int tag (val=11) in mesh_int_tag_test.\n");
    return FALSE;
  }

  dum_val2 = 0;
  iMesh_getEntSetData(mesh, root_set, *int_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get int tag (val=11) in mesh_int_tag_test.");
    return FALSE;
  }

  if (dum_val2 != 11) {
    printf("Value of entityset tag (val=11) wrong.\n");
    return FALSE;
  }

    /* put a value in the last faces and retrieve */
  dum_val = 12;
  iMesh_setEntSetData(mesh, root_set, *int_tag,
                      (char*)(&dum_val), sizeof(int), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set int tag (val=12) in mesh_int_tag_test.");
    return FALSE;
  }

  iMesh_getEntSetData(mesh, root_set, *int_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get int tag (val=12) in mesh_int_tag_test.");
    return FALSE;
  }
  
  if (dum_val2 != 12) {
    printf("Value of entityset tag (val=12) wrong.\n");
    return FALSE;
  }

    /* ok, the int tag test worked */
  return TRUE;
}

int mesh_double_tag_test(iMesh_Instance mesh, 
                               iBase_TagHandle *double_tag) 
{
  int result;
  double dum_val = 1.0e6, dum_val2;
  void *dum_val2_ptr = &dum_val2;
  int dum_val2_alloc = sizeof(double), dum_val2_size;

    /* create a tag */
  const char *tag_name = "mesh_double_tag";
  iMesh_createTag(mesh, tag_name, 1, iBase_DOUBLE, 
                  double_tag, &result, 15);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag double_tag in mesh_double_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first set and retrieve */

  iMesh_setEntSetData(mesh, root_set, *double_tag, 
                      (char*)(&dum_val), sizeof(double), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set double tag (val=11) in mesh_double_tag_test.\n");
    return FALSE;
  }

  dum_val2 = 0;
  iMesh_getEntSetData(mesh, root_set, *double_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get double tag (val=1.0e6) in mesh_double_tag_test.");
    return FALSE;
  }

  if (dum_val2 != 1.0e6) {
    printf("Value of entityset tag (val=11) wrong.\n");
    return FALSE;
  }

    /* put a value in the last faces and retrieve */
  dum_val = 2.0e9;
  iMesh_setEntSetData(mesh, root_set, *double_tag,
                      (char*)(&dum_val), sizeof(double), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set double tag (val=2.0e9) in mesh_double_tag_test.");
    return FALSE;
  }

  iMesh_getEntSetData(mesh, root_set, *double_tag,
                      &dum_val2_ptr, &dum_val2_alloc, &dum_val2_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get double tag (val=2.0e9) in mesh_double_tag_test.");
    return FALSE;
  }
  
  if (dum_val2 != 2.0e9) {
    printf("Value of entityset tag (val=2.0e9) wrong.\n");
    return FALSE;
  }

    /* ok, the double tag test worked */
  return TRUE;
}

int mesh_struct_tag_test(iMesh_Instance mesh, 
                               iBase_TagHandle *struct_tag) 
{
  int result;
  struct TagStruct dum_struct = {3.0e12, 2, 3}, dum_struct2;
  void *dum_struct2_ptr = &dum_struct2;
  int dum_struct_alloc = sizeof(struct TagStruct), dum_struct_size;

    /* create a tag */
  const char *tag_name = "mesh_struct_tag";
  iMesh_createTag(mesh, tag_name, sizeof(struct TagStruct), iBase_BYTES, 
                  struct_tag, &result, 11);
  if (iBase_SUCCESS != result) {
    printf("Failed to create tag struct_tag in vertex_struct_tag_test.\n");
    return FALSE;
  }

    /* put a value in the first vertex and retrieve */

    /* careful setting the value, since tags are opaque */
  iMesh_setEntSetData(mesh, root_set, *struct_tag, 
                      (const char*) &dum_struct, sizeof(struct TagStruct), &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to set struct tag in mesh_struct_tag_test.\n");
    return FALSE;
  }

  iMesh_getEntSetData(mesh, root_set, *struct_tag,
                      &dum_struct2_ptr, &dum_struct_alloc, &dum_struct_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to get struct tag in mesh_struct_tag_test.\n");
    return FALSE;
  }

  if (dum_struct_size != sizeof(struct TagStruct)) {
    printf("Size of entityset struct tag wrong.\n");
    return FALSE;
  }
  if (dum_struct2.test_int1 != dum_struct.test_int1 ||
     dum_struct2.test_int2 != dum_struct.test_int2 ||
     dum_struct2.test_double != dum_struct.test_double) {
    printf("Value of entityset struct tag wrong.\n");
    return FALSE;
  }

    /* ok, the int tag test worked */
  return TRUE;
}

int mesh_tag_delete_test(iMesh_Instance mesh) 
{
    /* test forced, unforced deletion of tags from entities */
  int result;
  int delete_err = FALSE;

    /* test getAlliBase_TagHandles for first entity */
  iBase_TagHandle *all_tags = NULL;
  int all_tags_alloc = 0, all_tags_size;
  iMesh_getAllEntSetTags(mesh, root_set, &all_tags, &all_tags_alloc,
                         &all_tags_size, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't get all tag handles from entityset.\n");
    return FALSE;
  }

  iMesh_rmvEntSetTag(mesh, root_set, all_tags[0], &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't remove tag from entityset.\n");
    free(all_tags);
    return FALSE;
  }
  
  iMesh_destroyTag(mesh, all_tags[1], FALSE, &result);
  if (iBase_SUCCESS != result) {
    delete_err = TRUE;
  }
  if (!delete_err) {
    printf("Error when unforced-deleting tag in use on a entityset.\n");
    free(all_tags);
    return FALSE;
  }

  iMesh_destroyTag(mesh, all_tags[1], TRUE, &result);
  if (iBase_SUCCESS != result) {
    printf("Couldn't force-delete a tag in use on a entityset.\n");
    free(all_tags);
    return FALSE;
  }

  free(all_tags);
  
    /* ok, we're done */
  return TRUE;
}

/*!
  @test
  TSTT entityset tag Test
  @li Check entityset tags
*/
int mesh_tag_test(iMesh_Instance mesh)
{
  iBase_TagHandle int_tag, double_tag, struct_tag;
  
    /* entityset int tag */
  int int_success = mesh_int_tag_test(mesh, &int_tag);

    /* entityeset double tag */
  int double_success = mesh_double_tag_test(mesh, &double_tag);

    /* entityset struct tag */
  int struct_success = mesh_struct_tag_test(mesh, &struct_tag);

  int tag_delete_success = mesh_tag_delete_test(mesh);

  return (int_success && double_success && struct_success
          && tag_delete_success);
}

int set_remove_contained_regression( iMesh_Instance mesh )
{
    int err, contained;
    iBase_EntitySetHandle set, sub;

    iMesh_createEntSet(mesh, 0, &set, &err);
    if (iBase_SUCCESS != err)
      return 0;
    iMesh_createEntSet(mesh, 1, &sub, &err);
    if (iBase_SUCCESS != err)
      return 0;

    iMesh_addEntSet(mesh, sub, set, &err);
    if (iBase_SUCCESS != err)
      return 0;

    iMesh_isEntSetContained(mesh, set, sub, &contained, &err);
    if (iBase_SUCCESS != err)
      return 0;
    if (!contained) {
      fprintf(stderr, "isEntSetContained returned false for contained set\n");
      return 0;
    }

    iMesh_rmvEntSet(mesh, sub, set, &err);
    if (iBase_SUCCESS != err)
      return 0;

    iMesh_isEntSetContained(mesh, set, sub, &contained, &err);
    if (iBase_SUCCESS != err)
      return 0;
    if (contained) {
      fprintf(stderr, "isEntSetContained returned true for removed set\n");
      return 0;
    }

    return 1;
}

int all_adjacency_regression( iMesh_Instance mesh )
{
    int err;

    double coords[] = { 0,0,0, 1,1,1 };

    iBase_EntityHandle *verts = NULL;
    int verts_alloc = 0, verts_size;

    iBase_EntityHandle line;
    int status;

    iBase_EntityHandle *adj = NULL;
    int adj_alloc = 0, adj_size;

    iMesh_newMesh("", &mesh, &err, 0);
    if (iBase_SUCCESS != err)
      return 0;

    iMesh_createVtxArr(mesh, 2, iBase_INTERLEAVED, coords, 6, &verts, &verts_alloc,
                       &verts_size, &err);
    if (iBase_SUCCESS != err)
      return 0;

    iMesh_createEnt(mesh, iMesh_LINE_SEGMENT, verts, 2, &line, &status, &err);
    if (iBase_SUCCESS != err || status != iBase_NEW)
      return 0;

    iMesh_getEntAdj(mesh, verts[0], iBase_ALL_TYPES, &adj, &adj_alloc, &adj_size,
                    &err);
    if (iBase_SUCCESS != err)
      return 0;
    if (adj_size != 1 || adj[0] != line) {
        printf("Bad: couldn't find adjacency for vertex\n");
        return 0;
    }
    free(adj);

    adj_alloc = 0; adj = NULL;
    iMesh_getEntAdj(mesh, line, iBase_ALL_TYPES, &adj, &adj_alloc, &adj_size,
                    &err);
    if (iBase_SUCCESS != err)
      return 0;
    if (adj_size != 2 || ((adj[0] != verts[0] || adj[1] != verts[1]) &&
                         (adj[0] != verts[1] || adj[1] != verts[0])) ) {
        printf("Bad: couldn't find adjacencies for line\n");
        free(adj);
        free(verts);
        return 0;
    }
    free(adj);
    free(verts);
    iMesh_dtor(mesh,&err);
    if (iBase_SUCCESS != err)
      return 0;
    return 1;
}

static int ordered_set_regression( iMesh_Instance mesh )
{
  double coords[] = { 0,0,0, 1,1,1 };
  iBase_EntityHandle line, *p, verts[2], ents_in[3], ents_out[3];
  iBase_EntitySetHandle set;
  int i, err, status, two = 2, three = 3;

  iMesh_newMesh("", &mesh, &err, 0);
  CHK(err);
  p = verts;
  iMesh_createVtxArr(mesh, 2, iBase_INTERLEAVED, coords, 6, &p, &two, &two, &err);
  CHK(err);
  iMesh_createEnt(mesh, iMesh_LINE_SEGMENT, verts, 2, &line, &status, &err);
  CHK(err);
  if (status != iBase_NEW)
    return 0;
  iMesh_createEntSet(mesh, 1, &set, &err);
  CHK(err);
  ents_in[0] = verts[1];
  ents_in[1] = line;
  ents_in[2] = verts[0];
  iMesh_addEntArrToSet(mesh, ents_in, three, set, &err);
  CHK(err);
  p = ents_out;
  iMesh_getEntities(mesh, set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, &p, &three, &three, &err);
  CHK(err);
  for (i = 0; i < three; i++) {
    if (ents_in[i] != ents_out[i]) {
      printf("Ordered set does not preserve order\n");
      return 0;
    }
  }
  iMesh_dtor(mesh, &err);
  CHK(err);
  return 1;
}

int array_allocation( iMesh_Instance mesh )
{
  int err;

  double coords[] = { 0,0,0, 1,1,1, 2,2,2, 3,3,3 };

  iBase_EntityHandle *verts = NULL;
  int verts_alloc = 0, verts_size;

  iBase_EntityHandle *ents;
  int ents_alloc, ents_size;

  iMesh_newMesh("", &mesh, &err, 0);
  if (iBase_SUCCESS != err)
    return 0;

  iMesh_getRootSet(mesh, &root_set, &err);
  if (iBase_SUCCESS != err) {
    printf("Failed to return a root set.\n");
    return 0;
  }

  iMesh_createVtxArr(mesh, 4, iBase_INTERLEAVED, coords, 12, &verts, &verts_alloc,
                     &verts_size, &err);
  if (iBase_SUCCESS != err)
    return 0;

  free(verts);

    /* test for proper allocation when array pointer passed in null but alloc'd size not */
  ents_alloc = 3;
  ents = NULL;
  iMesh_getEntities(mesh, root_set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, 
                    &ents, &ents_alloc, &ents_size, &err);
  CHK(err);

  free(ents);

    /* test for proper allocation when array pointer passed in non-null but alloc'd size 0 */
  ents_alloc = 0;
  iMesh_getEntities(mesh, root_set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, 
                    &ents, &ents_alloc, &ents_size, &err);
  CHK(err);

  free(ents);

    /* test for failure when passed in alloc'd size is smaller than it should be */
  ents_alloc -= 1;
  iMesh_getEntities(mesh, root_set, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, 
                    &ents, &ents_alloc, &ents_size, &err);
  if (iBase_BAD_ARRAY_SIZE != err && iBase_BAD_ARRAY_DIMENSION != err) {
    err = iBase_FAILURE;
    CHK(err);
  }

  iMesh_dtor(mesh, &err);
  CHK(err);

  return 1;
}

int compare_single_iter( const char* info,
                         iMesh_Instance mesh,
                         iBase_EntitySetHandle set,
                         iBase_EntityHandle* contents,
                         int contents_size,
                         enum iBase_EntityType type,
                         enum iMesh_EntityTopology topo )
{
  iBase_EntityIterator iter = 0;
  int i, twice, has_data, result = iBase_SUCCESS, result2;
  iBase_EntityHandle value;
  
  iMesh_initEntIter( mesh, set, type, topo, 0, &iter, &result );
  if (iBase_SUCCESS != result) {
    printf("%s:%d: Error %d initializing %s iterator for type %d/topo %d\n",
      __FILE__, __LINE__, result, info, (int)type, (int)topo);
    return result;
  }
  
  for (twice = 0; twice < 2; ++twice) {
  
    for (i = 0; i < contents_size; ++i) {
      iMesh_getNextEntIter( mesh, iter, &value, &has_data, &result );
      if (iBase_SUCCESS != result) {
        printf("%s:%d: Error %d stepping %s iterator for type %d/topo %d\n",
          __FILE__, __LINE__, result, info, (int)type, (int)topo);
        goto end_single_iter;
      }

      if (!has_data) {
        printf("%s:%d: %s iterator for type %d/topo %d ended prematurely at %d of %d\n",
          __FILE__, __LINE__, info, (int)type, (int)topo, i, contents_size);
        result = iBase_FAILURE;
        goto end_single_iter;
      }

      if (value != contents[i]) {
        printf("%s:%d: %s iterator for type %d/topo %d returned incorrect value at %d of %d\n",
          __FILE__, __LINE__, info, (int)type, (int)topo, i, contents_size);
        result = iBase_FAILURE;
        goto end_single_iter;
      }
    }

    iMesh_getNextEntIter( mesh, iter, &value, &has_data, &result );
    if (iBase_SUCCESS != result) {
      printf("%s:%d: Error %d stepping %s iterator for type %d/topo %d\n",
        __FILE__, __LINE__, result, info, (int)type, (int)topo);
      goto end_single_iter;
    }

    if (has_data) {
      printf("%s:%d: %s iterator for type %d/topo %d did not end after %d values\n",
        __FILE__, __LINE__, info, (int)type, (int)topo, contents_size);
      result = iBase_FAILURE;
      goto end_single_iter;
    }

    iMesh_resetEntIter( mesh, iter, &result );
    if (iBase_SUCCESS != result) {
      printf("%s:%d: Error %d resetting %s iterator for type %d/topo %d\n",
        __FILE__, __LINE__, result, info, (int)type, (int)topo);
      result = iBase_FAILURE;
      goto end_single_iter;
    }
  }

end_single_iter:
  iMesh_endEntIter( mesh, iter, &result2 );
  if (iBase_SUCCESS != result2) {
    printf("%s:%d: Error %d releasing %s iterator for type %d/topo %d\n",
      __FILE__, __LINE__, result, info, (int)type, (int)topo);
    if (iBase_SUCCESS == result)
      result = result2;
  }
  return result;
}

int compare_array_iter( const char* info,
                        iMesh_Instance mesh,
                        iBase_EntitySetHandle set,
                        iBase_EntityHandle* contents,
                        int contents_size,
                        int array_size,
                        enum iBase_EntityType type,
                        enum iMesh_EntityTopology topo )
{
  iBase_EntityArrIterator iter = 0;
  int i, j, twice, has_data, result = iBase_SUCCESS, result2;
  iBase_EntityHandle* values;
  int values_size, values_alloc = array_size;
  values = (iBase_EntityHandle*)malloc( array_size * sizeof(iBase_EntityHandle) );
  
  iMesh_initEntArrIter( mesh, set, type, topo, array_size, 0, &iter, &result );
  if (iBase_SUCCESS != result) {
    printf("%s:%d: Error %d initializing %s array iterator for type %d/topo %d\n",
      __FILE__, __LINE__, result, info, (int)type, (int)topo);
    free(values);
    return result;
  }
  
  for (twice = 0; twice < 2; ++twice) {
    i = 0;
    while (i < contents_size) {
      
      iMesh_getNextEntArrIter( mesh, iter, &values, &values_alloc, &values_size, &has_data, &result );
      if (iBase_SUCCESS != result) {
        printf("%s:%d: Error %d stepping %s array iterator for type %d/topo %d\n",
          __FILE__, __LINE__, result, info, (int)type, (int)topo);
        goto end_arr_iter;
      }

      if (!has_data || !values_size) {
        printf("%s:%d: %s array iterator for type %d/topo %d ended prematurely at %d of %d\n",
          __FILE__, __LINE__,info, (int)type, (int)topo, i, contents_size);
        result = iBase_FAILURE;
        goto end_arr_iter;
      }
      
      if (i + values_size > contents_size) {
        printf("%s:%d: %s array iterator for type %d/topo %d returned more than %d handles\n",
          __FILE__, __LINE__,info, (int)type, (int)topo, contents_size);
        result = iBase_FAILURE;
        goto end_arr_iter;
      }
      
      if (contents_size - i >= array_size && values_size < array_size) {
        printf("%s:%d: %s array iterator for type %d/topo %d returned fewer than %d handles\n",
          __FILE__, __LINE__, info, (int)type, (int)topo, array_size);
        result = iBase_FAILURE;
        goto end_arr_iter;
      }

      for (j = 0; j < values_size; ++j, ++i) {
        if (values[j] != contents[i]) {
          printf("%s:%d: %s array iterator for type %d/topo %d returned incorrect value at %d of %d\n",
            __FILE__, __LINE__, info, (int)type, (int)topo, i, contents_size);
          result = iBase_FAILURE;
          goto end_arr_iter;
        }
      }
    }

    iMesh_getNextEntArrIter( mesh, iter, &values, &values_alloc, &values_size, &has_data, &result );
    if (iBase_SUCCESS != result) {
      printf("%s:%d: Error %d stepping %s array iterator for type %d/topo %d\n",
        __FILE__, __LINE__, result, info, (int)type, (int)topo);
      goto end_arr_iter;
    }

    if (has_data || values_size) {
      printf("%s:%d: %s array iterator for type %d/topo %d did not end after %d values\n",
        __FILE__, __LINE__, info, (int)type, (int)topo, contents_size);
      result = iBase_FAILURE;
      goto end_arr_iter;
    }

    iMesh_resetEntArrIter( mesh, iter, &result );
    if (iBase_SUCCESS != result) {
      printf("%s:%d: Error %d resetting %s array iterator for type %d/topo %d\n",
        __FILE__, __LINE__, result, info, (int)type, (int)topo);
      result = iBase_FAILURE;
      goto end_arr_iter;
    }
  }

end_arr_iter:
  free(values);
  iMesh_endEntArrIter( mesh, iter, &result2 );
  if (iBase_SUCCESS != result2) {
    printf("%s:%d: Error %d releasing %s array iterator for type %d/topo %d\n",
      __FILE__, __LINE__, result, info, (int)type, (int)topo);
    if (iBase_SUCCESS == result)
      result = result2;
  }
  return result;
}

int test_iterator_common( const char* info,
                          iMesh_Instance mesh,
                          iBase_EntitySetHandle set,
                          int array_size,
                          enum iBase_EntityType type,
                          enum iMesh_EntityTopology topo )
{
  iBase_EntityHandle *contents = NULL;
  int content_size = 0, content_alloc = 0;
  int result;

  iMesh_getEntities( mesh, set, type, topo, &contents, &content_alloc, &content_size, &result );
  CHK(result);
  if (array_size == 1) 
    result = compare_single_iter( info, mesh, set, contents, content_size, type, topo );
  else
    result = compare_array_iter(  info, mesh, set, contents, content_size, array_size, type, topo );
  free(contents);
  return result;
}

int test_iterator( iMesh_Instance mesh )
{
  int result, i;
  iBase_EntitySetHandle root, list, set, *setptr;
  iBase_EntityHandle* array = NULL;
  int array_len = 0, array_size = 0;

  iMesh_getRootSet( mesh, &root, &result );
  CHK(result);

  /* create some sets containing every other handle */
  iMesh_getEntities( mesh, root, iBase_ALL_TYPES, iMesh_ALL_TOPOLOGIES, 
                     &array, &array_size, &array_len, &result );
  CHK(result);
  for (i = 1; i < array_size; i += 2)
    array[i] = array[i - 1];
  for (i = 0; i < 2; ++i) {
    setptr = i ? &list : &set;
    iMesh_createEntSet( mesh, i, setptr, &result );
    if (iBase_SUCCESS != result)
      free(array);
    CHK(result);
    iMesh_addEntArrToSet( mesh, array, array_size, *setptr, &result );
    if (iBase_SUCCESS != result)
      free(array);
    CHK(result);
  }
  free(array);

  /* test single iterator and array iterator for all types */
  for (i = 0; i < iBase_ALL_TYPES; ++i) {
    array_size = 2*i + 2;
    result = test_iterator_common( "root", mesh, root, 1, (enum iBase_EntityType)i, iMesh_ALL_TOPOLOGIES );
    CHK(result);
    result = test_iterator_common( "root", mesh, root, array_size, (enum iBase_EntityType)i, iMesh_ALL_TOPOLOGIES );
    CHK(result);
    result = test_iterator_common( "list", mesh, list, 1, (enum iBase_EntityType)i, iMesh_ALL_TOPOLOGIES );
    CHK(result);
    result = test_iterator_common( "list", mesh, list, array_size, (enum iBase_EntityType)i, iMesh_ALL_TOPOLOGIES );
    CHK(result);
    result = test_iterator_common( "set", mesh, set, 1, (enum iBase_EntityType)i, iMesh_ALL_TOPOLOGIES );
    CHK(result);
    result = test_iterator_common( "set", mesh, set, array_size, (enum iBase_EntityType)i, iMesh_ALL_TOPOLOGIES );
    CHK(result);
  }

  /* test single iterator and array iterator for all types */
  for (i = 0; i < iMesh_ALL_TOPOLOGIES; ++i) {
    array_size = 2*i + 2;
    result = test_iterator_common( "root", mesh, root, 1, iBase_ALL_TYPES, (enum iMesh_EntityTopology)i );
    CHK(result);
    result = test_iterator_common( "root", mesh, root, array_size, iBase_ALL_TYPES, (enum iMesh_EntityTopology)i );
    CHK(result);
    result = test_iterator_common( "list", mesh, list, 1, iBase_ALL_TYPES, (enum iMesh_EntityTopology)i );
    CHK(result);
    result = test_iterator_common( "list", mesh, list, array_size, iBase_ALL_TYPES, (enum iMesh_EntityTopology)i );
    CHK(result);
    result = test_iterator_common( "set", mesh, set, 1, iBase_ALL_TYPES, (enum iMesh_EntityTopology)i );
    CHK(result);
    result = test_iterator_common( "set", mesh, set, array_size, iBase_ALL_TYPES, (enum iMesh_EntityTopology)i );
    CHK(result);
  }

  return 1;
}

int main( int argc, char *argv[] )
{
    /* Check command line arg */
  const char *filename;
  int number_tests = 0;
  int number_tests_successful = 0;
  int number_tests_not_implemented = 0;
  int number_tests_failed = 0;
  int result;
  iMesh_Instance mesh = NULL;

  if (argc == 2) {
    filename = argv[1];
  }
  else {
    printf("Usage: %s <mesh_filename>\n", argv[0]);
    if (argc != 1)
      return 1;
    printf("  No file specified.  Defaulting to: %s\n", DEFAULT_INPUT_FILE );
    filename = DEFAULT_INPUT_FILE;
  }

    /* initialize the Mesh */
  iMesh_newMesh(NULL, &mesh, &result, 0);
  if (iBase_SUCCESS != result) {
    printf("Failed to create a mesh instance.\n");
    return 1;
  }
  iMesh_getRootSet(mesh, &root_set, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to return a root set.\n");
    return 1;
  }

    /* Print out Header information */
  printf("\n\nTSTT TEST PROGRAM:\n\n");

    /* load_mesh test */
  printf("   load_mesh: ");
  result = load_mesh_test(filename, mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* topology_adjacency_test */
  printf("   topology_adjacency_test: ");
  result = topology_adjacency_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* entity connectivity test */
  printf("   entity_connectivity_test: ");
  result = entity_connectivity_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");
    
    /* vertex_coordinates_test */
  printf("   vertex_coordinates_test: ");
  result = vertex_coordinates_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");
  
    /* topology dimension test */
  printf("   topology_dimension_test: ");
  result = topology_dimension_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* entity sets test */
  printf("   entity_sets_test: ");
  result = entity_sets_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* vertex tag test */
  printf("   vertex_tag_test: ");
  result = vertex_tag_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* entityset tag test */
  printf("   entityset_tag_test: ");
  result = entityset_tag_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* mesh tag test */
  printf("   mesh_tag_test: ");
  result = mesh_tag_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* iterator test */
  printf("   test_iterator: ");
  result = test_iterator(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* regression test for remove/contained bug */
  printf("   set_remove_contained_regression: ");
  result = set_remove_contained_regression(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* regression test for adjacencies with iBase_ALL_TYPES bug */
  printf("   all_adjacency_regression: ");
  result = all_adjacency_regression(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* test for error codes */
  printf("   error_code_test: ");
  result = error_code_test(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

      /* regression test for ordered sets not preserving order */
  printf("   ordered_set_regression: ");
  result = ordered_set_regression(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* test for array allocation behavior */
  printf("   array_allocation_regression: ");
  result = array_allocation(mesh);
  handle_error_code(result, &number_tests_failed,
                    &number_tests_not_implemented,
                    &number_tests_successful);
  number_tests++;
  printf("\n");

    /* summary */

  printf("\nTSTT TEST SUMMARY: \n");
                                                                                                                                
  printf("   Number Tests:           %d\n", number_tests);
  printf("   Number Successful:      %d\n", number_tests_successful);
  printf("   Number Not Implemented: %d\n", number_tests_not_implemented);
  printf("   Number Failed:          %d\n", number_tests_failed);
  printf("\n\n\n");

    /* delete the mesh */
  iMesh_dtor(mesh, &result);
  if (iBase_SUCCESS != result) {
    printf("Failed to destruct the mesh instance.\n");
    return 1;
  }
  
  return number_tests_failed;
}
