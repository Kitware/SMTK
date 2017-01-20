#ifndef ASSOCPAIR_HPP
#define ASSOCPAIR_HPP

#include "irel_export.h"
#include "iRel_Lasso.hpp"
#include "AssocPairSide.hpp"

#include <sstream>

class Lasso;

class IREL_EXPORT AssocPair
{
public:
  friend class Lasso;

  AssocPair(iRel_Instance instance,
            iBase_Instance iface0,
            iRel_RelationType ent_or_set0,
            iRel_IfaceType type0,
            iRel_RelationStatus status0,
            iBase_Instance iface1,
            iRel_RelationType ent_or_set1,
            iRel_IfaceType type1,
            iRel_RelationStatus status1);

  ~AssocPair();

  iBase_Instance iface_instance(int iface_no) const;
  iRel_IfaceType iface_type(int iface_no) const;
  iRel_RelationType relation_type(int iface_no) const;
  iRel_RelationStatus relation_status(int iface_no) const;

  int change_type(int iface_no, iRel_RelationType type);
  int change_status(int iface_no, iRel_RelationStatus status);

  bool equivalent(iBase_Instance iface1, iBase_Instance iface2,
                  bool *order_switched = NULL);
  bool equivalent(iRel_IfaceType type1, iRel_IfaceType type2,
                  bool *order_switched = NULL);

  bool contains(iBase_Instance iface);

  int get_all_entities(int iface_no, int dimension,
                       iBase_EntityHandle **entities, int *entities_alloc,
                       int *entities_size);

  int get_all_sets(int iface_no, iBase_EntitySetHandle **sets, int *sets_alloc,
                   int *sets_size);

  int get_entities(int iface_no, int dimension,
                   iBase_EntitySetHandle set_handle,
                   iBase_EntityHandle **entities, int *entities_alloc,
                   int *entities_size);

  int get_ents_dims(int iface_no, iBase_EntityHandle *entities,
                    int entities_size, int **ent_types, int *ent_types_alloc,
                    int *ent_types_size);

  int set_relation(iBase_EntityHandle    ent1, iBase_EntityHandle    ent2);
  int set_relation(iBase_EntitySetHandle set1, iBase_EntityHandle    ent2);
  int set_relation(iBase_EntityHandle    ent1, iBase_EntitySetHandle set2);
  int set_relation(iBase_EntitySetHandle set1, iBase_EntitySetHandle set2);

  int get_relation(int iface_no, iBase_EntityHandle *entities,
                   int num_entities, iBase_EntityHandle *tag_values);
  int get_relation(int iface_no, iBase_EntitySetHandle *sets,
                   int num_sets, iBase_EntityHandle *tag_values);
  int get_relation(int iface_no, iBase_EntityHandle *entities,
                   int num_entities, iBase_EntitySetHandle *tag_values);
  int get_relation(int iface_no, iBase_EntitySetHandle *sets,
                   int num_sets, iBase_EntitySetHandle *tag_values);
  int get_relation(int iface_no, iBase_EntityHandle *entities,
                   int num_entities, iBase_EntityIterator *tag_values);
  int get_relation(int iface_no, iBase_EntitySetHandle *sets,
                   int num_sets, iBase_EntityIterator *tag_values);

  int rmv_relation(int iface_no, iBase_EntityHandle *entities,
                   int num_entities);
  int rmv_relation(int iface_no, iBase_EntitySetHandle *sets, int num_sets);

  int get_gids(int iface_no, iBase_EntityHandle *entities, int num_entities,
               int *tag_values);
  int get_gids(int iface_no, iBase_EntitySetHandle *sets, int num_sets,
               int *tag_values);

  int get_dims(int iface_no, iBase_EntityHandle *entities, int num_entities,
               int *tag_values);
  int get_dims(int iface_no, iBase_EntitySetHandle *sets, int num_sets,
               int *tag_values);
private:
  AssocPair();

  int populate_recursive(int iface_no, iBase_EntitySetHandle set,
                         iBase_EntityHandle related_ent);
  int populate_recursive(int iface_no, iBase_EntitySetHandle set,
                         iBase_EntitySetHandle related_set);

  int unpopulate_recursive(int iface_no, iBase_EntitySetHandle set);

  iRel_Instance instance;
  AssocPairSide *relSides[2];
  iRel_RelationType entOrSet[2];
  iRel_RelationStatus relStatus[2];
  int pairId;

  static int currId;
};

inline iBase_Instance AssocPair::iface_instance(int iface_no) const
{
  return relSides[iface_no]->instance();
}

inline iRel_IfaceType AssocPair::iface_type(int iface_no) const
{
  return relSides[iface_no]->type();
}

inline iRel_RelationType AssocPair::relation_type(int iface_no) const
{
  return entOrSet[iface_no];
}

inline iRel_RelationStatus AssocPair::relation_status(int iface_no) const
{
  return relStatus[iface_no];
}

static inline AssocPair *assocpair_handle(iRel_PairHandle pair)
{
  return reinterpret_cast<AssocPair*>(pair);
}
#define ASSOCPAIRI assocpair_handle(pair)


#endif
