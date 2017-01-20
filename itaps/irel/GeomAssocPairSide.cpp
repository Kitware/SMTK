#include "GeomAssocPairSide.hpp"

#include <cstring>
#include <sstream>
#include <string>

#include "Lasso.hpp"
#include "iRel_Lasso.hpp"

#define PROCESS_ERROR do {                              \
    if (iBase_SUCCESS != result) {                      \
      char this_descr[120];                             \
      iGeom_getDescription(instance_, this_descr, 120); \
      ERRORR(result, this_descr);                       \
    }                                                   \
  } while(false)

// Redefine LASSOI
#undef LASSOI
#define LASSOI lasso_instance(relation)

static const char *GLOBAL_ID_TAG_NAME = "GLOBAL_ID";
static const char *RELATION_TAG_NAME = "__GEOM_ASSOCIATION";

GeomAssocPairSide::GeomAssocPairSide(iRel_Instance p_relation,
                                     iBase_Instance p_instance, int p_id) :
  relation(p_relation),
  instance_(reinterpret_cast<iGeom_Instance>(p_instance)),
  id(p_id)
{
  int result;

  create_relation_side();

  iGeom_getTagHandle(instance_, GLOBAL_ID_TAG_NAME, &gid_tag, &result,
                     strlen(GLOBAL_ID_TAG_NAME));
  if (result == iBase_TAG_NOT_FOUND) {
    iGeom_createTag(instance_, GLOBAL_ID_TAG_NAME, 1, iBase_INTEGER,
                    &gid_tag, &result, strlen(GLOBAL_ID_TAG_NAME));
  }
}

GeomAssocPairSide::~GeomAssocPairSide()
{
  destroy_relation_side();
}

iBase_Instance GeomAssocPairSide::instance() const
{
  return instance_;
}

iRel_IfaceType GeomAssocPairSide::type() const
{
  return iRel_IGEOM_IFACE;
}

int GeomAssocPairSide::create_relation_side()
{
  int result;
  std::stringstream ss;
  ss << RELATION_TAG_NAME << id;
  std::string rel_tag_name(ss.str());

  iGeom_getTagHandle(instance_, rel_tag_name.c_str(), &relation_tag, &result,
                     rel_tag_name.size());
  if (result == iBase_TAG_NOT_FOUND) {
    iGeom_createTag(instance_, rel_tag_name.c_str(), 1, iBase_ENTITY_HANDLE,
                    &relation_tag, &result, rel_tag_name.size());
  }

  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::destroy_relation_side()
{
  if (relation_tag) {
    int result;

    iGeom_destroyTag(instance_, relation_tag, true, &result);
    relation_tag = NULL;

    PROCESS_ERROR;
  }
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_all_entities(int dimension,
                                        iBase_EntityHandle **entities,
                                        int *entities_alloc,
                                        int *entities_size)
{
  int this_type = (dimension == -1 ? iBase_ALL_TYPES : dimension);
  int result;

  iGeom_getEntities(instance_, 0, this_type, entities, entities_alloc,
                    entities_size, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_all_sets(iBase_EntitySetHandle **sets,
                                    int *sets_alloc, int *sets_size)
{
  int result;

  iGeom_getEntSets(instance_, 0, 0, sets, sets_alloc, sets_size, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_entities(int dimension,
                                    iBase_EntitySetHandle set_handle,
                                    iBase_EntityHandle **entities,
                                    int *entities_alloc,
                                    int *entities_size)
{
  int this_type = (dimension == -1 ? iBase_ALL_TYPES : dimension);
  int result;

  iGeom_getEntities(instance_, set_handle, this_type, entities, entities_alloc,
                    entities_size, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_ents_dims(iBase_EntityHandle *entities,
                                     int entities_size,
                                     int **ent_types,
                                     int *ent_types_alloc,
                                     int *ent_types_size)
{
  int result;

  iGeom_getArrType(instance_, entities, entities_size, ent_types,
                   ent_types_alloc, ent_types_size, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_related_ents(iBase_EntityHandle ** /*entities*/,
                                        int * /*entities_alloc*/, int * /*entities_size*/)
{
  RETURNR(iBase_FAILURE);
}

int GeomAssocPairSide::get_related_sets(iBase_EntitySetHandle ** /*sets*/,
                                        int * /*sets_alloc*/, int * /*sets_size*/)
{
  RETURNR(iBase_FAILURE);
}

int GeomAssocPairSide::get_relation_side(iBase_EntityHandle *entities,
                                         int num_entities, void *values)
{
  int values_alloc = num_entities * sizeof(iBase_EntityHandle);
  int values_size;
  int result;

  iGeom_getArrData(instance_, entities, num_entities, relation_tag,
                   &values, &values_alloc, &values_size, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_relation_side(iBase_EntitySetHandle *sets, 
                                         int num_sets, void *values)
{
  char *data = static_cast<char*>(values);
  int values_alloc = sizeof(iBase_EntityHandle);
  int values_size;
  int result;

  for (int i = 0; i < num_sets; i++) {
    iGeom_getEntSetData(instance_, sets[i], relation_tag,
                        reinterpret_cast<void**>(&data),
                        &values_alloc, &values_size, &result);
    data += values_size;
    PROCESS_ERROR;
  }
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::set_relation_side(iBase_EntityHandle *entities,
                                         int num_entities, const void *values)
{
  int result;

  iGeom_setArrData(instance_, entities, num_entities, relation_tag,
                   static_cast<const char*>(values),
                   num_entities*sizeof(iBase_EntityHandle), &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::set_relation_side(iBase_EntitySetHandle *sets,
                                         int num_sets, const void *values)
{
  const char *data = static_cast<const char*>(values);
  int size = sizeof(iBase_EntityHandle);
  int result;

  for (int i = 0; i < num_sets; i++) {
    iGeom_setEntSetData(instance_, sets[i], relation_tag, data, size, &result);
    data += size;
    PROCESS_ERROR;
  }
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::rmv_relation_side(iBase_EntityHandle *entities,
                                         int num_entities)
{
  int result;

  iGeom_rmvArrTag(instance_, entities, num_entities, relation_tag, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::rmv_relation_side(iBase_EntitySetHandle *sets,
                                         int num_sets)
{
  int result;

  for (int i = 0; i < num_sets; i++) {
    iGeom_rmvEntSetTag(instance_, sets[i], relation_tag, &result);
    PROCESS_ERROR;
  }

  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_iterator(iBase_EntitySetHandle set,
                                    iBase_EntityIterator *iter)
{
  int result;
  iGeom_initEntIter(instance_, set, iBase_ALL_TYPES, iter, &result);
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_gids(iBase_EntityHandle *entities, int num_entities,
                                int *values)
{
  int values_alloc = num_entities * sizeof(int);
  int values_size;
  int result;

  iGeom_getArrData(instance_, entities, num_entities, gid_tag,
                   reinterpret_cast<void**>(&values), &values_alloc,
                   &values_size, &result);
  PROCESS_ERROR;
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_gids(iBase_EntitySetHandle *sets, int num_sets,
                                int *values)
{
  char *data = reinterpret_cast<char*>(values);
  int values_alloc = sizeof(int);
  int values_size;
  int result;

  for (int i = 0; i < num_sets; i++) {
    iGeom_getEntSetData(instance_, sets[i], gid_tag,
                        reinterpret_cast<void**>(&data),
                        &values_alloc, &values_size, &result);
    data += values_size;
    PROCESS_ERROR;
  }
  RETURNR(iBase_SUCCESS);
}

int GeomAssocPairSide::get_dims(iBase_EntityHandle * /*entities*/, int /*num_entities*/,
                                int * /*values*/)
{
  RETURNR(iBase_FAILURE);
}

int GeomAssocPairSide::get_dims(iBase_EntitySetHandle * /*sets*/, int /*num_sets*/,
                                int * /*values*/)
{
  RETURNR(iBase_FAILURE);
}
