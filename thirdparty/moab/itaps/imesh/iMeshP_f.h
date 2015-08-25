#ifndef IMESHP_F_H
#define IMESHP_F_H

#define iMeshP_PartitionHandle IBASE_HANDLE_T
#define iMeshP_PartHandle IBASE_HANDLE_T

#endif 

#include "iMesh_f.h"

      integer iMeshP_LOCAL
      integer iMeshP_REMOTE
      integer iMeshP_INVALID

      integer iMeshP_INTERNAL
      integer iMeshP_BOUNDARY
      integer iMeshP_GHOST

      parameter (iMeshP_LOCAL = 0)
      parameter (iMeshP_REMOTE = 1)
      parameter (iMeshP_INVALID = 2)

      parameter (iMeshP_INTERNAL = 0)
      parameter (iMeshP_BOUNDARY = 1)
      parameter (iMeshP_GHOST = 2)
