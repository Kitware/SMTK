
#include <stdio.h>
#include <string.h>
#include "moab_mpi.h"
#include "iMeshP.h"


#define IMESH_ASSERT(ierr) if (ierr!=0) printf("imesh assert\n");
#define IMESH_NULL 0
#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)

int main(int argc, char* argv[]){
  MPI_Init(&argc, &argv);
  printf("Hello\n");

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
  IMESH_ASSERT(ierr);

  const char options[] = " moab:PARALLEL=READ_PART "
                         " moab:PARTITION=PARALLEL_PARTITION "
                         " moab:PARALLEL_RESOLVE_SHARED_ENTS "
                         " moab:PARTITION_DISTRIBUTE ";
  const char * filename = STRINGIFY(MESHDIR) "/64bricks_1khex.h5m";;

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
  printf("There's %d entity sets here\n", num_sets);

  iMesh_dtor(imesh, &ierr);
  IMESH_ASSERT(ierr);

  printf("Done\n");
  MPI_Finalize(); //probably the 4th time this is called.. no big deal

}
