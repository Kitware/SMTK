! advection: do a one time step transport, using c binding module
!            example of how to do it in parallel in fortran
! This program shows how to load in parallel in moab from Fortran90, and how
!   to call the transport wrapper. It is a fortran equivalent of intx_imesh
!   driver
!
! Usage: advection

#define CHECK(a)   if (ierr .ne. 0) print *, a
program advection

  use ISO_C_BINDING
  implicit none

#include "mpif.h"
#include "moab/MOABConfig.h"
#ifdef MOAB_HAVE_MPI
#  include "iMeshP_f.h"
#else
#  include "iMesh_f.h"
#endif

! extern void update_tracer( iMesh_Instance instance,  iBase_EntitySetHandle * opEulerSet, int * ierr);
  INTERFACE
   SUBROUTINE update_tracer ( instance , opEulerSet, ierr) bind(C)
     use ISO_C_BINDING
     implicit none
     iMesh_Instance, INTENT(IN) , VALUE :: instance
     iBase_EntitySetHandle, INTENT(IN), VALUE :: opEulerSet
     integer(c_int) , INTENT (OUT) :: ierr
   END SUBROUTINE update_tracer
  END INTERFACE

  ! declarations
  ! imesh is the instance handle
  iMesh_Instance imesh

  ! cells will be storing 2d cells
  TYPE(C_PTR) cells_ptr
  iBase_EntityHandle, pointer :: cells(:)
  INTEGER ents_alloc,  ents_size

  iBase_EntitySetHandle root_set
  iBase_EntitySetHandle opEulerSet
  CHARACTER (LEN=200) options
  CHARACTER (LEN=200) filename
  CHARACTER (LEN=200) optionswrite
  CHARACTER (LEN=200) outname
  ! TYPE(C_PTR) :: vertsPtr, entsPtr

  integer rank, sz, ierr
  integer lenopt, lenname
  integer isList

#ifdef MOAB_HAVE_MPI
  ! local variables for parallel runs
  iMeshP_PartitionHandle imeshp
  IBASE_HANDLE_T mpi_comm_c
#endif

  ! init the parallel partition
  call MPI_INIT(ierr)
  CHECK("fail to initialize MPI")
  call MPI_COMM_SIZE(MPI_COMM_WORLD, sz, ierr)
  CHECK("fail to get MPI size")
  call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierr)
  CHECK("fail to get MPI rank")

  ! now load the mesh; this also initializes parallel sharing
  imesh = 0
  imeshp = 0
  call iMesh_newMesh("MOAB", imesh, ierr)
  CHECK("fail to initialize imesh")

  call iMesh_getRootSet(%VAL(imesh), root_set, ierr)
  CHECK("fail to get root set")

  call iMeshP_getCommunicator(%VAL(imesh), MPI_COMM_WORLD, mpi_comm_c, ierr)

  call iMeshP_createPartitionAll(%VAL(imesh), %VAL(mpi_comm_c), imeshp, ierr)
  CHECK("fail to create parallel partition ")
  options = " moab:PARALLEL=READ_PART moab:PARTITION=PARALLEL_PARTITION" // &
            " moab:PARALLEL_RESOLVE_SHARED_ENTS moab:PARTITION_DISTRIBUTE "
!            " moab:PARALLEL=READ_PART moab:PARTITION=PARALLEL_PARTITION " &
!              " moab:PARALLEL_RESOLVE_SHARED_ENTS moab:PARTITION_DISTRIBUTE ", & ! options
  if (0 .eq. rank) then
    print *, "load in parallel file HN16DP.h5m"
  endif
  lenname = 11;
  lenopt = 123
  filename = "HN16DP.h5m"
  call iMeshP_loadAll(%VAL(imesh), &
              %VAL(imeshp), &
              %VAL(root_set), &
              filename, & ! filename
              options, & !options
              ierr, &
              %VAL(lenname), & ! strlen(filename),
              %VAL(lenopt) )  !119) !strlen(options));
  CHECK("fail to load mesh in parallel ")

  isList = 0
  call iMesh_createEntSet(%VAL(imesh), %VAL(isList), opEulerSet, ierr)
  CHECK("fail to create euler set ")

  ents_alloc = 0
  ents_size = 0

  call iMesh_getEntities(%VAL(imesh),%VAL(root_set), &
   %VAL(iBase_FACE), &
   %VAL(iMesh_ALL_TOPOLOGIES), cells_ptr, &
      ents_alloc, ents_size, ierr);
  CHECK("fail to get 2d entities ")

  call c_f_pointer(cells_ptr, cells, [ents_size])

  call iMesh_addEntArrToSet(%VAL(imesh), cells, %VAL(ents_size), &
      %VAL(opEulerSet), ierr)

  call update_tracer(imesh, opEulerSet, ierr)
  CHECK("fail to update tracer ")

  outname = "outF.h5m";
  optionswrite = " moab:PARALLEL=WRITE_PART " ;
  lenname = 8
  lenopt = 27
  call iMeshP_saveAll( %VAL(imesh), &
              %VAL(imeshp), &
               %VAL(opEulerSet), &
               outname, &
               optionswrite, &
               ierr, &
               %VAL(lenname), &  ! strlen(filename),
               %VAL(lenopt) )   !119) !strlen(options));
  CHECK(" can't save ")

  if (0==rank) then
    print *, "Done"
  endif

  call MPI_FINALIZE(ierr)
  stop
end program advection

