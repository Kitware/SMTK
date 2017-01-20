#ifndef IMESHP_REC_CBIND_H__
#define IMESHP_REC_CBIND_H__

#include "iMeshP.h"
#include "iMeshP_protos.h"
#include "moab_mpi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Convert a fortran to C communicator
 * Given a Fortran communicator, convert to a C communicator that can be passed back to iMeshP.
 * Though this is an iMeshP function, it doesn't take an iMeshP Partition handle, since the (C)
 * communicator is needed by the function that creates the partition.
 *  COMMUNICATION:  None.
 * 
 *  \param  instance         (In)  Mesh instance to contain the partition.
 *  \param  fcomm            (In)  Pointer to fortran communicator
 *  \param  ccomm            (Out) Pointer to the C communicator
 *  \param  err              (Out) Error code.
 */
    void iMeshP_getCommunicator(
        iMesh_Instance instance,
        int *fcomm,
        MPI_Comm *ccomm,
        int *err);

/** \brief Assign a global id space to entities
 * Assign a global id space to entities and vertices, and optionally intermediate-dimension entities
 *
 *  COMMUNICATION:  Collective.
 */
    void iMeshP_assignGlobalIds(
        iMesh_Instance instance,
        const iMeshP_PartitionHandle partition,
        const iBase_EntitySetHandle this_set,
        const int dimension,
        const int start_id,
        const int largest_dim_only,
        const int parallel,
        const int owned_only,
        int *err);


#ifdef __cplusplus
}
#endif

#endif

