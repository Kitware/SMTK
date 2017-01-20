/**
 * Copyright 2006 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Coroporation, the U.S. Government
 * retains certain rights in this software.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 */
#include "iRel_Lasso.hpp"

#include "Lasso.hpp"
#include "AssocPair.hpp"
#include "ArrayManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <vector>

const bool debug = false;

void iRel_getErrorType(iRel_Instance instance, int *error_type)
{
  if (instance == NULL)
    *error_type = iBase_FAILURE;
  else
    *error_type = LASSOI->lastErrorType;
}

void iRel_getDescription(iRel_Instance instance, char *descr, int descr_len)
{
  if (instance == NULL) {
    strcpy(descr, "iRel_getDescription: Invalid instance");
  }
  else {
    unsigned int len = std::min(strlen(LASSOI->lastErrorDescription),
                                static_cast<size_t>(descr_len));
    strncpy(descr, LASSOI->lastErrorDescription, len);
    descr[len] = '\0';
  }
}

void iRel_create(/* in */ const char * /* options */,
                 iRel_Instance *instance,
                 int *err,
                 const int options_len)
{
  if (0 != options_len) {
    *instance = NULL;
    *err = iBase_NOT_SUPPORTED;
  }

  *instance = new Lasso();
  *err = iBase_SUCCESS;
}

void iRel_destroy(iRel_Instance instance, int *err)
{
  delete LASSOI;
  *err = iBase_SUCCESS;
}

void iRel_createPair (
  iRel_Instance instance,
  iBase_Instance iface1,
  const int ent_or_set1,
  const int iface_type1,
  const int irel_status1,
  iBase_Instance iface2,
  const int ent_or_set2,
  const int iface_type2,
  const int irel_status2,
  iRel_PairHandle *pair,
  int *err)
{
  AssocPair *assoc_pair = new AssocPair(
    instance,
    iface1, static_cast<iRel_RelationType>(ent_or_set1),
    static_cast<iRel_IfaceType>(iface_type1),
    static_cast<iRel_RelationStatus>(irel_status1),
    iface2, static_cast<iRel_RelationType>(ent_or_set2),
    static_cast<iRel_IfaceType>(iface_type2),
    static_cast<iRel_RelationStatus>(irel_status2)
  );
  LASSOI->insert_pair(assoc_pair);

  *pair = reinterpret_cast<iRel_PairHandle>(assoc_pair);
  RETURN(iBase_SUCCESS);
}

void iRel_getPairInfo (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_Instance *iface1,
  int *ent_or_set1,
  int *iface_type1,
  int *irel_status1,
  iBase_Instance *iface2,
  int *ent_or_set2,
  int *iface_type2,
  int *irel_status2,
  int *err)
{
  CHK_PAIR();

  *iface1       = ASSOCPAIRI->iface_instance(0);
  *ent_or_set1  = ASSOCPAIRI->relation_type(0);
  *iface_type1  = ASSOCPAIRI->iface_type(0);
  *irel_status1 = ASSOCPAIRI->relation_status(0);
  *iface2       = ASSOCPAIRI->iface_instance(1);
  *iface_type2  = ASSOCPAIRI->iface_type(1);
  *ent_or_set2  = ASSOCPAIRI->relation_type(1);
  *irel_status2 = ASSOCPAIRI->relation_status(1);

  RETURN(iBase_SUCCESS);
}

void iRel_changePairType (
  iRel_Instance instance,
  iRel_PairHandle pair,
  int ent_or_set1,
  int ent_or_set2,
  int *err)
{
  CHK_PAIR();

  CHK_ERROR( ASSOCPAIRI->change_type(0, static_cast<iRel_RelationType>(
                                     ent_or_set1)) );
  CHK_ERROR( ASSOCPAIRI->change_type(1, static_cast<iRel_RelationType>(
                                     ent_or_set2)) );
}

void iRel_changePairStatus (
  iRel_Instance instance,
  iRel_PairHandle pair,
  int irel_status1,
  int irel_status2,
  int *err)
{
  CHK_PAIR();

  CHK_ERROR( ASSOCPAIRI->change_status(0, static_cast<iRel_RelationStatus>(
                                       irel_status1)) );
  CHK_ERROR( ASSOCPAIRI->change_status(1, static_cast<iRel_RelationStatus>(
                                       irel_status2)) );
}

void iRel_destroyPair (
  iRel_Instance instance,
  iRel_PairHandle pair,
  int *err)
{
  CHK_PAIR();

  CHK_ERROR( LASSOI->erase_pair(ASSOCPAIRI) );
}

void iRel_findPairs (
  iRel_Instance instance,
  iBase_Instance iface,
  iRel_PairHandle **pairs,
  int *pairs_allocated,
  int *pairs_size,
  int *err
  )
{
  std::vector<AssocPair*> tmp_pairs;
  LASSOI->find_pairs(iface, tmp_pairs);

  ALLOC_CHECK_ARRAY_NOFAIL(pairs, tmp_pairs.size());
  for (size_t i=0; i<tmp_pairs.size(); ++i) {
    (*pairs)[i] = reinterpret_cast<iRel_PairHandle>(tmp_pairs[i]);
  }

  RETURN(iBase_SUCCESS);
}

void iRel_setEntEntRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle ent1,
  iBase_EntityHandle ent2,
  int *err)
{
  CHK_PAIR();
  CHK_ERROR( ASSOCPAIRI->set_relation(ent1, ent2) );
}

void iRel_setEntSetRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle ent1,
  iBase_EntitySetHandle set2,
  int *err)
{
  CHK_PAIR();
  CHK_ERROR( ASSOCPAIRI->set_relation(ent1, set2) );
}

void iRel_setSetEntRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle set1,
  iBase_EntityHandle ent2,
  int *err)
{
  CHK_PAIR();
  CHK_ERROR( ASSOCPAIRI->set_relation(set1, ent2) );
}

void iRel_setSetSetRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle set1,
  iBase_EntitySetHandle set2,
  int *err)
{
  CHK_PAIR();
  CHK_ERROR( ASSOCPAIRI->set_relation(set1, set2) );
}

void iRel_setEntArrEntArrRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *ent_array_1,
  int num_entities1,
  iBase_EntityHandle *ent_array_2,
  int num_entities2,
  int *err)
{
  CHK_PAIR();

  if (num_entities1 != num_entities2)
    ERROR(iBase_INVALID_ENTITY_COUNT, "setEntArrEntArrRelation doesn't support "
          "different #'s of entities.");

  int result = iBase_SUCCESS;
  char descr[200];
  for (int i = 0; i < num_entities1; i++) {
    int tmp_result = ASSOCPAIRI->set_relation(ent_array_1[i], ent_array_2[i]);
    if (result == iBase_SUCCESS && tmp_result != iBase_SUCCESS) {
      result = tmp_result;
      iRel_getDescription(instance, descr, sizeof(descr));
    }
  }

  if (result != iBase_SUCCESS)
    ERROR(result, descr);
  RETURN(iBase_SUCCESS);
}

void iRel_setEntArrSetArrRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *ent_array_1,
  int num_entities1,
  iBase_EntitySetHandle *set_array_2,
  int num_sets2,
  int *err)
{
  CHK_PAIR();

  if (num_entities1 != num_sets2)
    ERROR(iBase_INVALID_ENTITY_COUNT, "setEntArrSetArrRelation doesn't support "
          "different #'s of entities.");

  int result = iBase_SUCCESS;
  char descr[200];
  for (int i = 0; i < num_entities1; i++) {
    int tmp_result = ASSOCPAIRI->set_relation(ent_array_1[i], set_array_2[i]);
    if (result == iBase_SUCCESS && tmp_result != iBase_SUCCESS) {
      result = tmp_result;
      iRel_getDescription(instance, descr, sizeof(descr));
    }
  }

  if (result != iBase_SUCCESS)
    ERROR(result, descr);
  RETURN(iBase_SUCCESS);
}

void iRel_setSetArrEntArrRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle *set_array_1,
  int num_sets1,
  iBase_EntityHandle *ent_array_2,
  int num_entities2,
  int *err)
{
  CHK_PAIR();

  if (num_sets1 != num_entities2)
    ERROR(iBase_INVALID_ENTITY_COUNT, "setSetArrEntArrRelation doesn't support "
          "different #'s of entities.");

  int result = iBase_SUCCESS;
  char descr[200];
  for (int i = 0; i < num_sets1; i++) {
    int tmp_result = ASSOCPAIRI->set_relation(set_array_1[i], ent_array_2[i]);
    if (result == iBase_SUCCESS && tmp_result != iBase_SUCCESS) {
      result = tmp_result;
      iRel_getDescription(instance, descr, sizeof(descr));
    }
  }

  if (result != iBase_SUCCESS)
    ERROR(result, descr);
  RETURN(iBase_SUCCESS);
}

void iRel_setSetArrSetArrRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle *set_array_1,
  int num_sets1,
  iBase_EntitySetHandle *set_array_2,
  int num_sets2,
  int *err)
{
  CHK_PAIR();

  if (num_sets1 != num_sets2)
    ERROR(iBase_INVALID_ENTITY_COUNT, "setSetArrSetArrRelation doesn't support "
          "different #'s of entities.");

  int result = iBase_SUCCESS;
  char descr[200];
  for (int i = 0; i < num_sets1; i++) {
    int tmp_result = ASSOCPAIRI->set_relation(set_array_1[i], set_array_2[i]);
    if (result == iBase_SUCCESS && tmp_result != iBase_SUCCESS) {
      result = tmp_result;
      iRel_getDescription(instance, descr, sizeof(descr));
    }
  }

  if (result != iBase_SUCCESS)
    ERROR(result, descr);
  RETURN(iBase_SUCCESS);
}

void iRel_getEntEntRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle ent1,
  int switch_order,
  iBase_EntityHandle *ent2,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, &ent1, 1, ent2) );
}

void iRel_getEntSetRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle ent1,
  int switch_order,
  iBase_EntitySetHandle *set2,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, &ent1, 1, set2) );
}

void iRel_getSetEntRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle set1,
  int switch_order,
  iBase_EntityHandle *ent2,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, &set1, 1, ent2) );
}

void iRel_getSetSetRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle set1,
  int switch_order,
  iBase_EntitySetHandle *set2,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, &set1, 1, set2) );
}

void iRel_getEntSetIterRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle ent1,
  int switch_order,
  iBase_EntityIterator *entset2,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, &ent1, 1, entset2) );
}

void iRel_getEntArrEntArrRelation(
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *ent_array_1,
  int ent_array_1_size,
  int switch_order,
  iBase_EntityHandle **ent_array_2,
  int *ent_array_2_allocated,
  int *ent_array_2_size,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  ALLOC_CHECK_ARRAY(ent_array_2, ent_array_1_size);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, ent_array_1, ent_array_1_size,
                                        *ent_array_2) );

  KEEP_ARRAY(ent_array_2);
  RETURN(iBase_SUCCESS);
}

void iRel_getEntArrSetArrRelation(
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *ent_array_1,
  int ent_array_1_size,
  int switch_order,
  iBase_EntitySetHandle **set_array_2,
  int *set_array_2_allocated,
  int *set_array_2_size,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  ALLOC_CHECK_ARRAY(set_array_2, ent_array_1_size);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, ent_array_1, ent_array_1_size,
                                        *set_array_2) );

  KEEP_ARRAY(set_array_2);
  RETURN(iBase_SUCCESS);
}

void iRel_getSetArrEntArrRelation(
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle *set_array_1,
  int set_array_1_size,
  int switch_order,
  iBase_EntityHandle **ent_array_2,
  int *ent_array_2_allocated,
  int *ent_array_2_size,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  ALLOC_CHECK_ARRAY(ent_array_2, set_array_1_size);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, set_array_1, set_array_1_size,
                                        *ent_array_2) );

  KEEP_ARRAY(ent_array_2);
  RETURN(iBase_SUCCESS);
}

void iRel_getSetArrSetArrRelation(
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle *set_array_1,
  int set_array_1_size,
  int switch_order,
  iBase_EntitySetHandle **set_array_2,
  int *set_array_2_allocated,
  int *set_array_2_size,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  ALLOC_CHECK_ARRAY(set_array_2, set_array_1_size);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, set_array_1, set_array_1_size,
                                        *set_array_2) );

  KEEP_ARRAY(set_array_2);
  RETURN(iBase_SUCCESS);
}

void iRel_getEntArrSetIterArrRelation (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *ent_array_1,
  int ent_array_1_size,
  int switch_order,
  iBase_EntityIterator **entiter,
  int *entiter_allocated,
  int *entiter_size,
  int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);;
  ALLOC_CHECK_ARRAY(entiter, ent_array_1_size);
  CHK_ERROR( ASSOCPAIRI->get_relation(iface_no, ent_array_1, ent_array_1_size,
                                        *entiter) );

  KEEP_ARRAY(entiter);
  RETURN(iBase_SUCCESS);
}


void iRel_rmvEntRelation(
  iRel_Instance instance,
  /*in*/ iRel_PairHandle pair,
  /*in*/ iBase_EntityHandle ent,
  /*in*/ int switch_order,
  /*out*/ int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->rmv_relation(iface_no, &ent, 1) );
}

void iRel_rmvSetRelation(
  iRel_Instance instance,
  /*in*/ iRel_PairHandle pair,
  /*in*/ iBase_EntitySetHandle entset,
  /*in*/ int switch_order,
  /*out*/ int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->rmv_relation(iface_no, &entset, 1) );
}

void iRel_rmvEntArrRelation(
  iRel_Instance instance,
  /*in*/ iRel_PairHandle pair,    
  /*in*/ iBase_EntityHandle *ent_array,
  /*in*/ int num_ent,
  /*in*/ int switch_order,
  /*out*/ int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->rmv_relation(iface_no, ent_array, num_ent) );
}

void iRel_rmvSetArrRelation(
  iRel_Instance instance,
  /*in*/ iRel_PairHandle pair,    
  /*in*/ iBase_EntitySetHandle *entset_array,
  /*in*/ int num_ent,
  /*in*/ int switch_order,
  /*out*/ int *err)
{
  CHK_PAIR();

  int iface_no = (switch_order ? 1 : 0);
  CHK_ERROR( ASSOCPAIRI->rmv_relation(iface_no, entset_array, num_ent) );
}

static int
get_gids_and_dims(iRel_PairHandle pair,
                  int iface_no,
                  iBase_EntityHandle *ents,
                  int ents_size,
                  int ent_or_set,
                  std::vector<int> &ents_gids,
                  std::vector<int> &ents_dims)
{
  int result;
  iBase_EntitySetHandle *sets = reinterpret_cast<iBase_EntitySetHandle*>(ents);

  ents_gids.resize(ents_size);
  if (ent_or_set == iRel_ENTITY)
    result = ASSOCPAIRI->get_gids(iface_no, ents, ents_size, &ents_gids[0]);
  else
    result = ASSOCPAIRI->get_gids(iface_no, sets, ents_size, &ents_gids[0]);

  if (iBase_SUCCESS != result && iBase_TAG_NOT_FOUND != result)
    return result;

  ents_dims.resize(ents_size, -1);
  if (ent_or_set == iRel_ENTITY) {
    int *ents_dims_ptr = &ents_dims[0];
    int ents_dims_alloc = ents_dims.size(), ents_dims_size;
    result = ASSOCPAIRI->get_ents_dims(iface_no, ents, ents_size,
                                       &ents_dims_ptr, &ents_dims_alloc,
                                       &ents_dims_size);
  }
  else {
    result = ASSOCPAIRI->get_dims(iface_no, sets, ents_size, &ents_dims[0]);
  }

  if (iBase_SUCCESS != result && iBase_TAG_NOT_FOUND != result)
    return result;

  return iBase_SUCCESS;
}

static void
iRel_inferArrArrRelations(iRel_Instance instance,
                          iRel_PairHandle pair,
                          iBase_EntityHandle *ents1,
                          const int ents1_size,
                          int ent_or_set1,
                          iBase_EntityHandle *ents2,
                          const int ents2_size,
                          int ent_or_set2,
                          int *err)
{
  int result;

  std::vector<int> ents_gids, ents_dims;
  std::map<const int, iBase_EntityHandle> ents_gid_map[4];

  get_gids_and_dims(pair, 0, ents1, ents1_size, ent_or_set1, ents_gids,
                    ents_dims);
  for (int i = 0; i < ents1_size; i++) {
    int dim = ents_dims[i];
    if (0 <= dim && 3 >= dim)
      ents_gid_map[dim][ents_gids[i]] = ents1[i];
  }

  get_gids_and_dims(pair, 1, ents2, ents2_size, ent_or_set2, ents_gids,
                    ents_dims);
  for (int i = 0; i < ents2_size; i++) {
    int dim = ents_dims[i];

    // only check entities for which the dimension entry is in a reasonable
    // range
    if (0 > dim || 3 < dim) continue;

    // there's a match if there's an entity with that dimension with matching id
    std::map<const int, iBase_EntityHandle>::iterator iter =
      ents_gid_map[dim].find(ents_gids[i]);

      // if it matches, set the relation tags for those entities
    if (iter != ents_gid_map[dim].end()) {
      if (ent_or_set1 == iRel_ENTITY && ent_or_set2 == iRel_ENTITY) {
        result = ASSOCPAIRI->set_relation(
          (*iter).second,
          ents2[i]);
      }
      else if (ent_or_set1 != iRel_ENTITY && ent_or_set2 == iRel_ENTITY) {
        result = ASSOCPAIRI->set_relation(
          (iBase_EntitySetHandle)(*iter).second,
          ents2[i]);
      }
      else if (ent_or_set1 == iRel_ENTITY && ent_or_set2 != iRel_ENTITY) {
        result = ASSOCPAIRI->set_relation(
          (*iter).second,
          (iBase_EntitySetHandle)ents2[i]);
      }
      else { // ent_or_set1 != iRel_ENTITY && ent_or_set2 != iRel_ENTITY
        result = ASSOCPAIRI->set_relation(
          (iBase_EntitySetHandle)(*iter).second,
          (iBase_EntitySetHandle)ents2[i]);
      }

      CHK_ERROR(result);
    }
  }

  RETURN(iBase_SUCCESS);
}

void
iRel_inferEntArrEntArrRelations(iRel_Instance instance,
                                iRel_PairHandle pair,
                                iBase_EntityHandle *ents1,
                                const int ents1_size,
                                iBase_EntityHandle *ents2,
                                const int ents2_size,
                                int *err)
{
  iRel_inferArrArrRelations(instance, pair, ents1, ents1_size, 0,
                            ents2, ents2_size, 0, err);
}

void
iRel_inferEntArrSetArrRelations(iRel_Instance instance,
                                iRel_PairHandle pair,
                                iBase_EntityHandle *ents1,
                                const int ents1_size,
                                iBase_EntitySetHandle *ents2,
                                const int ents2_size,
                                int *err)
{
  iRel_inferArrArrRelations(instance, pair, ents1, ents1_size, 0,
                            (iBase_EntityHandle*)ents2, ents2_size, 1, err);
}

void
iRel_inferSetArrEntArrRelations(iRel_Instance instance,
                                iRel_PairHandle pair,
                                iBase_EntitySetHandle *ents1,
                                const int ents1_size,
                                iBase_EntityHandle *ents2,
                                const int ents2_size,
                                int *err)
{
  iRel_inferArrArrRelations(instance, pair, (iBase_EntityHandle*)ents1,
                            ents1_size, 1, ents2, ents2_size, 0, err);
}

void
iRel_inferSetArrSetArrRelations(iRel_Instance instance,
                                iRel_PairHandle pair,
                                iBase_EntitySetHandle *ents1,
                                const int ents1_size,
                                int /*is_set1*/,
                                iBase_EntitySetHandle *ents2,
                                const int ents2_size,
                                int /*is_set2*/,
                                int *err)

{
  iRel_inferArrArrRelations(instance, pair, (iBase_EntityHandle*)ents1,
                            ents1_size, 1, (iBase_EntityHandle*)ents2,
                            ents2_size, 1, err);
}

void iRel_inferAllRelations (
  iRel_Instance instance,
  iRel_PairHandle pair,
  int *err)
{
  CHK_PAIR();

  // get all entities in those interfaces
  int result;

  iBase_EntityHandle *ents1 = NULL;
  int ents1_alloc = 0, ents1_size;
  if (ASSOCPAIRI->relation_type(0) != iRel_ENTITY)
    result = ASSOCPAIRI->get_all_sets(0, (iBase_EntitySetHandle**)&ents1,
                                      &ents1_alloc, &ents1_size);
  else
    result = ASSOCPAIRI->get_all_entities(0, -1, &ents1, &ents1_alloc,
                                          &ents1_size);
  CHK_ERROR(result);

  iBase_EntityHandle *ents2 = NULL;
  int ents2_alloc = 0, ents2_size;
  if (ASSOCPAIRI->relation_type(1) != iRel_ENTITY)
    result = ASSOCPAIRI->get_all_sets(1, (iBase_EntitySetHandle**)&ents2,
                                     &ents2_alloc, &ents2_size);
  else
  result = ASSOCPAIRI->get_all_entities(1, -1, &ents2, &ents2_alloc,
                                       &ents2_size);
  CHK_ERROR(result);

  iRel_inferArrArrRelations(instance, pair,
                            ents1, ents1_size, ASSOCPAIRI->relation_type(0),
                            ents2, ents2_size, ASSOCPAIRI->relation_type(1),
                            &result);

  free(ents1); free(ents2);
  CHK_ERROR(result);
}

void iRel_inferAllRelationsAndType (
  iRel_Instance instance,
  iRel_PairHandle * /*pair*/,
  int *err)
{
  ERROR(iBase_NOT_SUPPORTED, "Not currently supported.");
}

void iRel_inferEntRelations (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle entity,
  int iface_no,
  int *err)
{
  iRel_inferEntArrRelations(instance, pair, &entity, 1, iface_no, err);
}

void iRel_inferSetRelations (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle entity,
  int iface_no,
  int *err)
{
  iRel_inferSetArrRelations(instance, pair, &entity, 1, iface_no, err);
}

static void iRel_inferArrRelations (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *entities,
  int entities_size,
  bool is_set,
  int iface_no,
  int *err)
{
  CHK_PAIR();

  if (0 > iface_no || 1 < iface_no) {
    ERROR(iBase_INVALID_ARGUMENT, "Interface number must be 0 or 1");
  }
  else if (( is_set && ASSOCPAIRI->relation_type(iface_no) == iRel_ENTITY) ||
           (!is_set && ASSOCPAIRI->relation_type(iface_no) != iRel_ENTITY)) {
    ERROR(iBase_INVALID_ARGUMENT, "is_set must match entOrSet in call to "
          "inferArrRelations");
  }

  // get all entities in iface2
  int result;
  iBase_EntityHandle *ents1 = entities;
  int ents1_size = entities_size;
  iBase_EntityHandle *ents2 = NULL;
  int ents2_alloc = 0, ents2_size;
  if (ASSOCPAIRI->relation_type(1-iface_no) != iRel_ENTITY)
    result = ASSOCPAIRI->get_all_sets(!iface_no,
                                     (iBase_EntitySetHandle**)&ents2,
                                     &ents2_alloc, &ents2_size);
  else
    result = ASSOCPAIRI->get_all_entities(!iface_no, -1,
                                         &ents2, &ents2_alloc, &ents2_size);
  CHK_ERROR(result);

  // switch so that entity lists always go into inferArrArrRelations in
  // forward order wrt pair
  if (1 == iface_no) {
    std::swap(ents1, ents2);
    std::swap(ents1_size, ents2_size);
  }

  iRel_inferArrArrRelations(instance, pair,
                            ents1, ents1_size,
                            ASSOCPAIRI->relation_type(0),
                            ents2, ents2_size,
                            ASSOCPAIRI->relation_type(1), &result);

  free(1 == iface_no ? ents1 : ents2);

  CHK_ERROR(result);
}

void iRel_inferEntArrRelations (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntityHandle *entities,
  int entities_size,
  int iface_no,
  int *err)
{
  iRel_inferArrRelations(instance, pair, entities, entities_size, false,
                         iface_no, err);
}

void iRel_inferSetArrRelations (
  iRel_Instance instance,
  iRel_PairHandle pair,
  iBase_EntitySetHandle *entities,
  int entities_size,
  int iface_no,
  int *err)
{
  iRel_inferArrRelations(instance, pair, (iBase_EntityHandle*)entities,
                         entities_size, true, iface_no, err);
}
