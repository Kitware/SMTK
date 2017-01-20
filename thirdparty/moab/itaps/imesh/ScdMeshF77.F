! MOAB structured mesh extension test
! 
! This test also tests fortran free-source format
!

#define ERROR(rval) if (0 .ne. rval) call exit(1)

      real function reinterpret_ptr(xm, ni, nj, nk)
      integer ni, nj, nk
      real xm(ni, nj, nk)

      reinterpret_ptr = 0.0
      do 10 k = 1, nk
         do 10 j = 1, nj
            do 10 i = 1, ni
               reinterpret_ptr = reinterpret_ptr + xm(i, j, k)
 10         continue
      return
      end 

      program ScdMesh
      implicit none
      integer comm1, mysize,myproc,ier
#include "iMesh_f.h"
      iMesh_Instance mesh
      iBase_EntitySetHandle handle
      iBase_EntityHandle root_set
      iBase_TagHandle tagh
      iBase_EntityArrIterator iter
      integer local_dims(6),global_dims(6)
      integer geom_dim,num_regions, num_verts, num_quads, count
      integer*8 rpxm1
      integer i
      real rsum
      !real, dimension(:) :: xm
      real xm
      pointer (rpxm1, xm(*))
      real reinterpret_ptr
      
      ! declarations
      
      ! create the Mesh instance
      
      local_dims(1)=0
      local_dims(2)=0
      local_dims(3)=-1
      local_dims(4)=64
      local_dims(5)=64
      local_dims(6)=-1
      
      global_dims(1)=0
      global_dims(2)=0
      global_dims(3)=-1
      global_dims(4)=64
      global_dims(5)=64
      global_dims(6)=-1
      
      call iMesh_newMesh('MOAB', mesh, ier)
      ERROR(ier)
      
      handle = 0
      call iMesh_createStructuredMesh(%VAL(mesh), local_dims, 
     1  global_dims, %VAL(0),%VAL(0),%VAL(0), %VAL(1), %VAL(-1),
     1  %VAL(-1), %VAL(-1), %VAL(0), %VAL(1), %VAL(1), handle, ier)
      ERROR(ier)

      call iMesh_getRootSet(%VAL(mesh), root_set, ier)
      ERROR(ier)
      
      call iMesh_getGeometricDimension(%VAL(mesh), geom_dim, ier)
      ERROR(ier)

      call iMesh_getNumOfType(%VAL(mesh), %VAL(root_set), 
     1  %VAL(iBase_FACE), num_quads, ier)
      ERROR(ier)
      
      call iMesh_getNumOfType(%VAL(mesh), %VAL(root_set), 
     1  %VAL(iBase_VERTEX), num_verts, ier)
      ERROR(ier)
      
      
      call iMesh_initEntArrIter(%VAL(mesh), %VAL(root_set), 
     1  %VAL(iBase_FACE), %VAL(iMesh_QUADRILATERAL),%VAL(num_quads), 
     1  %VAL(0), iter, ier) 
      
      call iMesh_createTagWithOptions(%VAL(mesh), "XM1", 
     1  "moab:TAG_STORAGE_TYPE=DENSE; moab:TAG_DEFAULT_VALUE=0.0",
     1  %VAL(5), %VAL(iBase_DOUBLE), tagh, ier)
      
      call iMesh_tagIterate(%VAL(mesh),%VAL(tagh),%VAL(iter),rpxm1,
     1  count,ier)
      
      call iMesh_endEntArrIter(%VAL(mesh), %VAL(iter), ier)
      ERROR(ier)

      do 20 i = 1, 5*64*64
        xm(i) = 1.0
 20     continue

      rsum = reinterpret_ptr(xm, 5, 64, 64)

      call iMesh_dtor(%VAL(mesh), ier)
      ERROR(ier)

      if (rsum .ne. 5*64*64) call exit(1)
      
      call exit(0)
      end
