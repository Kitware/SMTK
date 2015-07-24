! FindAdjacency: Interacting with iMesh
! 
! This program shows how to get more information about a mesh, by
! getting connectivity two different ways (as connectivity and as
! adjacent 0-dimensional entities).
  
! Usage: FindAdjacency
#define CHECK(a) \
  if (ierr .ne. 0) print *, a

program findadjacency
  implicit none
#include "iMesh_f.h"

  ! declarations
  iMesh_Instance mesh
  IBASE_HANDLE_T rpents, rpverts, rpallverts, ipoffsets
  integer ioffsets
  IBASE_HANDLE_T ents, verts, allverts, verths
  pointer (rpents, ents(0:*))
  pointer (rpverts, verts(0:*))
  pointer (rpallverts, allverts(0:*))
  pointer (ipoffsets, ioffsets(0:*))
!  for all vertices in one call
  iBase_EntityHandle verth
  pointer (verths, verth(0:*))
  integer ierr, ents_alloc, ents_size
  integer iverts_alloc, iverts_size
  integer allverts_alloc, allverts_size
  integer offsets_alloc, offsets_size, coords_alloc, coords_size
  iBase_EntitySetHandle root_set
  integer vert_uses, i, num_ents
  real*8 coords
  pointer (pcoord, coords(0:*))
!
  iBase_EntitySetHandle :: sethand

  ! create the Mesh instance
  call iMesh_newMesh("", mesh, ierr)
  CHECK("Problems instantiating interface.")

  ! load the mesh
  call iMesh_getRootSet(%VAL(mesh), root_set, ierr)
  CHECK("Problems getting root set")

  call iMesh_load(%VAL(mesh), %VAL(root_set), &
       "../../MeshFiles/unittest/125hex.g", "", ierr)
  CHECK("Load failed")

  ! get all 3d elements
  ents_alloc = 0
  call iMesh_getEntities(%VAL(mesh), %VAL(root_set), %VAL(iBase_REGION), &
       %VAL(iMesh_ALL_TOPOLOGIES), rpents, ents_alloc, ents_size, &
       ierr)
  CHECK("Couldn't get entities")

  vert_uses = 0

  ! iterate through them; 
  do i = 0, ents_size-1
     ! get connectivity
     iverts_alloc = 0
     call iMesh_getEntAdj(%VAL(mesh), %VAL(ents(i)), &
          %VAL(iBase_VERTEX), &
          rpverts, iverts_alloc, iverts_size, &
          ierr)
     CHECK("Failure in getEntAdj")
     ! sum number of vertex uses

     vert_uses = vert_uses + iverts_size

     if (iverts_size .ne. 0) call iMesh_freeMemory(%VAL(mesh), rpverts)
  end do

  ! now get adjacencies in one big block
  allverts_alloc = 0
  offsets_alloc = 0
  call iMesh_getEntArrAdj(%VAL(mesh), %VAL(rpents), &
       %VAL(ents_size), %VAL(iBase_VERTEX), rpallverts,  &
       allverts_alloc, allverts_size, ipoffsets, offsets_alloc, &
       offsets_size, ierr)
  CHECK("Failure in getEntArrAdj")

  if (allverts_size .ne. 0) call iMesh_freeMemory(%VAL(mesh), rpallverts);
  if (offsets_size .ne. 0) call iMesh_freeMemory(%VAL(mesh), ipoffsets);
  if (ents_size .ne. 0) call iMesh_freeMemory(%VAL(mesh), rpents);

  ! compare results of two calling methods
  if (allverts_size .ne. vert_uses) then
     write(*,'("Sizes didn''t agree!")')
  else 
     write(*, *)"Sizes did agree: ", vert_uses
  endif
! get all vertices , and then their coordinates
  ents_alloc = 0
  call iMesh_getEntities(%VAL(mesh), %VAL(root_set), %VAL(iBase_VERTEX), &
       %VAL(iMesh_ALL_TOPOLOGIES), verths, ents_alloc, ents_size, &
       ierr)
  write (*, *) "number of vertices: " , ents_size
  print *, "few vertex handles: ", (verth(i), i=0,ents_size/10)

! set creation
  call iMesh_createEntSet(%VAL(mesh), %VAL(1), &
        sethand,ierr)
  write(0,*) "createset",ierr,sethand 

! we should have 
  call iMesh_addEntArrToSet(%VAL(mesh),verth,%VAL(ents_size), &
      %VAL(sethand),ierr)
  write(0,*) "add Ent Arr to Set",ierr,sethand 

  call iMesh_getNumOfType(%VAL(mesh), %VAL(sethand),  &
   %VAL(iBase_VERTEX), num_ents, ierr)
  write(0,*) "num verts retrieved from set", num_ents

  ents_alloc = 0;
  call iMesh_getVtxArrCoords(%VAL(mesh), verth, %VAL(ents_size), &
    %VAL(iBase_INTERLEAVED), pcoord, ents_alloc , ents_size, ierr)
!
  write(*, *) "num coords: ", ents_size, " few coords: ", (coords(i), i=0, ents_size/100)  

  call iMesh_freeMemory(%VAL(mesh), verths);
  call iMesh_freeMemory(%VAL(mesh), pcoord);

  call iMesh_dtor(%VAL(mesh), ierr)
  CHECK("Failed to destroy interface")

end program findadjacency
