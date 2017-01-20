#ifndef GEOMASSOCPAIRSIDE_HPP
#define GEOMASSOCPAIRSIDE_HPP

#include "irel_export.h"
#include "AssocPairSide.hpp"
#include <iGeom.h>

class IREL_EXPORT GeomAssocPairSide : public AssocPairSide
{
public:
  GeomAssocPairSide(iRel_Instance relation, iBase_Instance instance, int id);
  virtual ~GeomAssocPairSide();

  virtual iBase_Instance instance() const;
  virtual iRel_IfaceType type() const;

  virtual int create_relation_side();
  virtual int destroy_relation_side();

  virtual int get_all_entities(int dimension, iBase_EntityHandle **entities,
                               int *entities_alloc, int *entities_size);

  virtual int get_all_sets(iBase_EntitySetHandle **sets,
                           int *sets_alloc, int *sets_size);

  virtual int get_entities(int dimension, iBase_EntitySetHandle set_handle,
                           iBase_EntityHandle **entities,
                           int *entities_alloc, int *entities_size);

  virtual int get_ents_dims(iBase_EntityHandle *entities, int entities_size,
                            int **ent_types, int *ent_types_alloc,
                            int *ent_types_size);

  virtual int get_related_ents(iBase_EntityHandle **entities,
                               int *entities_alloc, int *entities_size);
  virtual int get_related_sets(iBase_EntitySetHandle **sets,
                               int *sets_alloc, int *sets_size);

  virtual int get_relation_side(iBase_EntityHandle *entities, int num_entities,
                                void *values);
  virtual int get_relation_side(iBase_EntitySetHandle *sets, int num_sets,
                                void *values);

  virtual int set_relation_side(iBase_EntityHandle *entities, int num_entities,
                                const void *values);
  virtual int set_relation_side(iBase_EntitySetHandle *sets, int num_sets,
                                const void *values);

  virtual int rmv_relation_side(iBase_EntityHandle *entities, int num_entities);
  virtual int rmv_relation_side(iBase_EntitySetHandle *sets, int num_sets);

  virtual int get_iterator(iBase_EntitySetHandle set,
                           iBase_EntityIterator *iter);

  virtual int get_gids(iBase_EntityHandle *entities, int num_entities,
                       int *values);
  virtual int get_gids(iBase_EntitySetHandle *sets, int num_sets, int *values);

  virtual int get_dims(iBase_EntityHandle *entities, int num_entities,
                       int *values);
  virtual int get_dims(iBase_EntitySetHandle *sets, int num_sets, int *values);
private:
  iRel_Instance relation;
  iGeom_Instance instance_;
  int id;

  iBase_TagHandle relation_tag;
  iBase_TagHandle gid_tag;
};

#endif
