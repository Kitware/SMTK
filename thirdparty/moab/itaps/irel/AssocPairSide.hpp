#ifndef ASSOCPAIRSIDE_HPP
#define ASSOCPAIRSIDE_HPP

#include <iBase.h>
#include <iRel.h>

class AssocPairSide
{
public:
  virtual ~AssocPairSide();
  virtual iBase_Instance instance() const = 0;
  virtual iRel_IfaceType type() const = 0;

  virtual int create_relation_side() = 0;
  virtual int destroy_relation_side() = 0;

  virtual int get_all_entities(int dimension, iBase_EntityHandle **entities,
                               int *entities_alloc, int *entities_size) = 0;

  virtual int get_all_sets(iBase_EntitySetHandle **sets,
                           int *sets_alloc, int *sets_size) = 0;

  virtual int get_entities(int dimension, iBase_EntitySetHandle set_handle,
                           iBase_EntityHandle **entities,
                           int *entities_allocated, int *entities_size) = 0;

  virtual int get_ents_dims(iBase_EntityHandle *entities, int entities_size,
                            int **ent_types, int *ent_types_alloc,
                            int *ent_types_size) = 0;

  virtual int get_related_ents(iBase_EntityHandle **entities,
                               int *entities_alloc, int *entities_size) = 0;
  virtual int get_related_sets(iBase_EntitySetHandle **sets, int *sets_alloc,
                               int *sets_size) = 0;

  virtual int get_relation_side(iBase_EntityHandle *entities, int num_entities,
                                void *values) = 0;
  virtual int get_relation_side(iBase_EntitySetHandle *sets, int num_sets,
                                void *values) = 0;

  virtual int set_relation_side(iBase_EntityHandle *entities, int num_entities,
                                const void *values) = 0;
  virtual int set_relation_side(iBase_EntitySetHandle *sets, int num_sets,
                                const void *values) = 0;

  virtual int rmv_relation_side(iBase_EntityHandle *entities,
                                int num_entities) = 0;
  virtual int rmv_relation_side(iBase_EntitySetHandle *sets, int num_sets) = 0;

  virtual int get_iterator(iBase_EntitySetHandle set,
                           iBase_EntityIterator *iter) = 0;

  virtual int get_gids(iBase_EntityHandle *entities, int num_entities,
                       int *values) = 0;
  virtual int get_gids(iBase_EntitySetHandle *sets, int num_sets,
                       int *values) = 0;

  virtual int get_dims(iBase_EntityHandle *entities, int num_entities,
                       int *values) = 0;
  virtual int get_dims(iBase_EntitySetHandle *sets, int num_sets,
                       int *values) = 0;
};

inline
AssocPairSide::~AssocPairSide()
{
  // Nothing to do
}

#endif
