#include "AssocPair.hpp"

#include <cstdlib>

#include "Lasso.hpp"

#ifdef ENABLE_IGEOM
#include "GeomAssocPairSide.hpp"
#endif
#ifdef ENABLE_FBIGEOM
#include "FBGeomAssocPairSide.hpp"
#endif
#ifdef ENABLE_IMESH
#include "MeshAssocPairSide.hpp"
#endif

int AssocPair::currId = 0;

AssocPair::AssocPair(iRel_Instance p_instance,
                     iBase_Instance iface0,
                     iRel_RelationType ent_or_set0,
                     iRel_IfaceType type0,
                     iRel_RelationStatus p_status0,
                     iBase_Instance iface1,
                     iRel_RelationType ent_or_set1,
                     iRel_IfaceType type1,
                     iRel_RelationStatus p_status1)
  : instance(p_instance)
{
  pairId = currId++;

  iBase_Instance ifaces[] = {iface0, iface1};
  iRel_IfaceType types[] = {type0, type1};
  for (int i = 0; i < 2; i++) {
    switch (types[i]) {
#ifdef ENABLE_IGEOM
    case iRel_IGEOM_IFACE:
      relSides[i] = new GeomAssocPairSide(instance, ifaces[i], pairId);
      break;
#endif
#ifdef ENABLE_FBIGEOM
    case iRel_FBIGEOM_IFACE:
      relSides[i] = new FBGeomAssocPairSide(instance, ifaces[i], pairId);
      break;
#endif
#ifdef ENABLE_IMESH
    case iRel_IMESH_IFACE:
      relSides[i] = new MeshAssocPairSide(instance, ifaces[i], pairId);
      break;
#endif
    default:
      relSides[i] = NULL;
    }
  }

  entOrSet[0] = ent_or_set0;
  entOrSet[1] = ent_or_set1;
  relStatus[0] = p_status0;
  relStatus[1] = p_status1;
}

AssocPair::~AssocPair()
{
  for (int i = 0; i < 2; i++)
    delete relSides[i];
}

int AssocPair::get_all_entities(int iface_no, int dimension,
                                iBase_EntityHandle **entities,
                                int *entities_alloc, int *entities_size)
{
  return relSides[iface_no]->get_all_entities(dimension, entities,
                                              entities_alloc, entities_size);
}

int AssocPair::get_all_sets(int iface_no, iBase_EntitySetHandle **sets,
                            int *sets_alloc, int *sets_size)
{
  return relSides[iface_no]->get_all_sets(sets, sets_alloc, sets_size);
}

int AssocPair::get_entities(int iface_no, int dimension,
                            iBase_EntitySetHandle set_handle,
                            iBase_EntityHandle **entities, int *entities_alloc,
                            int *entities_size)
{
  return relSides[iface_no]->get_entities(dimension, set_handle, entities,
                                          entities_alloc, entities_size);
}

int AssocPair::get_ents_dims(int iface_no, iBase_EntityHandle *entities,
                             int entities_size, int **ent_types,
                             int *ent_types_alloc, int *ent_types_size)
{
  return relSides[iface_no]->get_ents_dims(entities, entities_size, ent_types,
                                           ent_types_alloc, ent_types_size);
}

int AssocPair::set_relation(iBase_EntityHandle ent1, iBase_EntityHandle ent2)
{
  if (entOrSet[0] == iRel_SET || entOrSet[1] == iRel_SET)
    ERRORR(iBase_FAILURE, "Invalid relation type");

  // check that if we're passing in an ent for a 'both'-type
  // assoc, there's already a set associated to the other ent
  iBase_EntityHandle tmp_ent;
  if (entOrSet[0] == iRel_BOTH &&
      relSides[1]->get_relation_side(&ent2, 1, &tmp_ent) != iBase_SUCCESS)
    ERRORR(iBase_FAILURE, "Couldn't find associated set on left side");
  if (entOrSet[1] == iRel_BOTH &&
      relSides[0]->get_relation_side(&ent1, 1, &tmp_ent) != iBase_SUCCESS)
    ERRORR(iBase_FAILURE, "Couldn't find associated set on right side");

  // set ent1 => ent2
  if (relStatus[0] == iRel_ACTIVE)
    CHK_ERRORR( relSides[0]->set_relation_side(&ent1, 1, &ent2) );

  // set ent1 <= ent2
  if (relStatus[1] == iRel_ACTIVE)
    CHK_ERRORR( relSides[1]->set_relation_side(&ent2, 1, &ent1) );

  RETURNR(iBase_SUCCESS);
}

int AssocPair::set_relation(iBase_EntityHandle ent1, iBase_EntitySetHandle set2)
{
  if (entOrSet[0] == iRel_SET || entOrSet[1] == iRel_ENTITY)
    ERRORR(iBase_FAILURE, "Invalid relation type");

  // check that if we're passing in an ent for a 'both'-type
  // assoc, there's already a set associated to the other ent
  iBase_EntityHandle tmp_ent;
  if (entOrSet[0] == iRel_BOTH &&
      relSides[1]->get_relation_side(&set2, 1, &tmp_ent) != iBase_SUCCESS)
    ERRORR(iBase_FAILURE, "Couldn't find associated set on left side");

  // set ent1 => set2
  if (relStatus[0] == iRel_ACTIVE)
    CHK_ERRORR( relSides[0]->set_relation_side(&ent1, 1, &set2) );

  // set ent1 <= set2
  if (relStatus[1] == iRel_ACTIVE)
    CHK_ERRORR( relSides[1]->set_relation_side(&set2, 1, &ent1) );

  // if the right side is a 'both'-type association, set the contents of set2
  // to point to ent1 as well
  if (entOrSet[1] == iRel_BOTH)
    CHK_ERRORR( populate_recursive(1, set2, ent1) );

  RETURNR(iBase_SUCCESS);
}

int AssocPair::set_relation(iBase_EntitySetHandle set1, iBase_EntityHandle ent2)
{
  if (entOrSet[0] == iRel_ENTITY || entOrSet[1] == iRel_SET)
    ERRORR(iBase_FAILURE, "Invalid relation type");

  // check that if we're passing in an ent for a 'both'-type
  // assoc, there's already a set associated to the other ent
  iBase_EntityHandle tmp_ent;
  if (entOrSet[1] == iRel_BOTH &&
      relSides[0]->get_relation_side(&set1, 1, &tmp_ent) != iBase_SUCCESS)
    ERRORR(iBase_FAILURE, "Couldn't find associated set on right side");

  // set set1 => ent2
  if (relStatus[0] == iRel_ACTIVE)
    CHK_ERRORR( relSides[0]->set_relation_side(&set1, 1, &ent2) );

  // set ent1 <= set2
  if (relStatus[1] == iRel_ACTIVE)
    CHK_ERRORR( relSides[1]->set_relation_side(&ent2, 1, &set1) );

  // if the left side is a 'both'-type association, set the contents of set1
  // to point to ent2 as well
  if (entOrSet[0] == iRel_BOTH)
    CHK_ERRORR( populate_recursive(0, set1, ent2) );

  RETURNR(iBase_SUCCESS);
}

int AssocPair::set_relation(iBase_EntitySetHandle set1,
                            iBase_EntitySetHandle set2)
{
  if (entOrSet[0] == iRel_ENTITY || entOrSet[1] == iRel_ENTITY)
    ERRORR(iBase_FAILURE, "Invalid relation type");

  // set set1 => set2
  if (relStatus[0] == iRel_ACTIVE)
    CHK_ERRORR( relSides[0]->set_relation_side(&set1, 1, &set2) );

  // set set1 <= set2
  if (relStatus[1] == iRel_ACTIVE)
    CHK_ERRORR( relSides[1]->set_relation_side(&set2, 1, &set1) );

  // if either side is a 'both'-type association, set the contents of set1
  // to point to set2 as well (and/or vice-versa)
  if (entOrSet[0] == iRel_BOTH)
    CHK_ERRORR( populate_recursive(0, set1, set2) );
  if (entOrSet[1] == iRel_BOTH)
    CHK_ERRORR( populate_recursive(1, set2, set1) );

  RETURNR(iBase_SUCCESS);
}

int AssocPair::get_relation(int iface_no, iBase_EntityHandle *entities,
                            int num_entities, iBase_EntityHandle *tag_values)
{
  if (relStatus[iface_no] == iRel_NOTEXIST)
    ERRORR(iBase_FAILURE, "Relation does not exist on this side");
  if (entOrSet[!iface_no] != iRel_ENTITY) // other iface is sets
    ERRORR(iBase_INVALID_ENTITY_HANDLE, "Expected EntitySet, got Entity");

  return relSides[iface_no]->get_relation_side(entities, num_entities,
                                               tag_values);
}

int AssocPair::get_relation(int iface_no, iBase_EntitySetHandle *sets,
                            int num_sets, iBase_EntityHandle *tag_values)
{
  if (relStatus[iface_no] == iRel_NOTEXIST)
    ERRORR(iBase_FAILURE, "Relation does not exist on this side");
  if (entOrSet[!iface_no] != iRel_ENTITY) // other iface is sets
    ERRORR(iBase_INVALID_ENTITY_HANDLE, "Expected EntitySet, got Entity");

  return relSides[iface_no]->get_relation_side(sets, num_sets, tag_values);
}

int AssocPair::get_relation(int iface_no, iBase_EntityHandle *entities,
                            int num_entities, iBase_EntitySetHandle *tag_values)
{
  if (relStatus[iface_no] == iRel_NOTEXIST)
    ERRORR(iBase_FAILURE, "Relation does not exist on this side");
  if (entOrSet[!iface_no] == iRel_ENTITY) // other iface is not sets
    ERRORR(iBase_INVALID_ENTITY_HANDLE, "Expected Entity, got EntitySet");

  return relSides[iface_no]->get_relation_side(entities, num_entities,
                                               tag_values);
}

int AssocPair::get_relation(int iface_no, iBase_EntitySetHandle *sets,
                            int num_sets, iBase_EntitySetHandle *tag_values)
{
  if (relStatus[iface_no] == iRel_NOTEXIST)
    ERRORR(iBase_FAILURE, "Relation does not exist on this side");
  if (entOrSet[!iface_no] == iRel_ENTITY) // other iface is not sets
    ERRORR(iBase_INVALID_ENTITY_HANDLE, "Expected Entity, got EntitySet");

  return relSides[iface_no]->get_relation_side(sets, num_sets, tag_values);
}

int AssocPair::get_relation(int iface_no, iBase_EntityHandle *entities,
                            int num_entities, iBase_EntityIterator *tag_values)
{
  std::vector<iBase_EntitySetHandle> sets(num_entities);
  CHK_ERRORR( get_relation(iface_no, entities, num_entities, &sets[0]) );

  for(int i=0; i<num_entities; i++)
    CHK_ERRORR( relSides[i]->get_iterator(sets[i], &tag_values[i]) );

  RETURNR(iBase_SUCCESS);
}

int AssocPair::get_relation(int iface_no, iBase_EntitySetHandle *sets,
                            int num_sets, iBase_EntityIterator *tag_values)
{
  std::vector<iBase_EntitySetHandle> sets2(num_sets);
  CHK_ERRORR( get_relation(iface_no, sets, num_sets, &sets2[0]) );

  for(int i=0; i<num_sets; i++)
    CHK_ERRORR( relSides[iface_no]->get_iterator(sets2[i], &tag_values[i]) );

  RETURNR(iBase_SUCCESS);
}

int AssocPair::rmv_relation(int iface_no, iBase_EntityHandle *entities,
                            int num_entities)
{
  if (relStatus[iface_no] == iRel_NOTEXIST)
    ERRORR(iBase_FAILURE, "Relation does not exist on this side");

  // TODO: handle "both" case

  // Remove the opposite side first
  if (relStatus[!iface_no] == iRel_ACTIVE) {
    if (entOrSet[!iface_no] == iRel_ENTITY) {
      std::vector<iBase_EntityHandle> other_entities(num_entities);
      CHK_ERRORR( get_relation(iface_no, entities, num_entities,
                               &other_entities[0]) );
      CHK_ERRORR( relSides[!iface_no]->rmv_relation_side(&other_entities[0],
                                                         num_entities) );
    }
    else {
      std::vector<iBase_EntitySetHandle> other_sets(num_entities);
      CHK_ERRORR( get_relation(iface_no, entities, num_entities,
                               &other_sets[0]) );
      CHK_ERRORR( relSides[!iface_no]->rmv_relation_side(&other_sets[0],
                                                         num_entities) );
    }
  }

  return relSides[iface_no]->rmv_relation_side(entities, num_entities);
}

int AssocPair::rmv_relation(int iface_no, iBase_EntitySetHandle *sets,
                            int num_sets)
{
  if (relStatus[iface_no] == iRel_NOTEXIST)
    ERRORR(iBase_FAILURE, "Relation does not exist on this side");

  // TODO: handle "both" case

  // Remove the opposite side first
  if (relStatus[!iface_no] == iRel_ACTIVE) {
    if (entOrSet[!iface_no] == iRel_ENTITY) {
      std::vector<iBase_EntityHandle> other_entities(num_sets);
      CHK_ERRORR( get_relation(iface_no, sets, num_sets, &other_entities[0]) );
      CHK_ERRORR( relSides[!iface_no]->rmv_relation_side(&other_entities[0],
                                                         num_sets) );
    }
    else {
      std::vector<iBase_EntitySetHandle> other_sets(num_sets);
      CHK_ERRORR( get_relation(iface_no, sets, num_sets, &other_sets[0]) );
      CHK_ERRORR( relSides[!iface_no]->rmv_relation_side(&other_sets[0],
                                                         num_sets) );
    }
  }

  return relSides[iface_no]->rmv_relation_side(sets, num_sets);
}

int AssocPair::get_gids(int iface_no, iBase_EntityHandle *entities,
                        int num_entities, int *tag_values)
{
  return relSides[iface_no]->get_gids(entities, num_entities, tag_values);
}

int AssocPair::get_gids(int iface_no, iBase_EntitySetHandle *sets,
                        int num_sets, int *tag_values)
{
  return relSides[iface_no]->get_gids(sets, num_sets, tag_values);
}

int AssocPair::get_dims(int iface_no, iBase_EntityHandle *entities,
                        int num_entities, int *tag_values)
{
  return relSides[iface_no]->get_dims(entities, num_entities, tag_values);
}

int AssocPair::get_dims(int iface_no, iBase_EntitySetHandle *sets,
                        int num_sets, int *tag_values)
{
  return relSides[iface_no]->get_dims(sets, num_sets, tag_values);
}

int AssocPair::change_type(int iface_no, iRel_RelationType type)
{
  if (entOrSet[iface_no] == type)
    RETURNR(iBase_SUCCESS);
  if (entOrSet[iface_no] == iRel_ENTITY || type == iRel_ENTITY)
    ERRORR(iBase_FAILURE, "Can only change type from \"set\" to \"both\", or "
           "vice-versa");

  entOrSet[iface_no] = type;
  if (relStatus[iface_no] != iRel_ACTIVE)
    RETURNR(iBase_SUCCESS);

  iBase_EntitySetHandle *sets = NULL;
  int set_alloc = 0, set_size;
  CHK_ERRORR( relSides[iface_no]->get_related_sets(&sets, &set_alloc,
                                                   &set_size) );
  if (type == iRel_BOTH) {
    if (entOrSet[!iface_no] == iRel_ENTITY) {
      std::vector<iBase_EntityHandle> related_ents(set_size);
      CHK_ERRORR( relSides[iface_no]->get_relation_side(sets, set_size,
                                                        &related_ents[0]) );

      for (int i = 0; i < set_size; i++)
        CHK_ERRORR( populate_recursive(iface_no, sets[i], related_ents[i]) );
    }
    else {
      std::vector<iBase_EntitySetHandle> related_sets(set_size);
      CHK_ERRORR( relSides[iface_no]->get_relation_side(sets, set_size,
                                                        &related_sets[0]) );

      for (int i = 0; i < set_size; i++)
        CHK_ERRORR( populate_recursive(iface_no, sets[i], related_sets[i]) );
    }
  }
  else if (type == iRel_SET) {
    for (int i = 0; i < set_size; i++)
      CHK_ERRORR( unpopulate_recursive(iface_no, sets[i]) );
  }

  free(sets);
  RETURNR(iBase_SUCCESS);
}

int AssocPair::change_status(int iface_no, iRel_RelationStatus status)
{
  if (relStatus[iface_no] == status)
    RETURNR(iBase_SUCCESS);

  relStatus[iface_no] = status;

  if (status == iRel_NOTEXIST) {
    // Destroy the assoc tag
    CHK_ERRORR( relSides[iface_no]->destroy_relation_side() );
  }
  else if (status == iRel_INACTIVE) {
    // Create the assoc tag
    CHK_ERRORR( relSides[iface_no]->create_relation_side() );
  }
  // Update the assoc tag
  else if (status == iRel_ACTIVE) {
    CHK_ERRORR( relSides[iface_no]->destroy_relation_side() );
    CHK_ERRORR( relSides[iface_no]->create_relation_side() );

    if (entOrSet[!iface_no] == iRel_ENTITY) {
      iBase_EntityHandle *entities = NULL;
      int ent_alloc = 0, ent_size;

      CHK_ERRORR( relSides[!iface_no]->get_related_ents(&entities, &ent_alloc,
                                                        &ent_size) );
      if (entOrSet[iface_no] == iRel_ENTITY) {
        std::vector<iBase_EntityHandle> related_ents(ent_size);
        int result = relSides[!iface_no]->get_relation_side(
          entities, ent_size, &related_ents[0]);

        if (result == iBase_SUCCESS) {
          if (iface_no == 0)
            for (int i = 0; i < ent_size; i++)
              CHK_ERRORR( set_relation(related_ents[i], entities[i]) );
          else
            for (int i = 0; i < ent_size; i++)
              CHK_ERRORR( set_relation(entities[i], related_ents[i]) );
        }
      }
      else {
        std::vector<iBase_EntitySetHandle> related_sets(ent_size);
        int result = relSides[!iface_no]->get_relation_side(
          entities, ent_size, &related_sets[0]);

        if (result == iBase_SUCCESS) {
          if (iface_no == 0)
            for (int i = 0; i < ent_size; i++)
              CHK_ERRORR( set_relation(related_sets[i], entities[i]) );
          else
            for (int i = 0; i < ent_size; i++)
              CHK_ERRORR( set_relation(entities[i], related_sets[i]) );
        }
      }
    
      free(entities);
    }
    else {
      iBase_EntitySetHandle *sets = NULL;
      int set_alloc = 0, set_size;

      CHK_ERRORR( relSides[!iface_no]->get_related_sets(&sets, &set_alloc,
                                                        &set_size) );
      if (entOrSet[iface_no] == iRel_ENTITY) {
        std::vector<iBase_EntityHandle> related_ents(set_size);
        int result = relSides[!iface_no]->get_relation_side(
          sets, set_size, &related_ents[0]);

        if (result == iBase_SUCCESS) {
          if (iface_no == 0)
            for (int i = 0; i < set_size; i++)
              CHK_ERRORR( set_relation(related_ents[i], sets[i]) );
          else
            for (int i = 0; i < set_size; i++)
              CHK_ERRORR( set_relation(sets[i], related_ents[i]) );
        }
      }
      else {
        std::vector<iBase_EntitySetHandle> related_sets(set_size);
        int result = relSides[!iface_no]->get_relation_side(
          sets, set_size, &related_sets[0]);

        if (result == iBase_SUCCESS) {
          if (iface_no == 0)
            for (int i = 0; i < set_size; i++)
              CHK_ERRORR( set_relation(related_sets[i], sets[i]) );
          else
            for (int i = 0; i < set_size; i++)
              CHK_ERRORR( set_relation(sets[i], related_sets[i]) );
        }
      }
    
      free(sets);
    }
  }
  else {
    ERRORR(iBase_INVALID_ARGUMENT, "Invalid argument for relation status");
  }

  RETURNR(iBase_SUCCESS);
}

bool AssocPair::equivalent(iBase_Instance iface0, iBase_Instance iface1,
                           bool *order_switched)
{
  if (iface0 == relSides[0]->instance() &&
      iface1 == relSides[1]->instance()) {
    if (order_switched) *order_switched = false;
    return true;
  }
  else if (iface0 == relSides[1]->instance() &&
           iface1 == relSides[0]->instance()) {
    if (order_switched) *order_switched = true;
    return true;
  }
  else
    return false;
}

bool AssocPair::equivalent(iRel_IfaceType type0, iRel_IfaceType type1,
                           bool *order_switched)
{
  if (type0 == relSides[0]->type() &&
      type1 == relSides[1]->type()) {
    if (order_switched) *order_switched = false;
    return true;
  }
  else if (type0 == relSides[1]->type() &&
           type1 == relSides[0]->type()) {
    if (order_switched) *order_switched = true;
    return true;
  }
  else
    return false;
}

bool AssocPair::contains(iBase_Instance iface)
{
  return (iface == relSides[0]->instance() ||
          iface == relSides[1]->instance());
}

int AssocPair::populate_recursive(int iface_no, iBase_EntitySetHandle set,
                                  iBase_EntityHandle related_ent)
{
  iBase_EntityHandle *entities = NULL;
  int entities_alloc = 0, entities_size;

  CHK_ERRORR( relSides[iface_no]->get_entities(-1, set, &entities,
                                               &entities_alloc,
                                               &entities_size) );

  for (int i = 0; i < entities_size; i++)
    CHK_ERRORR( relSides[iface_no]->set_relation_side(entities+i, 1,
                                                      &related_ent) );

  free(entities);
  RETURNR(iBase_SUCCESS);
}

int AssocPair::populate_recursive(int iface_no, iBase_EntitySetHandle set,
                                  iBase_EntitySetHandle related_set)
{
  iBase_EntityHandle *entities = NULL;
  int entities_alloc, entities_size;

  CHK_ERRORR( relSides[iface_no]->get_entities(-1, set, &entities,
                                               &entities_alloc,
                                               &entities_size) );

  for (int i = 0; i < entities_size; i++)
    CHK_ERRORR( relSides[iface_no]->set_relation_side(entities+i, 1,
                                                      &related_set) );

  free(entities);
  RETURNR(iBase_SUCCESS);
}

int AssocPair::unpopulate_recursive(int iface_no, iBase_EntitySetHandle set)
{
  iBase_EntityHandle *entities = NULL;
  int entities_alloc = 0, entities_size;

  CHK_ERRORR( relSides[iface_no]->get_entities(-1, set, &entities,
                                               &entities_alloc,
                                               &entities_size) );
  CHK_ERRORR( relSides[iface_no]->rmv_relation_side(entities, entities_size) );

  free(entities);
  RETURNR(iBase_SUCCESS);
}
