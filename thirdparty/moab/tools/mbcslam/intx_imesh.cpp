
/*
 *  This program updates a manufactured tracer field from time T0 to time T1, in parallel.
 *  Input: arrival mesh, already distributed on processors, and a departure position for
 *  each vertex, saved in a tag DP
 */
#include <stdio.h>
#include <string.h>
#include "moab_mpi.h"
#include "iMeshP.h"


#define IMESH_ASSERT(ierr) if (ierr!=0) printf("imesh assert\n");
#define IMESH_NULL 0

#ifdef __cplusplus
extern "C" {
#endif
 void update_tracer( iMesh_Instance instance,  iBase_EntitySetHandle eulerSet, int * ierr);

#ifdef __cplusplus
} // extern "C"
#endif


int main(int argc, char* argv[]){
  MPI_Init(&argc, &argv);

  iMesh_Instance imesh;
  iMeshP_PartitionHandle partn;
  int ierr, num_sets;

  iBase_EntitySetHandle root;
  imesh = IMESH_NULL;
  iMesh_newMesh(0, &imesh, &ierr, 0);
  IMESH_ASSERT(ierr);
  iMesh_getRootSet( imesh, &root, &ierr );
  IMESH_ASSERT(ierr);

  iMeshP_createPartitionAll(imesh, MPI_COMM_WORLD, &partn, &ierr);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  IMESH_ASSERT(ierr);

  const char options[] = " moab:PARALLEL=READ_PART "
                         " moab:PARTITION=PARALLEL_PARTITION "
                         " moab:PARALLEL_RESOLVE_SHARED_ENTS "
                         " moab:PARTITION_DISTRIBUTE ";
  const char * filename = "HN16DP.h5m"; // the file should have the dp tag already

  if (0==rank)
    printf("load in parallel the file: %s \n", filename);
  iMeshP_loadAll(imesh,
              partn,
              root,
              filename,
              options,
              &ierr,
              strlen(filename),
              strlen(options));
  IMESH_ASSERT(ierr);


  iMesh_getNumEntSets(imesh,
                      IMESH_NULL,
                      1,
                      &num_sets,
                      &ierr);
  IMESH_ASSERT(ierr);
  printf("There's %d entity sets here on process rank %d \n", num_sets, rank);

  iBase_EntitySetHandle euler_set;

  iMesh_createEntSet(imesh, 0, &euler_set, &ierr);
  IMESH_ASSERT(ierr);

  iBase_EntityHandle *cells = NULL;
  int ents_alloc = 0;
  int ents_size = 0;

  iMesh_getEntities(imesh, root, iBase_FACE, iMesh_ALL_TOPOLOGIES, &cells,
      &ents_alloc, &ents_size, &ierr);
  IMESH_ASSERT(ierr);

  iMesh_addEntArrToSet(imesh, cells, ents_size, euler_set, &ierr);

  IMESH_ASSERT(ierr);

  update_tracer( imesh, euler_set, &ierr);
  IMESH_ASSERT(ierr);

  // write everything
  const char * out_name = "out.h5m";
  const char optionswrite[] = " moab:PARALLEL=WRITE_PART " ;
  iMeshP_saveAll( imesh, partn, euler_set,
                     out_name,
                     optionswrite,
                     &ierr,
                     strlen(out_name),
                     strlen(optionswrite));
  IMESH_ASSERT(ierr);

  if (0==rank)
    printf("Done\n");
  MPI_Finalize(); //probably the 4th time this is called.. no big deal

}
