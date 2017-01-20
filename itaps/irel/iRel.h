#ifndef _ITAPS_iRel
#define _ITAPS_iRel

#include "irel_export.h"

/***************************************************************************//**
 * \ingroup VersionNumbers
 * \brief Compile time version number digits
 *
 * iRel maintains a major, minor and patch digit in its version number.
 * Technically speaking, there is not much practical value in patch digit
 * for an interface specification. A patch release is typically only used
 * for bug fix releases. Although it is rare, sometimes a bug fix
 * necessitates an API change. So, we define a patch digit for iRel.
 ******************************************************************************/
#define IREL_VERSION_MAJOR 1
#define IREL_VERSION_MINOR 1
#define IREL_VERSION_PATCH 0

/***************************************************************************//**
 * \ingroup VersionNumbers
 * \brief Maintain backward compatibility with old version symbol names
 ******************************************************************************/
#define IREL_MAJOR_VERSION IREL_VERSION_MAJOR
#define IREL_MINOR_VERSION IREL_VERSION_MINOR
#define IREL_PATCH_VERSION IREL_VERSION_PATCH

/***************************************************************************//**
 * \ingroup VersionNumbers
 * \brief Version Comparison
 *
 * Evaluates to true at CPP time if the version of iRel currently being
 * compiled is greater than or equal to the version specified.
 ******************************************************************************/
#define IREL_VERSION_GE(Maj,Min,Pat) ITAPS_VERSION_GE(Maj,Min,Pat)

/***************************************************************************//**
 * \ingroup VersionNumbers
 * \brief Compose string represention of the iRel version number
 ******************************************************************************/
#define IREL_VERSION_STRING ITAPS_VERSION_STRING_(iRel)

/***************************************************************************//**
 * \ingroup VersionNumbers
 * \brief Compose a symbol name derived from the current iRel version number.
 ******************************************************************************/
#define IREL_VERSION_TAG ITAPS_VERSION_TAG_(iRel)

/***************************************************************************//**
 * \ingroup VersionNumbers
 * \brief Define iRel_create symbol such that it depends on version number.
 *
 * Note: We ran into problems with this as it influences or is influenced by
 * fortran name mangling and so breaks fortran compilation. So, this is
 * currently disabled.
 ******************************************************************************/
#define IREL_CREATE_NAME__(A,B,C) A##_##B##_##C
#define IREL_CREATE_NAME_(A,B,C) IREL_CREATE_NAME__(A,B,C)
#define IREL_CREATE_NAME(A) IREL_CREATE_NAME_(A,IREL_VERSION_MAJOR,IREL_VERSION_MINOR)
/*
#undef  iRel_create
#define iRel_create IREL_CREATE_NAME(iRel_create)
*/

#include "iBase.h"
#include "iRel_protos.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* iRel_Instance;
typedef struct iRel_PairHandle_Private* iRel_PairHandle;

/***************************************************************************//**
 * \brief  \enum IfaceType Enumerator specifying interface types
 *
 * Enumerator specifying interface types.  This enumeration is
 * necessary because functions to get entities of a given dimension
 * are part of the higher-level interfaces (e.g. iGeom, iMesh) instead
 * of iBase.
 ******************************************************************************/
enum iRel_IfaceType {
    iRel_IfaceType_MIN = 0,
        /**< facilitates iteration over all values */
    iRel_IGEOM_IFACE = iRel_IfaceType_MIN, 
        /**< description unavailable */
    iRel_IMESH_IFACE, 
        /**< description unavailable */
    iRel_IFIELD_IFACE, 
        /**< description unavailable */
    iRel_IREL_IFACE,
        /**< description unavailable */
    iRel_FBIGEOM_IFACE,
        /**< description unavailable */
    iRel_IfaceType_MAX = iRel_FBIGEOM_IFACE
        /**< facilitates iteration over all values */
};

/***************************************************************************//**
 * \brief  \enum RelationType Enumerator specifying relation types
 *
 * Enumerator specifying relation types.  A relation has two types, one
 * for each side of the relation.
 ******************************************************************************/
enum iRel_RelationType {
    iRel_RelationType_MIN = 0,
        /**< facilitates iteration over all values */
    iRel_ENTITY = iRel_RelationType_MIN, 
        /**< description unavailable */
    iRel_SET, 
        /**< description unavailable */
    iRel_BOTH,
        /**< description unavailable */
    iRel_RelationType_MAX = iRel_BOTH
        /**< facilitates iteration over all values */
};

/***************************************************************************//**
 * \brief  \enum RelationStatus Enumerator specifying relation status
 *
 * Enumerator specifying relation status.  A relation has two statuses, one
 * for each side of the relation.  Allowed values of this enumeration are:
 * It is an error to request relations from a side that does not have
 * iRel_ACTIVE status.
 ******************************************************************************/
enum iRel_RelationStatus {
    iRel_RelationStatus_MIN = 0,
        /**< facilitates iteration over all values */
    iRel_ACTIVE = iRel_RelationType_MIN, 
        /**< the relation on this side is active and up to date */
    iRel_INACTIVE, 
        /**< the relation on this side is inactive, and may be out of date */
    iRel_NOTEXIST,
        /**< the relation on this side is not stored */
    iRel_RelationStatus_MAX = iRel_NOTEXIST
        /**< facilitates iteration over all values */
};

/***************************************************************************//**
 * \brief Get the error type returned from the last iRel function
 *
 * Get the error type returned from the last iRel function.  Value
 * returned is a member of the iBase_ErrorType enumeration.
 ******************************************************************************/
void IREL_EXPORT iRel_getErrorType(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    int* error_type
        /**< [out] Error type returned from last iRel function */
);

/***************************************************************************//**
 * \brief Get a description of the error returned from the last iRel function
 *
 * Get a description of the error returned from the last iRel function
 ******************************************************************************/
void IREL_EXPORT iRel_getDescription(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    char* descr,
        /**< [inout] Pointer to a character string to be filled with a
        description of the error from the last iRel function */
    int descr_len
        /**< [in] Length of the character string pointed to by descr */
);

/***************************************************************************//**
 * \brief Create a new iRel instance
 *
 * Create a new iRel instance.  Currently no options are implemented.
 ******************************************************************************/
void IREL_EXPORT iRel_create(
    const char* options,
        /**< const Options for the implementation */
    iRel_Instance* instance,
        /**< [in] iRel instance handle */
    int* err,
        /**< [out] Returned Error status (see iBase_ErrorType) */
    const int options_len
        /**< [in] Length of options string */
);

/***************************************************************************//**
 * \brief Destroy the interface object
 *
 * Calls destructor on interface object
 ******************************************************************************/
void IREL_EXPORT iRel_destroy(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Create a relation pair between two interfaces
 *
 * Creates a relation pair between two interfaces, passing
 * back a handle to the pair.  It is an error to create a relation pair
 * having both sides iRel_NOTEXIST.  If a relation pair has a side with status
 * iRel_NOTEXIST, the relation for that side is never stored, and the status
 * cannot change over the life of the relation pair.  
 ******************************************************************************/
void IREL_EXPORT iRel_createPair(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iBase_Instance iface1,
        /**< [in] 1st interface object in the relation pair */
    const int ent_or_set1,
        /**< [in] This relation relates entities, sets, or both from 1st
        interface object */
    const int iface_type1,
        /**< [in] Type of 1st interface */
    const int irel_status1,
        /**< [in] The status of 1st side */
    iBase_Instance iface2,
        /**< [in] 2nd interface object in the relation pair */
    const int ent_or_set2,
        /**< [in] This relation relates entities, sets, or both from 2nd
        interface object */
    const int iface_type2,
        /**< [in] Type of 2nd interface */
    const int irel_status2,
        /**< [in] The status of 2nd side */
    iRel_PairHandle* pair,
        /**< [out] Pointer to relation pair handle, returned from function */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get information for this relation handle
 *
 * Get information about the interfaces and relation type for this
 * relation.  Relation type for each side is passed back as integers,
 * but values will be from RelationType enumeration.
 ******************************************************************************/
void IREL_EXPORT iRel_getPairInfo(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] handle of relation pair being queried */
    iBase_Instance* iface1,
        /**< [out] Side 1 instance for this relation */
    int* ent_or_set1,
        /**< [out] relation type for side 1 of this relation */
    int* iface_type1,
        /**< [out] Interface type for side 1 of this relation */
    int* irel_status1,
        /**< [out] The status of the first side of this relation */
    iBase_Instance* iface2,
        /**< [out] Side 2 instance for this relation */
    int* ent_or_set2,
        /**< [out] Relation type for side 2 of this relation */
    int* iface_type2,
        /**< [out] Interface type for side 2 of this relation */
    int* irel_status2,
        /**< [out] Status of the 2nd side of this relation */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Change the relation type
 *
 * Change the type of one or both sides of a relation.  Only changes that
 * result in no lost information are allowed, e.g. changing a type from SET
 * to BOTH or vice versa.
 ******************************************************************************/
void IREL_EXPORT iRel_changePairType(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being changed */
    int ent_or_set1,
        /**< [in] The new type of side 1 of this relation pair */
    int ent_or_set2,
        /**< [in] The new type of side 2 of this relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Change the relation status
 *
 * Change the status of one or both sides of a relation.  It is an error to
 * change the status of both sides to iRel_NOTEXIST.  If a side is changed to
 * iRel_NOTEXIST, it will no longer be changeable back to iRel_ACTIVE or
 * iRel_INACTIVE. Changing a side from iRel_INACTIVE to iRel_ACTIVE implies a
 * traversal of all related entities on the other side, to recover the relations
 * on the side being changed. Changing both sides from iRel_ACTIVE to something
 * else is an error, since in that case neither will be able to be updated to
 * iRel_ACTIVE.
 ******************************************************************************/
void IREL_EXPORT iRel_changePairStatus(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being changed */
    int irel_status1,
        /**< [in] The new status of side 1 of this relation pair */
    int irel_status2,
        /**< [in] The new status of side 2 of this relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Destroy a relation pair
 *
 * Destroy the relation pair corresponding to the handle input
 ******************************************************************************/
void IREL_EXPORT iRel_destroyPair(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Handle of relation pair to destroy */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get relations containing specified interface
 *
 * Get relations containing the specified interface
 ******************************************************************************/
void IREL_EXPORT iRel_findPairs(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iBase_Instance iface,
        /**< [in] Specified interface */
    iRel_PairHandle** pairs,
        /**< [inout] Pointer to array holding returned relation pairs
        containing specified interface */
    int* pairs_allocated,
        /**< [inout] Pointer to allocated size of relation pairs list */
    int* pairs_size,
        /**< [out] Pointer to occupied size of relation pairs list */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set a relation between two entities
 *
 * Set a relation between an entity and several entities. It is an error
 * to set a relation on a pair with both sides not iRel_ACTIVE.
 ******************************************************************************/
void IREL_EXPORT iRel_setEntEntRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle ent1,
        /**< [in] 1st entity of relation being set */
    iBase_EntityHandle ent2,
        /**< [in] 2nd entity of relation being set */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set a relation between an entity and an entity set
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_setEntSetRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle ent1,
        /**< [in] entity of relation being set */
    iBase_EntitySetHandle entset2,
        /**< [in] entity set of relation being set */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set a relation between an entity set an an entity
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_setSetEntRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle entset1,
        /**< [in] entity set of relation being set */
    iBase_EntityHandle ent2,
        /**< [in] entity of relation being set */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set a relation between two entity sets
 *
 * Description unavailable.
 * Set a relation between an entity and several entities.  It is an error to set
 * a relation on a pair with both sides not iRel_ACTIVE.
 ******************************************************************************/
void IREL_EXPORT iRel_setSetSetRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle entset1,
        /**< [in] 1st entity set of relation being set */
    iBase_EntitySetHandle entset2,
        /**< [in] 2nd entity set of relation being set */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set relations between arrays of entities pairwise,
 * ent_array_1[i]<->ent_array_2[i]
 *
 * Set relations between arrays of entities pairwise, 
 * ent_array_1[i]<->ent_array_2[i].  If either array
 * contains sets and that side of the relation is 'both'-type, 
 * set relations for individual entities in those sets too.  It is an error to
 * set a relation on a pair with both sides not iRel_ACTIVE.
 ******************************************************************************/
void IREL_EXPORT iRel_setEntArrEntArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* ent_array_1,
        /**< [in] 1st array of entities of relation being set */
    int num_ent1,
        /**< [in] Number of entities in 1st array */
    iBase_EntityHandle* ent_array_2,
        /**< [in] 2nd array of entities of relation being set */
    int num_ent2,
        /**< [in] Number of entities in 2nd array */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set relations between arrays of entity sets and entities
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_setSetArrEntArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle* entset_array_1,
        /**< [in] 1st array of entities of relation being set */
    int num_set1,
        /**< [in] Number of entity sets in 1st array */
    iBase_EntityHandle* ent_array_2,
        /**< [in] 2nd array of entities of relation being set */
    int num_ent2,
        /**< [in] Number of entities in 2nd array */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set relations between arrays of entities and entity sets
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_setEntArrSetArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* ent_array_1,
        /**< [in] 1st array of entities of relation being set */
    int num_ent1,
        /**< [in] Number of entities in 1st array */
    iBase_EntitySetHandle* entset_array_2,
        /**< [in] 2nd array of entities of relation being set */
    int num_set2,
        /**< [in] Number of entity sets in 2nd array */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Set relations between arrays of entity sets pairwise,
 * ent_array_1[i]<->ent_array_2[i]
 *
 * Set relations between arrays of entities pairwise, 
 * ent_array_1[i]<->ent_array_2[i].  If either array
 * contains sets and that side of the relation is 'both'-type, 
 * set relations for individual entities in those sets too.  It is an error to
 * set a relation on a pair with both sides not iRel_ACTIVE.
 ******************************************************************************/
void IREL_EXPORT iRel_setSetArrSetArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle* entset_array_1,
        /**< [in] 1st array of entities of relation being set */
    int num_set1,
        /**< [in] Number of entities in 1st array */
    iBase_EntitySetHandle* entset_array_2,
        /**< [in] 2nd array of entities of relation being set */
    int num_set2,
        /**< [in] Number of entities in 2nd array */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity related to specified entity and relation handle
 *
 * Get entity related to specified entity and relation handle.  Also
 * returns whether the related entity is an entity or a set.  It is an error to
 * get a relation for a side with status iRel_NOTEXIST.
 ******************************************************************************/
void IREL_EXPORT iRel_getEntEntRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle ent1,
        /**< [in] 1st entity of relation being queried */
    int switch_order,
        /**< [in] 1st entity is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    iBase_EntityHandle* ent2,
        /**< [out] Pointer to entity related to ent1 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity set related to specified entity and relation handle
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getEntSetRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle ent1,
        /**< [in] entity of relation being queried */
    int switch_order,
        /**< [in] 1st entity is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    iBase_EntitySetHandle* entset2,
        /**< [out] Pointer to entity set related to ent1 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity related to specified entity set and relation handle
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getSetEntRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle entset1,
        /**< [in] entity set of relation being queried */
    int switch_order,
        /**< [in] 1st entity is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    iBase_EntityHandle* ent2,
        /**< [out] Pointer to entity related to entset1 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity set related to specified entity set and relation handle
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getSetSetRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle entset1,
        /**< [in] 1st entity set of relation being queried */
    int switch_order,
        /**< [in] 1st entity is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    iBase_EntitySetHandle* entset2,
        /**< [out] Pointer to entity set related to entset1 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity iterator related to specified entity set and relation
 * handle.
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getEntSetIterRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle ent1,
        /**< [in] ent1 1st entity set of relation being queried */
    int switch_order,
        /**< [in] 1st entity is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    iBase_EntityIterator* entIter,
        /**< [out] Returned entity iterator */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entities related to those in specified array and relation,
 * pairwise
 *
 * Get entities related to those in specified array and relation, pairwise.
 * Returns sets or entities, depending on relation type and entities in 
 * ent_array_1.  It is an error to get a relation for a side with status
 * iRel_NOTEXIST.
 ******************************************************************************/
void IREL_EXPORT iRel_getEntArrEntArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* ent_array_1,
        /**< [in] Array of entities whose relations are being queried */
    int ent_array_1_size,
        /**< [in] Number of entities in ent_array_1 */
    int switch_order,
        /**< [in] Entities in ent_array_1 are related with 1st (=0) or 2nd (=1)
        interface of this relation pair */
    iBase_EntityHandle** ent_array_2,
        /**< [inout] Pointer to array of entity handles returned from function */
    int* ent_array_2_allocated,
        /**< [inout] Pointer to allocated size of ent_array_2 */
    int* ent_array_2_size,
        /**< [out] Pointer to occupied size of ent_array_2 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity sets related to entities in specified array and relation,
 * pairwise
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getEntArrSetArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* ent_array_1,
        /**< [in] Array of entities whose relations are being queried */
    int ent_array_1_size,
        /**< [in] Number of entities in ent_array_1 */
    int switch_order,
        /**< [in] Entities in ent_array_1 are related with 1st (=0) or 2nd (=1)
        interface of this relation pair */
    iBase_EntitySetHandle** entset_array_2,
        /**< [inout] Pointer to array of entity set handles returned from function */
    int* entset_array_2_allocated,
        /**< [inout] Pointer to allocated size of entset_array_2 */
    int* entset_array_2_size,
        /**< [out] Pointer to occupied size of entset_array_2 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entities related to entity sets in specified array and relation,
 * pairwise
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getSetArrEntArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle* entset_array_1,
        /**< [in] Array of entity sets whose relations are being queried */
    int entset_array_1_size,
        /**< [in] Number of entity sets in entset_array_1 */
    int switch_order,
        /**< [in] Entities in ent_array_1 are related with 1st (=0) or 2nd (=1)
        interface of this relation pair */
    iBase_EntityHandle** ent_array_2,
        /**< [inout] Pointer to array of entity handles returned from function */
    int* ent_array_2_allocated,
        /**< [inout] Pointer to allocated size of ent_array_2 */
    int* ent_array_2_size,
        /**< [out] Pointer to occupied size of ent_array_2 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity sets related to entity sets in specified array and relation,
 * pairwise
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getSetArrSetArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle* entset_array_1,
        /**< [in] Array of entity sets whose relations are being queried */
    int entset_array_1_size,
        /**< [in] Number of entity sets in entset_array_1 */
    int switch_order,
        /**< [in] Entities in ent_array_1 are related with 1st (=0) or 2nd (=1)
        interface of this relation pair */
    iBase_EntitySetHandle** entset_array_2,
        /**< [inout] Pointer to array of entity handles returned from function */
    int* entset_array_2_allocated,
        /**< [inout] Pointer to allocated size of entset_array_2 */
    int* entset_array_2_size,
        /**< [out] Pointer to occupied size of entset_array_2 */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Get entity iterators related to entity sets in specified array and
 * relation.
 *
 * Description unavailable.
 ******************************************************************************/
void IREL_EXPORT iRel_getEntArrSetIterArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* ent_array_1,
        /**< [in] Array of entities whose relations are being queried */
    int ent_array_1_size,
        /**< [in] Number of entities in ent_array_1 */
    int switch_order,
        /**< [in] Entities in ent_array_1 are related with 1st (=0) or 2nd (=1)
        interface of this relation pair */
    iBase_EntityIterator** entiter,
        /**< [inout] Pointer to array of entity iterator handles returned from
        function */
    int* entiter_allocated,
        /**< [inout] Pointer to allocated size of entiter */
    int* entiter_size,
        /**< [out] Pointer to occupied size of entiter */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Remove a relation from an entity
 *
 * Remove a relation from an entity
 ******************************************************************************/
void IREL_EXPORT iRel_rmvEntRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle ent,
        /**< [in] entity of relation being removed */
    int switch_order,
        /**< [in] entity is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Remove a relation from an entity set
 *
 * Remove a relation from an entity set
 ******************************************************************************/
void IREL_EXPORT iRel_rmvSetRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle set,
        /**< [in] entity set of relation being removed */
    int switch_order,
        /**< [in] entity set is related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Remove a relation from an array of entities
 *
 * Remove a relation from an array of entities
 ******************************************************************************/
void IREL_EXPORT iRel_rmvEntArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* ent_array_1,
        /**< [in] Array of entities of relation being removed */
    int num_ent1,
        /**< [in] Number of entities in array */
    int switch_order,
        /**< [in] entities are related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Remove a relation from an array of entity sets
 *
 * Remove a relation from an array of entity sets
 ******************************************************************************/
void IREL_EXPORT iRel_rmvSetArrRelation(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntitySetHandle* entset_array_1,
        /**< [in] Array of entity sets of relation being removed */
    int num_entset1,
        /**< [in] Number of entity sets in array */
    int switch_order,
        /**< [in] entity sets are related to 1st interface (=0) or 2nd interface
        (=1) of relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Infer relations between entities in specified pair of interfaces
 *
 * Infer relations between entities in specified pair of interfaces.  The
 * criteria used to infer these relations depends on the interfaces in
 * the pair, the iRel implementation, and the source of the data in those
 * interfaces.
 ******************************************************************************/
void IREL_EXPORT iRel_inferAllRelations(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Infer relations and relation type between entities in specified pair
 * of interfaces 
 *
 * Infer relations between entities in specified pair of interfaces, and the
 * relation type used by this iRel implementation.  The criteria used to
 * infer these relations depends on the interfaces in the pair, the iRel
 * implementation, and the source of the data in those interfaces.
 ******************************************************************************/
void IREL_EXPORT iRel_inferAllRelationsAndType(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle* pair,
        /**< [in] Relation pair handle created by implementation */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Infer relations corresponding to specified entity and relation pair
 *
 * Infer relations corresponding to specified entity and relation pair.  The
 * criteria used to infer these relations depends on the interfaces in
 * the pair, the iRel implementation, and the source of the data in those
 * interfaces.
 ******************************************************************************/
void IREL_EXPORT iRel_inferEntRelations(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle entity,
        /**< [in] Entity whose relations are being inferred */
    int iface_no,
        /**< [in] Entity corresponds to 1st (=0) or 2nd (=1) interface in
        relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Brief unavailable
 *
 * Description unavailable
 ******************************************************************************/
void IREL_EXPORT iRel_inferSetRelations(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] description unknown */
    iBase_EntitySetHandle entity_set,
        /**< [in] description unknown */
    int iface_no,
        /**< [in] description unknown */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Infer relations corresponding to specified entities and relation pair
 *
 * Infer relations corresponding to specified entities and relation pair.
 * The criteria used to infer these relations depends on the interfaces in
 * the pair, the iRel implementation, and the source of the data in those
 * interfaces.
 ******************************************************************************/
void IREL_EXPORT iRel_inferEntArrRelations(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] Relation pair handle being queried */
    iBase_EntityHandle* entities,
        /**< [in] Array of entities whose relation are being inferred */
    int entities_size,
        /**< [in] Number of entities in array */
    int iface_no,
        /**< [in] Entities correspond to 1st (=0) or 2nd (=1) interface in
        relation pair */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);

/***************************************************************************//**
 * \brief Brief unavailable
 *
 * Description unavailable
 ******************************************************************************/
void IREL_EXPORT iRel_inferSetArrRelations(
    iRel_Instance instance,
        /**< [in] iRel instance handle */
    iRel_PairHandle pair,
        /**< [in] description unknown */
    iBase_EntitySetHandle* entity_sets,
        /**< [in] description unknown */
    int entities_size,
        /**< [in] description unknown */
    int iface_no,
        /**< [in] description unknown */
    int* err
        /**< [out] Returned Error status (see iBase_ErrorType) */
);


  /** \mainpage The ITAPS Relations Interface iRel
   *
   * Each ITAPS interface encapsulates functionality that "belongs"
   * together, for example mesh or geometric model functionality.  In
   * some cases, however, data in several of these interfaces need to
   * be related together.  For example, a collection of mesh faces
   * should be related to the geometric model face which they
   * discretize.  The ITAPS Relations interface accomplishes this in a
   * way which allows the lower-level interfaces to remain
   * independent.
   *
   * iRel defines relations as pairwise relations between entities
   * or entity sets.  Related entities can be in the same or different
   * interfaces.  A given relation is created for a given pair of
   * interfaces and returned in the form of a \em Relation \em Handle.
   * After a specific relation pair has been created, concrete
   * relations for that pair can be assigned and retrieved for
   * specific entities using set and get functions on the iRel
   * interface.  A given interface instance can appear in one or many
   * relation pairs, each identified by the relation pair handle.
   *
   * \section Types Relation Types
   *
   * Relations are also distinguished by a pair of relation types.
   * For each interface in a relation pair, a corresponding type
   * indicates whether the relation applies to entities, entity sets,
   * or both entities and sets in the corresponding interface in the
   * pair.  If only one of the interfaces in a given pair has a
   * 'both'-type, entities and entity sets in that
   * interface are each related to either entities or sets in the other
   * interface in the pair.  If both of the sides of a relation are of 
   * 'both'-type, entities and sets on one side of a relation point to 
   * sets on the other side.
   *
   * \section Status Relation Status
   *
   * Relations are also distinguished by a pair of relation statuses.
   * For each interface in a relation pair, a corresponding status indicates
   * whether the relation on that side is kept up to date, or stored at all.
   * Allowable values for status are iRel_ACTIVE, iRel_INACTIVE, and iRel_NOTEXIST,
   * defined in the iRel_RelationStatus enumeration.  Status for a given side
   * can be changed from iRel_ACTIVE to iRel_INACTIVE and vice versa, or from
   * either of those to iRel_NOTEXIST.  However, once changed to iRel_NOTEXIST
   * (or created that way), a side cannot be changed back to the other two.
   * Changing a side to be iRel_INACTIVE can be used when frequent changes to
   * the underlying entities are being made, e.g. during adaptive mesh refinement.
   * Changing from iRel_INACTIVE to iRel_ACTIVE implies a traversal of all entities
   * on the iRel_ACTIVE side to recover which entities on the iRel_INACTIVE side
   * must have their relations updated.
   *
   * \section ArgOrder Argument Order
   *
   * Many functions in the iRel interface take as input two entities,
   * or two lists of entities, along with a relation pair handle.  For
   * these functions, the entities or lists are assumed to be in the
   * same order as the interfaces used to create that relation pair.
   * For example, if a relation pair is created by calling:
   * \code 
   * iRel_createRelation(instance, iface1, ent_or_set1, type1, 
   *                     iface2, ent_or_set2, type2,
   *                     &relation_handle, &ierr)
   * \endcode
   * and relations set by calling
   * \code
   * iRel_setEntEntRelation(instance, relation_handle,
   *                        ent1, is_set1, ent2, is_set2, &ierr)
   * \endcode
   * it is assumed that ent1 is contained in iface1 and ent2 in
   * iface2.
   *
   * For functions taking only one entity or list as input, and
   * returning an entity or list, an additional argument indicates
   * whether the input entity or list belongs to the first or second
   * interface in that relation pair.
   * 
   */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef _ITAPS_iRel */
