
#ifndef _ITAPS_iMeshP
#define _ITAPS_iMeshP

#include "imesh_export.h"

#include "iMesh.h"
#include "iMeshP_protos.h"
#include "moab_mpi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Handles needed in iMeshP */
typedef struct iMeshP_PartitionHandle_Private* iMeshP_PartitionHandle;
typedef struct iMeshP_RequestHandle_Private* iMeshP_RequestHandle;

/* Since we allow overloading of iMesh functions' entity set handles with
 * part handles, iMeshP_PartHandle must be defined the same as 
 * iBase_EntitySetHandle. */
typedef iBase_EntitySetHandle iMeshP_PartHandle;

typedef unsigned iMeshP_Part;

/** Types for classifying entities within a part. */
enum iMeshP_EntStatus 
{
  iMeshP_INTERNAL, /**< An owned entity that is not on a part boundary. */
  iMeshP_BOUNDARY, /**< A shared entity on a part boundary. */
  iMeshP_GHOST     /**< An entity copy that is not a shared boundary entity. */
};

/** Part ID number indicating information should be returned about all parts. */
#define iMeshP_ALL_PARTS -1


/** \page imeshp  iMeshP: ITAPS Parallel Mesh Interface
iMeshP.h -- ITAPS Parallel Mesh Interface

Release 0.1; October 2008

\section ADM Abstract Data Model
-  The term "mesh" refers to an abstraction in the data model; 
   it does not imply a serial or parallel distribution.
-  The term "partition" refers to an assignment of a set of entities to 
   subsets; like a "mesh," it does not imply a serial or parallel 
   implementation.
-  An application may use one or more meshes.  
-  Partitions can create subsets of entities from one or more meshes.
-  Meshes can be subdivided by one or more partitions.
-  Partitions contain parts.  Parts contain the subsets of entities in the
   partition.

\section PAR Parallelism
-  A "process" can be thought of as an MPI process. The
   number of processes can be considered to be the result of MPI_Comm_size.
   The rank of a process can be thought of as the result of MPI_Comm_rank.
   We will think in terms of processes rather than processors.  Initial
   implementations of the parallel interface will likely use MPI terminology
   directly; future implementations may accommodate other communication 
   paradigms and libraries.
-  Partitions have communicators associated with them.  These communicators
   can be thought of as MPI communicators.  
-  "Global" operations are operations performed with respect to a 
   partition's communicator.
-  "Local" operations are operations performed with respect to a part or
   a mesh instance within a process.
-  Part A "neighbors" Part B if Part A has copies of entities owned by Part B
   and/or if Part B has copies of entities owned by Part A.
                  
\section INT Interfaces
-  Each process has one or more "mesh instances."  A mesh instance can be
   thought of as a mesh database.  An implementation should support the 
   existence of more than one mesh instance per process (e.g., it should 
   always associate mesh data with a mesh instance).  However, we expect 
   applications would most often use only one mesh instance per process.
-  There is one root set per mesh instance.
-  Each process may have one or more partition handles.
-  A partition assigns entities from one mesh instance to parts.  
-  Entities in a mesh instance can be partitioned by one or more partitions.  
   Mesh instances know which partitions they contain.
-  Parts are uniquely identified globally by part IDs of type iMeshP_Part.
   Local parts can also be accessed by part handles that provide more
   direct access to a part.  
   Functions accepting part handles operate correctly on only local 
   parts (parts on the calling process); they will return an error 
   for remote (off-process) parts.  
-  Generation and management of global IDs for entities 
   is not included in the iMeshP interface.  It can 
   be provided as a service above the iMeshP interface.
   Uniqueness of global IDs is managed at the partition level.

\section PRT Using Parts
-  Each part is wholly contained within a process.  
-  A process may have zero, one or multiple parts.
-  For each entity that is copied onto remote parts, the owning part knows 
   both the remote part ID and remote entity handle of all copies.
-  All parts with copies of a boundary entity know the remote part ID 
   and remote entity handle of all copies of the entity.  
-  All parts with copies of any entity know the part ID and
   entity handle corresponding to the owner of the entity.
-  Functions that return entity information for a part, set or mesh 
   instance return the information for all entities (including copies and
   ghosts) in that part, set or mesh instance.  Applications can check 
   whether an entity is owned or a ghost using iMeshP_isEntOwner or
   iMeshP_getEntStatus.
-  Many iMesh functions that accept an iBase_EntitySetHandle 
   are also useful in the context of a iMeshP_PartHandle.
   These functions are reinterpreted so that they can accept either an
   iBase_EntitySetHandle or an iMeshP_PartHandle.  
-  In particular, entities are added to and removed from local parts via
   the same functions that are used to manipulate entity sets.
   That is, given a mesh instance, an entity handle, and a part handle,
   the entity is added to or removed from the part via calls to 
   the following functions with the part handle passed as the entity set handle:
   - Add entity to part --> iMesh_addEntToSet
   - Remove entity from part --> iMesh_rmvEntFromSet
   - Add array of entities to part --> iMesh_addEntArrToSet
   - Remove array of entities from part --> iMesh_rmvEntArrFromSet

\section CMM Communication
-  Each function description includes its communication requirements.  The
   options are described here:
   -  COMMUNICATION:  Collective -- the function must be called by all 
      processes in the partition's communicator.  (These functions have the
      suffix "All" to indicate collective communication is done.)
   -  COMMUNICATION:  Point-to-Point -- communication is used, but the 
      communication is from one process to only one other process.  The
      receiving process must issue an appropriate receive call to receive 
      the message.
   -  COMMUNICATION:  None -- the function does not use communication; only
      local operations are performed.
   -  COMMUNICATION:  None++ -- no communication is done; the values
      are precomputed by iMeshP_syncPartitionAll or iMeshP_syncMeshAll.
-  Non-blocking calls for off-processor mesh-modification return a request 
   that indicates whether or not the operation has completed.  The request
   is more than an MPI request; it encapsulates both the MPI information and
   the mesh operations that were requested.  If non-blocking calls are used,
   appropriate calls to iMeshP "wait" or "poll" functions must be used to
   handle and satisfy requests.
*/

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*                          Partition Functionality                       */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/** \brief Create a partition; return its handle.
 * 
 *  Given a mesh instance and a communicator,
 *  return a partition handle for a new partition within the mesh instance
 *  that uses the communicator.  
 *  In the future, we may have different creation routines for different 
 *  communication systems; once the partition is created, the application 
 *  would not have to worry about the communication system again.
 *  For now, implementations are MPI based, so MPI communicators are provided.
 *  For serial use, the communicator may be MPI_COMM_SELF or communicator may
 *  be NULL.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance         (In)  Mesh instance to contain the partition.
 *  \param  communicator     (In)  Communicator to be used for parallel 
 *                                 communication.
 *  \param  partition        (Out) The newly created partition.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_createPartitionAll(
            iMesh_Instance instance,
            MPI_Comm communicator,
            iMeshP_PartitionHandle *partition,
            int *err);


 
/**  \brief Destroy a partition. 
 *
 *  Given a partition handle, 
 *  destroy the partition associated with the handle.
 *  Note that the partition handle is not invalidated upon return.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition to be destroyed.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_destroyPartitionAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int *err);



/**  \brief Return communicator associated with a partition.
 *
 *  Given a partition handle, return the communicator associated with
 *  it during its creation by iMeshP_createPartitionAll.
 *
 *  COMMUNICATION:  None
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  communicator     (Out) Communicator associated with the partition.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getPartitionComm(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            MPI_Comm *communicator,
            int *err);
    


/**  \brief Update a partition after parts have been added.
 * 
 *  This function gives the implementation an opportunity to locally store info
 *  about the partition so that queries on the partition can be 
 *  performed without synchronous communication. 
 *  This function must be called after all parts have been added to the
 *  partition and after changes to the partition (e.g., due to load balancing).
 *  Values that are precomputed by syncPartitionAll include:
 *  -  the total number of parts in a partition;
 *  -  the mapping between part IDs and processes; and
 *  -  updated remote entity handle information.
 *
 *  COMMUNICATION:  Collective.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being updated.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_syncPartitionAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int *err); 



/**  \brief Return the number of partitions associated with a mesh instance.
 *
 *  Given a mesh instance, return the number of partition handles
 *  associated with the mesh instance.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance         (In)  Mesh instance containing the partitions.
 *  \param  num_partitions   (Out) Number of partitions associated with the
 *                                 mesh instance.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumPartitions(
            iMesh_Instance instance,
            int *num_partitions,
            int *err);



/**  \brief Return the partition handles associated with a mesh instance.
 *
 *  Given a mesh instance, return all partition handles
 *  associated with the mesh instance.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                    (In)     Mesh instance containing the 
 *                                               partitions.
 *  \param  partitions                  (In/Out) Array of partition handles 
 *                                               associated with the mesh 
 *                                               instance.
 *  \param  partitions_allocated        (In/Out) Allocated size of 
 *                                               partitions array.
 *  \param  partitions_size             (Out)    Occupied size of 
 *                                               partitions array.
 *  \param  err                         (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getPartitions(
            iMesh_Instance instance,
            iMeshP_PartitionHandle **partitions,
            int *partitions_allocated, 
            int *partitions_size, 
            int *err); 



/** \brief Return the global number of parts in a partition.
 *
 *  Given a partition handle, return the total number of parts 
 *  in the partition across all processes in the partition's communicator.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  num_global_part  (Out) Global number of parts in the partition.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumGlobalParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            int *num_global_part, 
            int *err); 



/** \brief Return the local number of parts in a partition.
 *
 *  Given a partition handle, return the number of local (on-process) parts 
 *  in the partition.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  num_local_part   (Out) Local (on-process) number of parts in 
 *                                 the partition.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumLocalParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            int *num_local_part, 
            int *err); 



/** \brief Return the part handles of local parts in a partition.
 * 
 *  Given a partition handle, return the 
 *  part handles for the local (on-process) parts in the partition.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance               (In)     Mesh instance containing the 
 *                                          partition.
 *  \param  partition              (In)     The partition being queried.
 *  \param  parts                  (In/Out) Array of part handles 
 *                                          for local parts in the partition.
 *  \param  parts_allocated        (In/Out) Allocated size of 
 *                                          parts array.
 *  \param  parts_size             (Out)    Occupied size of 
 *                                          parts array.
 *  \param  err                    (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getLocalParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_PartHandle **parts,
            int *parts_allocated,
            int *parts_size,
            int *err); 



/**  \brief Return the process rank of a given part.
 *
 *  Given a partition handle and a part ID, return the process rank 
 *  (with respect to the partition's communicator) of the 
 *  process that owns the part. The part may be local or remote.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance         (In)  Mesh instance containing the partition.
 *  \param  partition        (In)  The partition being queried.
 *  \param  part_id          (In)  Part ID for the part being queried.
 *  \param  rank             (Out) Process rank of part_id.
 *  \param  err              (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getRankOfPart(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part part_id,
            int *rank,
            int *err); 



/**  \brief Return the process ranks of given parts.
 *
 *  Given a partition handle and an array of part IDs, return the process ranks 
 *  (with respect to the partition's communicator) of the 
 *  process that owns each part. The parts may be local or remote.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance         (In)     Mesh instance containing the partition.
 *  \param  partition        (In)     The partition being queried.
 *  \param  part_ids         (In)     Array of Part IDs for the parts being 
 *                                    queried.
 *  \param  part_ids_size    (In)     The number of Part IDs in part_ids.
 *  \param  ranks            (In/Out) Array of ranks for the Part Ids in 
 *                                    part_ids.
 *  \param  ranks_allocated  (In/Out) Allocated size of ranks array.
 *  \param  ranks_size       (Out)    Occupied size of ranks array.
 *  \param  err              (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getRankOfPartArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part *part_ids,
            const int part_ids_size,
            int **ranks, 
            int *ranks_allocated, 
            int *ranks_size,
            int *err); 



/** \brief  Return the number of entities of a given type in a partition.
 * 
 *  Given a partition handle and an entity set (possibly the root set), 
 *  return the global number of  entities of a 
 *  given entity type in the partition and set.  This function may require 
 *  communication and, thus, must be called by all processes in the partition's 
 *  communicator.
 * 
 *  COMMUNICATION:  Collective.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set
 *                                  being queried.
 *  \param  entity_type       (In)  Requested entity type;
 *                                  may be iBase_ALL_TYPES.
 *  \param  num_type          (Out) Number of entities of entity_type in
 *                                  the partition and entity set.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumOfTypeAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            int entity_type, 
            int *num_type, 
            int *err);



/** \brief  Return the number of entities of a given topology in a partition.
 * 
 *  Given a partition handle and an entity set (possibly the root set), 
 *  return the global number of  entities of a 
 *  given entity topology in the partition and set.  This function may require 
 *  communication and, thus, must be called by all processes in the partition's 
 *  communicator.
 * 
 *  COMMUNICATION:  Collective.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set
 *                                  being queried; may be the root set.
 *  \param  entity_topology   (In)  Requested entity topology;
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  num_topo          (Out) Number of entities with entity_topology in
 *                                  the partition and entity set.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumOfTopoAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            int entity_topology, 
            int *num_topo, 
            int *err);


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*                        Part Functionality                              */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/** \brief Create a new part in a partition.
 *
 *  Given a partition handle, create a new part and add it to the
 *  partition on the process invoking the creation.  Return the part handle
 *  for the new part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being updated.
 *  \param  part              (Out) The newly created part.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_createPart(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_PartHandle *part,
            int *err);
 


/** \brief  Remove a part from a partition.
 *
 *  Given a partition handle and a part handle, remove the part
 *  from the partition and destroy the part.  Note that the part handle
 *  is not invalidated by this function.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being updated.
 *  \param  part              (In)  The part to be removed.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_destroyPart(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_PartHandle part,
            int *err);



/** \brief Obtain a part ID from a part handle.
 *
 *  Given a partition handle and a local part handle, return the part ID.
 *  If the part handle is not a valid part handle for a local part,
 *  an error is returned.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  part_id           (Out) Part ID for part.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getPartIdFromPartHandle(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            iMeshP_Part *part_id,
            int *err);




/** \brief Obtain part IDs from part handles.
 *
 *  Given a partition handle and an array of local part handles, 
 *  return the part ID for each part handle.
 *  If any part handle is not a valid part handle for a local part,
 *  an error is returned.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance            (In)     Mesh instance containing the partition.
 *  \param  partition           (In)     The partition being queried.
 *  \param  parts               (In)     Array of part handles for the parts 
 *                                       being queried.
 *  \param  parts_size          (In)     Number of part handles being queried.
 *  \param  part_ids            (In/Out) Array of part IDs associated with the 
 *                                       parts.
 *  \param  part_ids_allocated  (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size       (Out)    Occupied size of part_ids array.
 *  \param  err                 (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getPartIdsFromPartHandlesArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle *parts,
            const int parts_size,
            iMeshP_Part **part_ids,
            int *part_ids_allocated,
            int *part_ids_size,
            int *err);



/** \brief Obtain a part handle from a part ID.
 *
 *  Given a partition handle and a part ID, return the part handle 
 *  associated with the part
 *  if the part is local; otherwise, return an error code.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part_id           (In)  Part ID for the part being queried.
 *  \param  part              (Out) Part handle associated with part_id.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getPartHandleFromPartId(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_Part part_id,
            iMeshP_PartHandle *part,
            int *err);




/** \brief Obtain part handles from part IDs.
 *
 *  Given a partition handle and an array of local part IDs, 
 *  return the part handle for each part ID.
 *  If any part ID is not a valid part ID for a local part,
 *  an error is returned.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  part_ids                (In)     Array of part IDs for the parts 
 *                                           being queried.
 *  \param  part_ids_size           (In)     Number of part IDs being queried.
 *  \param  parts                   (In/Out) Array of part handles associated 
 *                                           with the part_ids.
 *  \param  parts_allocated         (In/Out) Allocated size of parts 
 *                                           array.
 *  \param  parts_size              (Out)    Occupied size of parts 
 *                                           array.
 *  \param  err                     (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getPartHandlesFromPartsIdsArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part *part_ids,
            const int part_ids_size,
            iMeshP_PartHandle **parts,
            int *parts_allocated,
            int *parts_size,
            int *err);




/*------------------------------------------------------------------------*/
/*                        Part Boundaries                                 */
/*------------------------------------------------------------------------*/

/** \brief Return the number of parts that neighbor a given part.
 *
 *  Given a partition handle, a part handle, and an entity type, 
 *  return the number of parts in the partition that neighbor the given part
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the copied entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  num_part_nbors    (Out) Number of parts neighboring the given part.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumPartNbors(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            int entity_type,
            int *num_part_nbors,
            int *err); 



/** \brief Return the number of parts that neighbor given parts.
 *
 *  Given a partition handle, an array of part handles, and an entity type, 
 *  return the number of parts in the partition that neighbor each of the 
 *  given parts
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                  (In)     Mesh instance containing the 
 *                                             partition.
 *  \param  partition                 (In)     The partition being queried.
 *  \param  parts                     (In)     Array of part handles for the 
 *                                             parts being queried.
 *  \param  parts_size                (In)     Number of part handles in 
 *                                             parts.
 *  \param  entity_type               (In)     Entity type of the copied
 *                                             entities;
 *                                             may be iBase_ALL_TYPES.
 *  \param  num_part_nbors            (In/Out) Array of values specifying the 
 *                                             number of part neighbors for 
 *                                             each part in parts.
 *  \param  num_part_nbors_allocated  (In/Out) Allocated size of num_part_nbors 
 *                                             array.
 *  \param  num_part_nbors_size       (Out)    Occupied size of num_part_nbors 
 *                                             array.
 *  \param  err                       (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getNumPartNborsArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle *parts,
            int parts_size,
            int entity_type,
            int **num_part_nbors,
            int *num_part_nbors_allocated,
            int *num_part_nbors_size,
            int *err); 



/** \brief Return the parts that neighbor a given part.
 *
 *  Given a partition handle, a part handle, and an entity type, 
 *  return the part IDs of parts that neighbor the given part
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entity_type              (In)     Entity type of the copied
 *                                            entities; 
 *                                            may be iBase_ALL_TYPES.
 *  \param  num_part_nbors           (Out)    Number of parts neighboring
 *                                            the given part.
 *  \param  nbor_part_ids            (In/Out) Array of part IDs for 
 *                                            part neighbors of part.
 *  \param  nbor_part_ids_allocated  (In/Out) Allocated size of nbor_part_ids 
 *                                            array.
 *  \param  nbor_part_ids_size       (Out)    Occupied size of nbor_part_ids 
 *                                            array.
 *  \param  err                      (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getPartNbors(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            int entity_type,
            int *num_part_nbors,
            iMeshP_Part **nbor_part_ids,
            int *nbor_part_ids_allocated,
            int *nbor_part_ids_size,
            int *err); 



/** \brief Return the parts that neighbor given parts.
 *
 *  Given a partition handle, an array of part handles, and an entity type, 
 *  return the part IDs of parts that neighbor the given parts
 *  (i.e., that (1) have copies of entities of the given entity type owned by 
 *  the given part or (2) own entities of the given entity type that are 
 *  copied on the given part).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  parts                    (In)     The parts being queried.
 *  \param  parts_size               (In)     The number of parts being queried.
 *  \param  entity_type              (In)     Entity type of the copied 
 *                                            entities;
 *                                            may be iBase_ALL_TYPES.
 *  \param  num_part_nbors           (In/Out) Array of values specifying the 
 *                                            number of part neighbors for 
 *                                            each part in parts.
 *  \param  num_part_nbors_allocated (In/Out) Allocated size of num_part_nbors 
 *                                            array.
 *  \param  num_part_nbors_size      (Out)    Occupied size of num_part_nbors 
 *                                            array.
 *  \param  nbor_part_ids            (In/Out) Array of part IDs for 
 *                                            part neighbors of part.
 *  \param  nbor_part_ids_allocated  (In/Out) Allocated size of nbor_part_ids 
 *                                            array.
 *  \param  nbor_part_ids_size       (Out)    Occupied size of nbor_part_ids 
 *                                            array.
 *  \param  err                      (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getPartNborsArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle *parts,
            const int parts_size,
            int entity_type,
            int **num_part_nbors,
            int *num_part_nbors_allocated,
            int *num_part_nbors_size,
            iMeshP_Part **nbor_part_ids,
            int *nbor_part_ids_allocated,
            int *nbor_part_ids_size,
            int *err); 



/** \brief Return the number of entities on a part boundary.
 *
 *  Given a partition handle, a part handle, an entity type and topology, and a
 *  target part ID, return the number of entities of the given type and/or
 *  topology on the part boundary shared with the target part.  
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities; 
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  target_part_id    (In)  Part ID with which part is sharing
 *                                  the boundary entities; may be 
 *                                  iMeshP_ALL_PARTS.
 *  \param  num_entities      (Out) Number of part boundary entities shared
 *                                  by part and target_part_id.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumPartBdryEnts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            iMeshP_Part target_part_id, 
            int *num_entities, 
            int *err); 



/** \brief Return the entity handles of entities on a part boundary.
 *
 *  Given a partition handle, a part handle, an entity type and topology, and a
 *  target part ID, return the entity handles of entities of the given type 
 *  and/or topology on the part boundary shared with the target part.  
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entity_type              (In)     Entity type of the boundary 
 *                                            entities;
 *                                            may be iBase_ALL_TYPES.
 *  \param  entity_topology          (In)     Entity topology of the boundary 
 *                                            entities;
 *                                            may be iMesh_ALL_TOPOLOGIES.
 *  \param  target_part_id           (In)     Part ID with which part        
 *                                            is sharing the boundary entities;
 *                                            may be iMeshP_ALL_PARTS.
 *  \param  entities                 (In/Out) Array of entity handles for 
 *                                            entities on the part boundary
 *                                            between part and 
 *                                            target_part_id.
 *  \param  entities_allocated       (In/Out) Allocated size of entities 
 *                                            array.
 *  \param  entities_size            (Out)    Occupied size of entities 
 *                                            array.
 *  \param  err                      (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getPartBdryEnts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            iMeshP_Part target_part_id, 
            iBase_EntityHandle **entities,
            int *entities_allocated,
            int *entities_size, 
            int *err); 



/** \brief Initialize an iterator over a specified part boundary.
 *
 *  Given a partition handle, a part handle, and a 
 *  target part ID, return an iterator over all entities of a given
 *  entity type and topology along
 *  the part boundary shared with the target part.  
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntIter, iMesh_resetEntIter, and iMesh_endEntIter,
 *  respectively.  
 *
 *  COMMUNICATION:  None.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities; 
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  target_part_id    (In)  Part ID with which part is sharing
 *                                  the boundary entities; may be 
 *                                  iMeshP_ALL_PARTS.
 *  \param  entity_iterator   (Out) Iterator returned by the function.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_initPartBdryEntIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            iMeshP_Part target_part_id, 
            iBase_EntityIterator* entity_iterator, 
            int *err); 



/** \brief Initialize an array iterator over a specified part boundary.
 *
 *  Given a partition handle, a part handle, and a 
 *  target part ID, return an array iterator over all entities of a given
 *  entity type and topology along
 *  the part boundary shared with the target part.  
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntArrIter, iMesh_resetEntArrIter, and iMesh_endEntArrIter,
 *  respectively.  
 *
 *  COMMUNICATION:  None.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities; 
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  array_size        (In)  Size of chunks of handles returned for 
 *                                  each value of the iterator.
 *  \param  target_part_id    (In)  Part ID with which part is sharing
 *                                  the boundary entities; may be 
 *                                  iMeshP_ALL_PARTS.
 *  \param  entity_iterator   (Out) Iterator returned by the function.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_initPartBdryEntArrIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part, 
            int entity_type, 
            int entity_topology, 
            int array_size, 
            iMeshP_Part target_part_id, 
            iBase_EntityArrIterator* entity_iterator, 
            int *err); 


/*------------------------------------------------------------------------*/
/*                        Parts and Sets                                  */
/*------------------------------------------------------------------------*/

/**  \brief Return the number of entities of a given type in both a part and an entity set.
 *
 *  Given a part handle, an entity set handle, and an entity type, return
 *  the number of entities of the given type that are in BOTH the given
 *  part AND the given entity set.
 *  This function is similar to iMesh_getNumOfType, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set 
 *                                  being queried; may be the root set.
 *  \param  entity_type       (In)  Entity type of the boundary entities;
 *                                  may be iBase_ALL_TYPES.
 *  \param  num_type          (Out) Number of entities of entity_type in
 *                                  both part and entity_set.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumOfType(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_type, 
            int *num_type, 
            int *err);



/**  \brief Return the number of entities of a given topology in both a part and an entity set.
 *
 *  Given a part handle, an entity set handle, and an entity topology, return
 *  the number of entities of the given topology that are in BOTH the given
 *  part AND the given entity set.
 *  This function is similar to iMesh_getNumOfTopo, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part being queried.
 *  \param  entity_set        (In)  Entity set handle for the entity set 
 *                                  being queried; may be the root set.
 *  \param  entity_topology   (In)  Entity topology of the boundary entities;
 *                                  may be iMesh_ALL_TOPOLOGIES.
 *  \param  num_topo          (Out) Number of entities of entity_topology in
 *                                  both part and entity_set.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumOfTopo(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_topology, 
            int *num_topo, 
            int *err);

/**\brief Get indexed representation of mesh or subset of mesh
 *
 * Given part handle and an entity set and optionally a type or topology, 
 * for all entities that are in BOTH the part and the entity set, return:
 * - The entities in the part and set of the specified type or topology
 * - The entities adjacent to those entities with a specified
 *    type, as a list of unique handles.
 * - For each entity in the first list, the adjacent entities,
 *    specified as indices into the second list.
 *
 *  COMMUNICATION:  None.
 *
 *\param  instance                (In)     Mesh instance containing the 
 *                                         partition.
 *\param  partition               (In)     The partition being queried.
 *\param  part                    (In)     The part being queried.
 *\param entity_set_handle        (In)     The set being queried
 *\param entity_type_requestor    (In)     If not iBase_ALL_TYPES, act only 
 *                                         on the subset of entities with
 *                                         the specified type.
 *\param entity_topology_requestor (In)    If not iMesh_ALL_TOPOLOGIES, act 
 *                                         only on the subset of entities with
 *                                         the specified topology.
 *\param entity_type_requested    (In)     The type of the adjacent entities
 *                                         to return.
 *\param entity_handles           (In/Out) The handles of the (non-strict) 
 *                                         subset of the union of the part
 *                                         and entity set, and the optional 
 *                                         type and topology filtering 
 *                                         arguments.
 *\param adj_entity_handles       (In/Out) The union of the entities of type 
 *                                         'requested_entity_type' adjacent 
 *                                         to each entity in 'entity_handles'.
 *\param adj_entity_indices       (In/Out) For each entity in 'entity_handles', 
 *                                         the adjacent entities of type
 *                                         'entity_type_requested', specified as 
 *                                         indices into 'adj_entity_handles'.  
 *                                         The indices are concatenated into a 
 *                                         single array in the order of the 
 *                                         entity handles in 'entity_handles'.
 *\param offset                   (In/Out) For each entity in the 
 *                                         corresponding position in 
 *                                         'entity_handles', the position
 *                                         in 'adj_entity_indices' at which
 *                                         values for that entity are stored.
 */
IMESH_EXPORT
void iMeshP_getAdjEntIndices(iMesh_Instance instance,
                             iMeshP_PartitionHandle partition,
                             iMeshP_PartHandle part,
                             iBase_EntitySetHandle entity_set_handle,
                             int entity_type_requestor,
                             int entity_topology_requestor,
                             int entity_type_requested,
                             iBase_EntityHandle** entity_handles,
                             int* entity_handles_allocated,
                             int* entity_handles_size,
                             iBase_EntityHandle** adj_entity_handles,
                             int* adj_entity_handles_allocated,
                             int* adj_entity_handles_size,
                             int** adj_entity_indices,
                             int* adj_entity_indices_allocated,
                             int* adj_entity_indices_size,
                             int** offset,
                             int* offset_allocated,
                             int* offset_size,
                             int *err);

/** \brief Return entities in a both given part and entity set.
 *
 *  Given an entity set handle 
 *  and a part handle, return entity handles for entities
 *  that are in both the part and the entity set.
 *  This function is similar to iMesh_getEntities, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entity_set               (In)     Entity set handle for the 
 *                                            entity set being queried; 
 *                                            may be the root set.
 *  \param  entity_type              (In)     Entity type of the
 *                                            entities;
 *                                            may be iBase_ALL_TYPES.
 *  \param  entity_topology          (In)     Entity topology of the 
 *                                            entities;
 *                                            may be iMesh_ALL_TOPOLOGIES.
 *  \param  entities                 (In/Out) Array of entity handles for
 *                                            entities in both part       
 *                                            and entity_set.
 *  \param  entities_allocated       (In/Out) Allocated size of entities.
 *  \param  entities_size            (Out)    Occupied size of entities.
 *  \param  err                      (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getEntities(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_type,
            int entity_topology,
            iBase_EntityHandle **entities,
            int *entities_allocated,
            int *entities_size,
            int *err);

/** \brief Return entities adjacent to entities in a given part and entity set.
 *
 *  Given an entity set handle 
 *  and a part handle, return entities adjacent (with respect to a given
 *  entity type and/or topology) to entities
 *  that are in both the part and the entity set.
 *  This function is similar to iMesh_getAdjEntities, but it also restricts
 *  the returned data with respect to its existence in the given part.
 *  If a non-root entity set is specified, the function also returns
 *  flags indicating whether each adjacent entity 
 *  is in the entity set; (*in_entity_set)[i]=1 indicates that adjacent entity
 *  (*adj_entities)[i] is in the specified entity set.  
 *  Array entry offset[i] stores the index of first adjacent entity to 
 *  entity i.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                     (In)     Mesh instance containing the 
 *                                                partition.
 *  \param  partition                    (In)     The partition being queried.
 *  \param  part                         (In)     The part being queried.
 *  \param  entity_set                   (In)     Entity set handle for the 
 *                                                entity set being queried; 
 *                                                may be the root set.
 *  \param  entity_type_requestor        (In)     Return entities adjacent to 
 *                                                entities of this type;
 *                                                may be iBase_ALL_TYPES.
 *  \param  entity_topology_requestor    (In)     Return entities adjacent to
 *                                                entities of this topology;
 *                                                may be iMesh_ALL_TOPOLOGIES.
 *  \param  entity_type_requested        (In)     Return adjacent entities of 
 *                                                this type;
 *                                                may be iBase_ALL_TYPES.
 *  \param  adj_entities                 (In/Out) Array of adjacent entity 
 *                                                handles returned.
 *  \param  adj_entities_allocated       (In/Out) Allocated size of 
 *                                                adj_entities.
 *  \param  adj_entities_size            (Out)    Occupied size of 
 *                                                adj_entities.
 *  \param  offset                       (In/Out) Array of offsets returned.
 *  \param  offset_allocated             (In/Out) Allocated size of offset.
 *  \param  offset_size                  (Out)    Occupied size of offset.
 *  \param  in_entity_set                (In/Out) Array of flags returned if 
 *                                                non-root entity set was input;
 *                                                (*in_entity_set)[i]=1 
 *                                                indicates
 *                                                (*adj_entities)[i] 
 *                                                is in the entity set.
 *  \param  in_entity_set_allocated      (In/Out) Allocated size of 
 *                                                in_entity_set.
 *  \param  in_entity_set_size           (Out)    Occupied size of 
 *                                                in_entity_set.
 *  \param  err                          (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getAdjEntities(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            int entity_type_requestor,
            int entity_topology_requestor,
            int entity_type_requested,
            iBase_EntityHandle **adj_entities,
            int *adj_entities_allocated,
            int *adj_entities_size,
            int **offset,
            int *offset_allocated,
            int *offset_size,
            int **in_entity_set,
            int *in_entity_set_allocated,
            int *in_entity_set_size, 
            int *err);

/** \brief Create an entity iterator for a given part and entity set.  

 *  Given a local part and an entity set, return an iterator over the
 *  entities of the requested type and topology that are in both the
 *  part and the entity set.
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntIter, iMesh_resetEntIter, and iMesh_endEntIter,
 *  respectively.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                     (In)  Mesh instance containing the 
 *                                             partition.
 *  \param  partition                    (In)  The partition being queried.
 *  \param  part                         (In)  The part being queried.
 *  \param  entity_set                   (In)  Entity set handle for the 
 *                                             entity set being queried.
 *  \param  requested_entity_type        (In)  Type of entities to include in
 *                                             the iterator.
 *  \param  requested_entity_topology    (In)  Topology of entities to include
 *                                             in the iterator.
 *  \param  entity_iterator              (Out) Iterator returned from function.
 *  \param  err                          (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_initEntIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            const int requested_entity_type,
            const int requested_entity_topology,
            iBase_EntityIterator* entity_iterator,
            int *err);



/** \brief Create an entity array iterator for a given part and entity set.

 *  Given a local part and an entity set, return an array iterator over the
 *  entities of the requested type and topology that are in both the
 *  part and the entity set.  
 *  Iterator functionality for getNext, reset, and end is 
 *  provided through the regular iMesh iterator functions
 *  iMesh_getNextEntArrIter, iMesh_resetEntArrIter, and iMesh_endEntArrIter,
 *  respectively.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                     (In)  Mesh instance containing the 
 *                                             partition.
 *  \param  partition                    (In)  The partition being queried.
 *  \param  part                         (In)  The part being queried.
 *  \param  entity_set                   (In)  Entity set handle for the 
 *                                             entity set being queried.
 *  \param  requested_entity_type        (In)  Type of entities to include in
 *                                             the iterator.
 *  \param  requested_entity_topology    (In)  Topology of entities to include
 *                                             in the iterator.
 *  \param  requested_array_size         (In)  The number of handles returned 
 *                                             in each value of the iterator.
 *  \param  entArr_iterator              (Out) Iterator returned from function.
 *  \param  err                          (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_initEntArrIter(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iMeshP_PartHandle part,
            const iBase_EntitySetHandle entity_set,
            const int requested_entity_type,
            const int requested_entity_topology,
            const int requested_array_size,
            iBase_EntityArrIterator* entArr_iterator,
            int *err);




/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*                           Entity Functionality                         */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/** \brief  Return the part ID of the part owning an entity.
 *
 *  Given an entity handle and a partition handle, return the part ID 
 *  of the part that owns the entity.
 *  Return an error code if an entity is not in the partition.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                     (In)  Mesh instance containing the 
 *                                             partition.
 *  \param  partition                    (In)  The partition being queried.
 *  \param  entity                       (In)  Entity whose owning part is to be
 *                                             returned.
 *  \param  part_id                      (Out) Part ID of the part owning
 *                                             the entity.
 *  \param  err                          (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getEntOwnerPart(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity,
            iMeshP_Part *part_id,
            int *err); 


/** \brief  Return the part IDs of the parts owning the given entities.
 *
 *  Given an array of entity handles and a partition handle, return for each
 *  entity handle the part ID of the part that owns the entity.
 *  Return an error code if an entity is not in the partition.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance              (In)     Mesh instance containing the 
 *                                         partition.
 *  \param  partition             (In)     The partition being queried.
 *  \param  entities              (In)     Entity whose owning part is to be
 *                                         returned.
 *  \param  entities_size         (In)     Number of entities in 
 *                                         entities array.
 *  \param  part_ids              (Out)    Part IDs of the parts owning
 *                                         the entities.
 *  \param  part_ids_allocated    (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size         (Out)    Occupied size of part_ids array.
 *  \param  err                   (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getEntOwnerPartArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle *entities,
            const int entities_size,
            iMeshP_Part **part_ids,
            int *part_ids_allocated,
            int *part_ids_size,
            int *err); 
  


/** \brief Test for entity ownership with respect to a part.
 *
 *  Given a partition handle, a part handle, and an entity handle, return a
 *  flag indicating whether the entity is owned by the part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance             (In)  Mesh instance containing the partition.
 *  \param  partition            (In)  The partition being queried.
 *  \param  part                 (In)  The part being queried.
 *  \param  entity               (In)  Entity whose ownership is being tested.
 *  \param  is_owner             (Out) Flag indicating whether the given part 
 *                                     is the owner of the given entity.
 *  \param  err                  (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_isEntOwner(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle entity, 
            int *is_owner, 
            int *err); 


/** \brief Test for entity ownership of many entities with respect to a part.
 *
 *  Given a partition handle, a part handle, and an array of entity handles, 
 *  return for each entity handle a flag indicating whether the entity 
 *  is owned by the part.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                 (In)     Mesh instance containing the 
 *                                            partition.
 *  \param  partition                (In)     The partition being queried.
 *  \param  part                     (In)     The part being queried.
 *  \param  entities                 (In)     Entities whose ownership is 
 *                                            being tested.
 *  \param  entities_size            (In)     Number of entity handles in
 *                                            entities.
 *  \param  is_owner                 (Out)    Flag for each entity indicating 
 *                                            whether the given part is the 
 *                                            owner of the given entity.
 *  \param  is_owner_allocated       (In/Out) Allocated size of is_owner array.
 *  \param  is_owner_size            (Out)    Occupied size of is_owner array.
 *  \param  err                      (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_isEntOwnerArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle *entities, 
            const int entities_size, 
            int **is_owner, 
            int *is_owner_allocated, 
            int *is_owner_size, 
            int *err); 



/** \brief Return entity status (Internal, boundary, ghost).
 *
 *  Given a partition handle, a part handle, and an entity handle, return a
 *  flag indicating whether the entity is strictly internal, is on a 
 *  part boundary, or is a ghost with respect to the given part.  
 *  The returned value is a member of the iMeshP_EntStatus enumerated type.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance             (In)  Mesh instance containing the partition.
 *  \param  partition            (In)  The partition being queried.
 *  \param  part                 (In)  The part being queried.
 *  \param  entity               (In)  Entity whose status is being tested.
 *  \param  par_status           (Out) Value indicating the status of the
 *                                     is the entity with respect to the part.
 *  \param  err                  (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getEntStatus(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle entity, 
            int *par_status,
            int *err); 



/** \brief Return entity status (Internal, boundary, ghost).
 *
 *  Given a partition handle, a part handle, and an array of entity handles, 
 *  return for each entity handle a flag indicating whether the entity is 
 *  strictly internal, is on a part boundary, or is a ghost with respect 
 *  to the given part.  
 *  The returned value is a member of the iMeshP_EntStatus enumerated type.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  part                    (In)     The part being queried.
 *  \param  entities                (In)     Entities whose status is 
 *                                           being tested.
 *  \param  entities_size           (In)     Number of entity handles in
 *                                           entities.
 *  \param  par_status              (Out)    Value for each entity indicating 
 *                                           the status of the entity with 
 *                                           respect to the part.
 *  \param  par_status_allocated    (In/Out) Allocated size of par_status array.
 *  \param  par_status_size         (Out)    Occupied size of par_status array.
 *  \param  err                     (Out)    Error code.
 */

IMESH_EXPORT
void iMeshP_getEntStatusArr(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iMeshP_PartHandle part, 
            const iBase_EntityHandle *entities, 
            const int entities_size, 
            int **par_status, /* enum iMeshP_EntStatus */
            int *par_status_allocated, 
            int *par_status_size, 
            int *err); 



/** \brief Return the number of copies of an entity that exist in the partition.
 *
 *  Given a partition handle and an entity handle, return the number 
 *  of copies of the entity in the partition.  
 *  If the given entity is an owned entity or boundary entity, 
 *  the number of copies will be complete.
 *  If the given entity is a ghost entity, the number of copies will be two
 *  (the ghost and its owner).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance             (In)  Mesh instance containing the partition.
 *  \param  partition            (In)  The partition being queried.
 *  \param  entity               (In)  Entity whose copy info is requested.
 *  \param  num_copies_ent       (Out) Number of copies of the entity that 
 *                                     exist in the partition.
 *  \param  err                  (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getNumCopies(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            int *num_copies_ent,
            int *err); 



/** \brief Return the part IDs of parts having copies of a given entity.
 * 
 *  Given a partition handle and an entity handle, return the part IDs
 *  of copies of the entity in the partition. 
 *  If the given entity is an owned entity or boundary entity, 
 *  the number of copies considered will be complete.
 *  If the given entity is a ghost entity, the number of copies considered
 *  will be two (the ghost and its owner).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  entity                  (In)     Entity whose copy info
 *                                           is requested.
 *  \param  part_ids                (Out)    Part IDs of parts having copies
 *                                           of the given entity.
 *  \param  part_ids_allocated      (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size           (Out)    Occupied size of part_ids array.
 *  \param  err                     (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getCopyParts(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            iMeshP_Part **part_ids, 
            int *part_ids_allocated, 
            int *part_ids_size, 
            int *err); 



/**  \brief Get (remote) entity handles of copies of a given entity.
 *
 *  Given a partition handle and an entity handle, return (remote) entity
 *  handles and part IDs of all copies of the entity.
 *  If the given entity is an owned entity or boundary entity, 
 *  the number of copies considered will be complete.
 *  If the given entity is a ghost entity, the number of copies considered
 *  will be two (the ghost and its owner).
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  entity                  (In)     Entity whose copy info
 *                                           is requested.
 *  \param  part_ids                (Out)    Part IDs of parts having copies
 *                                           of the given entity.
 *  \param  part_ids_allocated      (In/Out) Allocated size of part_ids array.
 *  \param  part_ids_size           (Out)    Occupied size of part_ids array.
 *  \param  copies                  (Out)    (Remote) entity handles of the 
 *                                           entity copies.
 *  \param  copies_allocated        (In/Out) Allocated size of copies.
 *  \param  copies_size             (Out)    Occupied size of copies.
 *  \param  err                     (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_getCopies(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            iMeshP_Part **part_ids, 
            int *part_ids_allocated, 
            int *part_ids_size, 
            iBase_EntityHandle **copies, 
            int *copies_allocated, 
            int *copies_size,
            int *err); 



/**  \brief Get the entity handle of a copy of a given entity in a given part.
 *
 *  Given a partition handle, an entity handle and a part ID, 
 *  return the (remote) entity handle of the copy of the entity in that part.
 *  Return an error if the entity does not exist in the specified part.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  entity                  (In)  Entity whose copy info
 *                                        is requested.
 *  \param  part_id                 (In)  Part ID of part whose copy 
 *                                        of the given entity is requested.
 *  \param  copy_entity             (Out) (Remote) entity handle of the 
 *                                        entity copy from the given part.
 *  \param  err                     (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getCopyOnPart(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            const iMeshP_Part part_id, 
            iBase_EntityHandle *copy_entity, 
            int *err); 



/**  \brief Get the entity handle of a copy of a given entity in its owner part.
 *
 *  Given a partition handle and an entity handle, return the (remote) 
 *  entity handle of the copy of the entity in its owner part.
 *
 *  COMMUNICATION:  None++.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  entity                  (In)  Entity whose copy info
 *                                        is requested.
 *  \param  owner_part_id           (Out) Part ID of the entity's owner part.
 *  \param  owner_entity            (Out) (Remote) entity handle of the 
 *                                        entity copy from the owner part.
 *  \param  err                     (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_getOwnerCopy(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition, 
            const iBase_EntityHandle entity, 
            iMeshP_Part *owner_part_id, 
            iBase_EntityHandle *owner_entity, 
            int *err); 


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*-------                         COMMUNICATION                 ----------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/


/**\brief  Wait for a specific iMeshP request to complete.
 *
 *  Given an iMeshP_RequestHandle, wait for the request to complete.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  request                 (In)  iMeshP request for whose completion
 *                                        we should wait.
 *  \param  err                     (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_waitForRequest(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle request,
            int *err);


/**\brief  Wait for any of the specified iMeshP requests to complete.
 *
 *  Given an array of iMeshP_RequestHandles, wait for any one of the requests 
 *  to complete.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  requests                (In)  iMeshP requests for which we wait
 *                                        until one request completes.
 *  \param  requests_size           (In)  Number of requests in requests.
 *  \param  index                   (Out) Index of the request that completed.
 *  \param  err                     (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_waitForAnyRequest(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle *requests,
            int requests_size,
            int *index,
            int *err);



/**\brief  Wait for all of the specified iMeshP requests to complete.
 *
 *  Given an array of iMeshP_RequestHandles, wait for all of the requests 
 *  to complete.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  requests                (In)  iMeshP requests for which we wait
 *                                        until completion.
 *  \param  requests_size           (In)  Number of requests in requests.
 *  \param  err                     (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_waitForAllRequests(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle *requests,
            int requests_size,
            int *err);


/**\brief  Wait for a specific request to complete; return entities received.
 *
 *  Given an iMeshP_RequestHandle, wait for the request to complete.  Return
 *  entities for which information was received.
 *
 *  COMMUNICATION:  Blocking point-to-point.
 *
 *  \param  instance                (In)     Mesh instance containing the 
 *                                           partition.
 *  \param  partition               (In)     The partition being queried.
 *  \param  request                 (In)     iMeshP request for whose completion
 *                                           we should wait.
 *  \param  out_entities            (Out)    Entities for which information was
 *                                           received.
 *  \param  out_entities_allocated  (In/Out) Allocated size of out_entities.
 *  \param  out_entities_size       (Out)    Occupied size of out_entities.
 *  \param  err                     (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_waitForRequestEnt(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle request,
            iBase_EntityHandle **out_entities,
            int *out_entities_allocated,
            int *out_entities_size,
            int *err);

/**\brief  Test whether a specific request has completed.
 *
 *  Given an iMeshP_RequestHandle, test whether the request has completed.
 *  This function will not wait until the request completes; it will only
 *  return the completion status (complete = 1 or 0).
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance                (In)  Mesh instance containing the 
 *                                        partition.
 *  \param  partition               (In)  The partition being queried.
 *  \param  request                 (In)  iMeshP request for whose completion
 *                                        we should test.
 *  \param  completed               (Out) Flag indicating whether (1) or 
 *                                        not (0) the given request has 
 *                                        completed.
 *  \param  err                     (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_testRequest(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle request,
            int *completed,
            int *err);



/** \brief  Poll for outstanding requests.  
 *
 *  Check for outstanding requests from other parts, handle any requests 
 *  found, and return an array of requests that have been handled.  If
 *  the array has a size allocated already, then the implementation stops
 *  working when it has generated that many completed requests, even if there
 *  are more requests waiting. 
 *  
 *  COMMUNICATION:  non-blocking; point-to-point.
 *
 *  \param  instance                     (In)     Mesh instance containing the 
 *                                                partition.
 *  \param  partition                    (In)     The partition being queried.
 *  \param  requests_completed           (Out)    Requests that were completed.
 *  \param  requests_completed_allocated (In/Out) Allocated size of
 *                                                requests_completed.
 *  \param  requests_completed_size      (Out)    Occupied size of
 *                                                requests_completed.
 *  \param  err                          (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_pollForRequests(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            iMeshP_RequestHandle **requests_completed,
            int *requests_completed_allocated,
            int *requests_completed_size,
            int *err);

/*--------------------------------------------------------------------
  -------    Requests for off-processor mesh modification      -------
  --------------------------------------------------------------------*/

/** \brief  Add entities to on-process and/or off-process parts.
 *
 *  Given a partition and a list of entities, add those entities to the
 *  target parts.  The entities can be added as copies or migrated entirely
 *  (i.e., change ownership of the entities)
 *  to the parts.  The entities' downward adjacencies are also copied and/or
 *  migrated as appropriate to support the entities.
 *  This function is a collective, non-blocking operation
 *  to be called by all processes in the partition's communicator.  
 *  An iMeshP_RequestHandle is returned; any of the 
 *  iMeshP_wait* functions can be used to block until the request is completed.
 *
 *  COMMUNICATION:  Collective.  Non-blocking.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  Handle for the partition being queried.
 *  \param  entities          (In)  Entities to be sent.
 *  \param  entities_size     (In)  Number of entities to be sent.
 *  \param  target_part_ids   (In)  Array of size entities_size listing
 *                                  the parts to which the entities should
 *                                  be sent.
 *  \param  command_code      (In)  Flag indicating whether to migrate
 *                                  the entities or only make copies.
 *  \param  update_ghost      (In)  Flag indicating whether (1) or not (0)
 *                                  ghost copies of the entities should be
 *                                  updated with new owner information.
 *  \param  request           (Out) iMeshP RequestHandle returned; can be used 
 *                                  for blocking until this send is complete.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_exchEntArrToPartsAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntityHandle *entities,
            const int entities_size,
            const iMeshP_Part *target_part_ids,
            int command_code,  
            int update_ghost,
            iMeshP_RequestHandle *request,
            int *err);


/** \brief Request in-migration of an entity and its upward adjacencies.
 *
 *  This function is a "pull" migration, where a part requests to become the
 *  owner of an entity that is owned by another part (so that the part has
 *  the right to modify the entity).  The requested
 *  entity must be on the part boundary and is identified by a local handle
 *  (i.e., an entity part-boundary copy).   This operation may require multiple
 *  rounds of communication, and at some times, certain entities may be
 *  locked (unavailable for local modification) while info about their
 *  remote copies is still in question.  Tags and parallel set membership 
 *  are migrated as well as the appropriate adjacency info.
 *  An iMeshP request handle is returned.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, pull. 
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  part              (In)  The part to which the entity is migrated.
 *  \param  local_entity      (In)  The local entity copy for the entity to be
 *                                  migrated.
 *  \param  request           (Out) The iMeshP request handle returned.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_migrateEntity(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            iMeshP_PartHandle part,
            iBase_EntityHandle local_entity,
            iMeshP_RequestHandle *request,
            int *err);



/** \brief Update vertex coordinates for vertex copies.  
 *
 *  For a given vertex, update its copies with the vertex's coordinates.
 *  This function assumes that a local vertex's coordinates were updated
 *  through a call to iMesh_setVtxCoords.  This function then updates all
 *  copies of the vertex with the updated coordinates.
 *  The communication here is push-and-forget; as such, 
 *  no request handle needs to be returned.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push-and-forget.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  local_vertex      (In)  The vertex whose copies should be updated.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_updateVtxCoords(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iBase_EntityHandle local_vertex,
            int *err);



/** \brief Replace entities on the part boundary.
 *
 *  This function performs changes on the part boundary where the
 *  calling application can ensure that things are done
 *  identically on both sides and that the arguments are passed in an order 
 *  that can be matched.  (Specifically, matching new entities should appear in
 *  the same order in the call array.)  An example is creation of new 
 *  boundary edges during edge splitting.
 *  Communication here could be a
 *  two-way push-and-forget, or some variant on push-and-confirm.
 *  CHANGES: At Onkar's suggestion, added an offset array (similar to array
 *  adjacency requests) so that a single call can easily handle coordination
 *  with multiple entities on part-boundary.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push-and-forget. 
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  old_entities      (In)  The entities to be replaced.
 *  \param  old_entities_size (In)  The number of entities to be replaced.
 *  \param  new_entities      (In)  The entities that replace the old entities.
 *  \param  new_entities_size (In)  The number of entities in new_entities.
 *  \param  offset            (In)  Index into new_entities; old_entities[i]
 *                                  is replaced by new_entities[offset[i]] to
 *                                  new_entities[offset[i+1]-1].
 *  \param  offset_size       (In)  The number of entries in offset.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_replaceOnPartBdry(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iBase_EntityHandle *old_entities,
            const int old_entities_size,
            const iBase_EntityHandle *new_entities,
            const int new_entities_size,
            const int *offset,
            const int offset_size,
            int *err);



/** \brief Push ghost copies of individual entities onto other parts.
 *
 *  Given an entity and a target part, create a ghost copy of the entity on
 *  the target part.
 * 
 *  Communication here is push-and-confirm (so that the original knows remote
 *  entity handle of the created ghosts).  The closure of a new ghost is pushed
 *  automatically as part of the underlying communication.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  target_part_id    (In)  The part to receive the new ghost.
 *  \param  entity_to_copy    (In)  The entity to be copied in target_part_id.
 *  \param  request           (Out) The iMeshP request handle returned.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_addGhostOf(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part target_part_id,
            iBase_EntityHandle entity_to_copy,
            iMeshP_RequestHandle *request,
            int *err);

/** \brief Remove ghost copies of individual entities from other parts.
 *
 *  Given an entity and a target part, remove the ghost copy of the entity on
 *  the target part.
 *
 *  Communication is push-and-forget; as such, no request handle is needed.
 *  The remote part will clean up the closure of the removed ghost
 *  as appropriate during deletion.
 *
 *  COMMUNICATION:  point-to-point, non-blocking, push-and-forget.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  target_part_id    (In)  The part to lose the ghost.
 *  \param  copy_to_purge     (In)  The entity whose ghost is removed from 
 *                                  target_part_id.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_rmvGhostOf(
            iMesh_Instance instance, 
            const iMeshP_PartitionHandle partition,
            const iMeshP_Part target_part_id,
            iBase_EntityHandle copy_to_purge,
            int *err);

/** \brief Indicate completion of mesh modification.
 *
 *  Calling this function indicates that the user is finished with mesh 
 *  modification for now.  With mesh modification complete, the implementation
 *  can update ghost, partition, boundary, and other information to 
 *  re-establish a valid distributed mesh.  This function waits for all
 *  message traffic to clear and rebuilds ghost information that was
 *  allowed to go obsolete during mesh modification.
 *
 *  COMMUNICATION:  collective.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  err               (Out) Error code.
 */  
IMESH_EXPORT
void iMeshP_syncMeshAll(
            iMesh_Instance instance, 
            iMeshP_PartitionHandle partition,
            int *err);
                            
/*--------------------------------------------------------------------------*/
/*         Functions to send Tag data from owning entities to copies.       */
/*--------------------------------------------------------------------------*/

/**\brief  Synchronously send tag data for given entity types and topologies.
 *
 *  Send tag information for shared entities of specified type and
 *  topology.  The tag data is "pushed" from the owner entities to all copies.
 *  This version operates on all shared entities of specified type and topology
 *  (or all types/topologies if iBase_ALL_TYPES/iMesh_ALL_TOPOLOGIES are
 *  given).  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  This call blocks until communication is
 *  completed.
 *
 *  COMMUNICATION:  point-to-point, blocking.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  source_tag        (In)  Tag handle for the sending entities.
 *  \param  dest_tag          (In)  Tag handle for the receiving entities.
 *  \param  entity_type       (In)  Tag data is exchanged only for this 
 *                                  entity type.
 *  \param  entity_topo       (In)  Tag data is exchanged only for this 
 *                                  entity topology.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_pushTags(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            int entity_type, 
            int entity_topo, 
            int *err);


/**\brief  Synchronously send tag data for individual entities.
 *
 *  Send tag information for the specified entities.
 *  The tag data is "pushed" from the owner entities to all copies.
 *  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  This call blocks until communication is
 *  completed.
 *
 *  COMMUNICATION:  point-to-point, blocking.
 * 
 *  \param  instance        (In)  Mesh instance containing the partition.
 *  \param  partition       (In)  The partition being queried.
 *  \param  source_tag      (In)  Tag handle for the sending entities.
 *  \param  dest_tag        (In)  Tag handle for the receiving entities.
 *  \param  entities        (In)  Owned entities for which to send data.
 *  \param  entities_size   (In)  The number of entities for which to send data.
 *  \param  err             (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_pushTagsEnt(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            const iBase_EntityHandle *entities,
            int entities_size,
            int *err);




/**\brief  Asynchronously send tag data for given entity types and topologies.
 *
 *  Send tag information for shared entities of specified type and
 *  topology.  The tag data is "pushed" from the owner entities to all copies.
 *  This version operates on all shared entities of specified type and topology
 *  (or all types/topologies if iBase_ALL_TYPES/iMesh_ALL_TOPOLOGIES are
 *  given).  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  
 *  This call does not block; applications should call 
 *  iMeshP_waitForRequest (or a similar wait function)
 *  to block until this push is completed.
 *
 *  COMMUNICATION:  point-to-point, non-blocking.
 * 
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition being queried.
 *  \param  source_tag        (In)  Tag handle for the sending entities.
 *  \param  dest_tag          (In)  Tag handle for the receiving entities.
 *  \param  entity_type       (In)  Tag data is exchanged only for this 
 *                                  entity type.
 *  \param  entity_topo       (In)  Tag data is exchanged only for this 
 *                                  entity topology.
 *  \param  request           (Out) The iMeshP request handle returned.
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_iPushTags(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            int entity_type, 
            int entity_topo, 
            iMeshP_RequestHandle *request,
             int *err);

/**\brief  Asynchronously send tag data for individual entities.
 *
 *  Send tag information for the specified entities.
 *  The tag data is "pushed" from the owner entities to all copies.
 *  This function assumes tag handles given on various
 *  calling parts are consistent; i.e. they have the same name,
 *  data type, size, etc.  
 *  This call does not block; applications should call 
 *  iMeshP_waitForRequest (or a similar wait function)
 *  to block until this push is completed.
 *
 *  COMMUNICATION:  point-to-point, non-blocking.
 * 
 *  \param  instance        (In)  Mesh instance containing the partition.
 *  \param  partition       (In)  The partition being queried.
 *  \param  source_tag      (In)  Tag handle for the sending entities.
 *  \param  dest_tag        (In)  Tag handle for the receiving entities.
 *  \param  entities        (In)  Owned entities for which to send data.
 *  \param  entities_size   (In)  The number of entities for which to send data.
 *  \param  request         (Out) The iMeshP request handle returned.
 *  \param  err             (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_iPushTagsEnt(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            iBase_TagHandle source_tag, 
            iBase_TagHandle dest_tag, 
            const iBase_EntityHandle *entities,
            int entities_size,
            iMeshP_RequestHandle *request,
            int *err);


/*------------------------------------------------------------*
 *                   GHOSTING                                 *
 *------------------------------------------------------------*/

/* \brief Create ghost entities between parts.
 *
 *  Ghost entities are specified similar to 2nd-order adjacencies, i.e.,
 *  through a "bridge" dimension.  The number of layers is measured from
 *  the inter-part interfaces.  For example, to get two layers of region
 *  entities in the ghost layer, measured from faces on the interface,
 *  use ghost_dim=3, bridge_dim=2, and num_layers=2.
 *  The number of layers specified is with respect to the global mesh;
 *  that is, ghosting may extend beyond a single neighboring processor if the
 *  number of layers is high.
 *
 *  Ghost information is cached in the partition.  
 *  The triplet describing a ghosting "rule" (ghost dim, bridge dim, #
 *  layers) is stored in the partition; ghosting that became incorrect
 *  due to mesh modification or redistribution of mesh entities is 
 *  re-established using these rules by the end
 *  of iMeshP_syncPartitionAll and iMeshP_syncMeshAll.  
 *  Implementations can choose to keep ghosting consistent throughout 
 *  mesh modification, but ghosts are not required to be consistent until 
 *  the end of these two functions.

 *  iMeshP_createGhostEntsAll is cumulative; that is, multiple calls can only
 *  add more ghosts, not eliminate previous ghosts.  
 *  
 *  COMMUNICATION:  Collective.  Blocking.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition in which to create ghosts.
 *  \param  ghost_type        (In)  Entity type of entities to be ghosted.
 *  \param  bridge_type       (In)  Entity type through which bridge 
 *                                  adjacencies are found.
 *  \param  num_layers        (In)  Number of layers of ghost entities.
 *  \param  include_copies    (In)  Flag indicating whether to create ghosts 
 *                                  of non-owned part boundary entities 
 *                                  (YES=1, NO=0).
 *  \param  err               (Out) Error code.
 */
IMESH_EXPORT
void iMeshP_createGhostEntsAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int ghost_type,
            int bridge_type,
            int num_layers,
            int include_copies,
            int *err);



/* \brief Delete all ghost entities between parts.
 *
 *  Given a partition, delete all ghost entities in that partition of the mesh.
 *
 *  COMMUNICATION:  Collective.  Blocking.
 *
 *  \param  instance          (In)  Mesh instance containing the partition.
 *  \param  partition         (In)  The partition from which to delete ghosts.
 *  \param  err               (Out) Error code.
 *
 */
IMESH_EXPORT
void iMeshP_deleteGhostEntsAll(
            iMesh_Instance instance,
            iMeshP_PartitionHandle partition,
            int *err);




/** \brief Return information about all ghosting on a partition.
 *
 *  Return the ghosting rules established through calls to
 *  iMeshP_createGhostEntsAll.
 *
 *  COMMUNICATION:  None.
 *
 *  \param  instance               (In)     Mesh instance containing the 
 *                                          partition.
 *  \param  partition              (In)     The partition to be queried.
 *  \param  ghost_rules_allocated  (In/Out) Allocated size of ghost_type,
 *                                          bridge_type and num_layers.
 *  \param  ghost_rules_size       (Out)    Occupied size of ghost_type, 
 *                                          bridge_type and num_layers;
 *                                          equal to the number of ghosting 
 *                                          rules currently registered in 
 *                                          the partition.
 *  \param  ghost_type             (Out)    Entity type of ghost entities 
 *                                          for each rule.
 *  \param  bridge_type            (Out)    Entity type of bridge entities 
 *                                          for each rule.
 *  \param  num_layers             (Out)    Number of layers of ghosts in each 
 *                                          rule.
 *  \param  err                    (Out)    Error code.
 */
IMESH_EXPORT
void iMeshP_ghostEntInfo(
            const iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            int *ghost_rules_allocated, 
            int *ghost_rules_size, 
            int **ghost_type,
            int **bridge_type,
            int **num_layers,
            int *err);

/*--------------------------------------------------------------------------
            FILE I/O                                          
 --------------------------------------------------------------------------*/
/* iMeshP file I/O closely aligns with iMesh file I/O.  The major
 * change is the addition of a iMeshP_PartitionHandle argument to both
 * iMeshP_loadAll and iMeshP_saveAll, enabling I/O from parallel processes.
 * For now, individual implementations will support different sets of
 * options; Tim and Ken will work to unify the options by SC08.
 */

/** \brief Populate a mesh instance and a partition by reading data from files.
 * 
 *  Before calling iMeshP_loadAll, the application creates both a mesh 
 *  instance and a partition handle.  iMeshP_loadAll then reads the
 *  specified file, inserts entities into the mesh instance, constructs
 *  parts within the partition, and inserts entities into the parts.
 *  Options allow n>=1 files on p processes.
 *  Optional capabilities of iMeshP_loadAll include computing an initial
 *  partition (e.g., if a serial mesh file without part assignments is read)
 *  and creating ghost entities as requested by the application; the
 *  availability of these options is implementation dependent.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance            (In)  Mesh instance to contain the data.
 *  \param  partition           (In)  The newly populated partition.
 *  \param  entity_set          (In)  Set to which the mesh will be added.
 *  \param  name                (in)  File name from which mesh data is read.
 *  \param  options             (In)  Implementation-specific options string.
 *  \param  err                 (Out) Error code.
 *  \param  name_len            (In)  Length of the file name character string.
 *  \param  options_len         (In)  Length of the options character string.
 */
IMESH_EXPORT
void iMeshP_loadAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            const char *name, 
            const char *options,
            int *err, 
            int name_len, 
            int options_len);

/** \brief Write data from a mesh instance and a partition to files.
 *
 *  iMeshP_saveAll writes mesh and partition data to the specified file.
 *  Options allow n>=1 files on p processes.
 *
 *  COMMUNICATION:  Collective.
 * 
 *  \param  instance            (In)  Mesh instance containing the partition.
 *  \param  partition           (In)  The partition being saved.
 *  \param  entity_set          (In)  Set from which data will be saved.
 *  \param  name                (in)  File name to which mesh data is written.
 *  \param  options             (In)  Implementation-specific options string.
 *  \param  err                 (Out) Error code.
 *  \param  name_len            (In)  Length of the file name character string.
 *  \param  options_len         (In)  Length of the options character string.
 */
IMESH_EXPORT
void iMeshP_saveAll(
            iMesh_Instance instance,
            const iMeshP_PartitionHandle partition,
            const iBase_EntitySetHandle entity_set,
            const char *name, 
            const char *options,
            int *err, 
            const int name_len, 
            int options_len);


/*
------------------------------------------------
Major Items left to do:
-  Support for multiple partitions.
   We discussed designating a given partition as 
   the "active" partition; i.e., the partition that is actually used in 
   the distribution of mesh data in distributed memory.  We were concerned 
   that when multiple partitions were used, multiple copies of mesh 
   entities would be needed to fully support multiple partitions at the 
   same time.  Designating one partition as "active" would store data 
   with respect to only one partition.
-  File I/O support.
   Need common set of options to allow interoperability.
   Support single files, N << P files on P processes, and P files.
   Support reading and writing partition information.
   Support initial parallel partitioning of serial file data.
   Support storing mapping of parts to processes in files.

------------------------------------------------
Minor Items left to do:
-  Determine which capabilities need both "getNumX" and "getX" functions.
   That is, when would an application need "getNumX" to allocate memory
   for "getX" or separately from "getX".  When could we use only "getX"
   and return numX as a byproduct.

-  Determine with functions need "Ent" and "EntArr" versions, or whether
   we should adopt only the more general "EntArr" version.

-  Determine whether to revise iMeshP_createPartition to make it less MPI 
   specific.  We don't want to require applications to link with MPI if the 
   implementation doesn't require it.  We may define an ITAPS_Comm name 
   typedef'ed appropriately.

-  iMeshP_getOwnerCopy could be achieved by calling iMeshP_getOwnerPart
   followed by iMeshP_getCopyOnPart.  Do we want to keep iMeshP_getOwnerCopy?

-  Need function to receive tag data from part-boundary entities in owner.
   Possible options:  return the tag data values received directly, or 
   include a mathematical operation (similar to MPI_SUM). 9/15/08

------------------------------------------------
Comments and resolved questions:  

- Applications will build partitions by (1) creating a partition handle
  on each process to be included in the partition; (2) adding parts to 
  the partition handle within the process; (3) populating the parts with 
  entities, and (4) calling iMeshP_syncPartitionAll to allow the 
  implementation to compute global data for the partition.

- For now, we will not include an iterator over local (to a
  process) parts within a partition.  If it is needed, it can be added
  later.

- We will not provide capability to move entire parts to new
  processes;  instead, the new process must create the part in its
  partition handle and then receive (perhaps in bulk) the entities to 
  populate the part.  In other words, parts can be added to only a local 
  partition handle.

- Currently, iMesh doesn't have the functionality to get entities or 
  entity sets by type and tag in serial.  Should it?  
  Many people said it would be useful; others said it could be costly
  (in parallel) or numerically difficult (for floating point values).
  This issue is an iMesh issue, not a parallel interface issue, so
  for this document, the issue is resolved.  The resolution:  If 
  iMesh adopts this capability, we will add it to the
  parallel interface.

- We will not include functions that return all entities with 
  given characteristics within a partition; the memory use of these
  functions can be large.  Instead, we will return entity information
  with respect to parts and/or mesh instances.  If the user wants such
  info, he should go through the mechanics of gathering it himself so
  that he is painfully aware of how much memory he is allocating.
  Removed the following global queries:
  + All tag names over the partition;
  + All entities in this partition having a given type, tag and/or 
    tag name.
  + All entity sets in this partition having a given
    type, tag and/or tag name.

- We will not include functions that return information about each
  part and/or process in a partition.  Such functions limit memory 
  scalability for large numbers of parts.  If the user wants such
  info, he should go through the mechanics of gathering it himself so
  that he is painfully aware of how much memory he is allocating.
  Removed the following global queries:
  + The number of entities in each part of the partition;
  + The number of entity sets in each part of the partition;
  + The number of entities with given type, tag, and/or
    tag name in each part of the partition;
  + The number of entity sets with given type, tag, 
    and/or tag name in each part of the partition;
  + All tag names in each part of the partition;

- For functions that replace a set handle with a part handle, return
  all appropriate entities in a part, whether they are owned or are 
  copies.  The application can test for ownership if needed.

- Part assignments computed with respect to a set of 
  entities induce part assignments to adjacent entities in an
  implementation-dependent fashion.  That is, if a partition is computed
  with respect to regions, queries about ownership of faces and vertices
  are valid.

------------------------------------------------
Discussed but unresolved questions:

- We discussed adding functions that give
  hints to an implementation about which data mappings the application 
  will use, allowing the implementation to pre-compute them if it chooses 
  to.  The example discussed was mapping between entities and parts, but 
  other examples in iMesh may also exist.

- We discussed adding an iterator over entities 
  with given type/topology in a set or part.  We have non-iterator 
  functionality, but not an iterator.  
  KDD:  Is this true?  What is iMesh_initEntIter (and its analogous
  KDD:  iMeshP_initEntIter)?

- We discussed storing in a partition 
  information about which "objects" were used in computing the partition.  
  These objects can be single entities or groups of entities.
  KDD:  Perhaps this capability should be part of the load-balancing service.

- We discussed designating a given partition as 
  the "active" partition; i.e., the partition that is actually used in 
  the distribution of mesh data in distributed memory.  We were concerned 
  that when multiple partitions were used, multiple copies of mesh 
  entities would be needed to fully support multiple partitions at the 
  same time.  Designating one partition as "active" would store data 
  with respect to only one partition.

------------------------------------------------
Not-yet-discussed, unresolved questions

Entity questions:
- From Carl:  "getTag*Operate: Again, we haven't got this in serial.  Does 
  the existence of such operations imply that we expect to implement 
  fields as tags? (Because that wasn't what I was assuming about field 
  implementations at all, personally...)  Note that I'm not opposed to 
  this sort of global reduction operation, I just wonder whether it'll see 
  use outside of field-like situations.  If not, then it should be in 
  parallel fields, not parallel mesh, and usage for 
  fields-implemented-as-tags should be handled there."
*/

/*--------------------------------*/
/* NOTES FROM BOOTCAMP MARCH 2008 */
/*--------------------------------*/
/*
-  Changed getPartRank to getRankOfPart.  (Carl)
-  Made sure iMeshP_getNumOfTypeAll and iMeshP_getNumOfTopoAll were
documented as collective operations.  (Carl)
-  Changed suffix "Par" to "All".  (Lori)
-  Added iMeshP_testPart() to test status of part handle, returning
LOCAL, REMOTE, or INVALID.  (Mark M, Lori).
6/25/08:  Removed this function since part handles can no longer be remote.
If an application wants to test the validity of a part handle, it can try
to compute its Part ID.
-  Changed iMeshP_addCopyOf and iMeshP_rmvCopyOf back to
iMeshP_addGhostOf and iMeshP_rmvGhostOf.  If we wanted to use these
functions for adding boundary copies, we'd have to include a list of
already existing remote copies in the arguments, as well as
communicate with parts already owning copies to let them know a ghost
copy has been made.  Actually, this raises an interesting question:
does a boundary copy need to know about all ghost copies of it?
-  Change getEntParStatus to getEntStatus.  (Lori)
-  Changed sendEntArrToPartsPar to exchEntArrToPartsAll.  (Lori,Tim)


Parts and Processes:
-  Martin argued for consecutive unique Part IDs in addition to or
instead of Part handles.  He will send use cases.   If we decide to
add them back to the interface, we could compute them in
iMeshP_syncPartitionAll rather than in iMeshP_createPart.  That is, an
application couldn't access them until after iMeshP_syncPartitionAll.
6/25/08:  On follow-up, Martin couldn't recall why having consecutive
PartIDs was necessary.  While we all agree they are conceptually nice,
they are difficult to implement and not really necessary.  Part IDs will
be globally unique but not necessarily consecutive.
-  Are part handles globally unique?  They probably need to be
globally unique in order for them to be useful as remote part
handles.  Also, does the process rank need to be encoded in the part
handle in order to map from parts to processes for communication?
6/25/08:  DECIDED:  We will have globally unique part IDs.  Part handles
will be valid for only local parts.  Accessing remote parts must be done
via Part IDs.
-  If in iMeshP_syncPartitionAll, we computed a mapping from part
handles to integers 0,..., k-1, we could store only ranges of
integers to achieve the part-to-process and process-to-parts mappings;
this would require O(P) storage per process for P processes.
6/5/08:  DECIDED:  Do not need getPartOnRank or getNumPartOnRank.  These
functions were troublesome due to their storage or communication requirements.
We decided to remove them.
-  Alternatively, the mapping of all parts to processes can be stored
in O(k) total memory, distributed across processors (e.g., a
distributed data directory) but interrogating the directory requires
communication.  
6/5/08:  See note above.
-  iMeshP_getPartsOnRank created discussion and needs to be resolved.
IMeshP_getPartsOnRank would likely require either O(k) storage per
process for k parts or communication.  For other points, please see
Mark M's 3/12/08 email.  
6/5/08:  See note above.

CreateEnt:
-  Carl asked if we should have a version of createEnt that accepts a
part handle.  Should this function be used only for creating owned
entities?   How do you envision creating part boundary entities when a
parallel mesh is initially loaded?  

Ghost entities:
-  We currently have a mechanism only for pushing ghosts onto other
parts.  Will we want a mechanism for pulling them, too?  (E.g., a
part says, "I want ghosts for this entity.")

PartNbor functions:
-  Did we agree to remove the entity type from these functions?  That
is, do we want to return the part IDs for all parts that have
any copies?  The entity type was intended to allow us to get the part
IDs for all parts that have copies of a given type (perhaps
ALL_TYPES).  

Functions handling both Parts and Entity Sets:
-  Tim said these function names (e.g., iMeshP_getNumOfType,
iMeshP_getAllVtxCoord) are too close to existing iMesh function
names, even though the argument lists would be different.  He agreed
to email suggestions for better names.

Copies:
-  Functions getNumCopies, getCopies, getCopyParts, and getCopyOnPart
have different behavior for ghost and part-boundary copies.  Ghosts
will return only itself and its owner in getCopies; part-boundary
entities will return copies on other parts as well.
-  Tim envisions applications (e.g., FETI methods) updating tag data
in their copies that they would like to accumulate back to the
owner.  Xiaolin said that he writes in his ghosts, but doesn't send
those values back to the owner.  Currently, we have the ability 
to send tag data only from owners to ghosts.  Tim will look at this issue
and propose a solution.

Communication:
-  Although we should think in terms of parts, communication really
occurs with respect to processes.  We need to make sure all our
communication routines make sense with respect to both processes and
parts, and perhaps, revise their descriptions.  Also, if we send to
parts, the implementation really does need the mapping of parts to
processes. 

Entity Owner/Status Queries:
-  Should we combine functions getEntOwnerPart and getEntStatus into
one function?  Or should we combine functions getOwnerCopy and
getEntOwner into one function?  Or should we remove getOwnerCopy and
make applications call getOwnerPart followed by getCopyOnPart?

Reducing storage:
-  Mark Miller proposed allowing the user to specify the amount of 
copying done by the implementation, depending on applications' needs.
For example, for a static viz application, every boundary entity may not
need to know where all its copies are, so the implementation would not
have to store them.  Can the implementations accept a flag advising them how
much copying is needed?  If so, the implementations could choose to 
optimize storage or ignore the flag.
*/



/*--------------------------------------------------------------------
 * SVN File Information
 *
 *   $SVN:Author$
 *   $SVN:Date$
 *   $SVN:Revision$
 *--------------------------------------------------------------------
 */


#ifdef __cplusplus
} /*  extern "C"  */
#endif

#endif /* !defined(_ITAPS_iMeshP) */
