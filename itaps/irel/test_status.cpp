#include "iGeom.h"
#include "iMesh.h"
#include "iRel.h"

#include "TestUtil.hpp"

#include <cstdlib>

iGeom_Instance geom;
iMesh_Instance mesh;
iRel_Instance rel;
using namespace moab;

void test_initial_inactive()
{
  int err;
  iRel_PairHandle pair;

  iRel_createPair(rel, geom, iRel_SET, iRel_IGEOM_IFACE, iRel_INACTIVE,
                       mesh, iRel_SET, iRel_IMESH_IFACE, iRel_ACTIVE,
                  &pair, &err);
  CHECK_ERR(err);

  iBase_EntitySetHandle geom_set;
  iGeom_createEntSet(geom, false, &geom_set, &err);
  CHECK_ERR(err);

  iBase_EntitySetHandle mesh_set;
  iMesh_createEntSet(mesh, false, &mesh_set, &err);
  CHECK_ERR(err);

  iRel_setSetSetRelation(rel, pair, geom_set, mesh_set, &err);
  CHECK_ERR(err);

  iBase_EntitySetHandle related_set;
  iRel_getSetSetRelation(rel, pair, mesh_set, 1, &related_set, &err);
  CHECK_ERR(err);
  CHECK_EQUAL(related_set, geom_set);

  iRel_getSetSetRelation(rel, pair, geom_set, 0, &related_set, &err);
  CHECK(err != iBase_SUCCESS);

  iRel_changePairStatus(rel, pair, iRel_ACTIVE, iRel_ACTIVE, &err);
  CHECK_ERR(err);

  iRel_getSetSetRelation(rel, pair, geom_set, 0, &related_set, &err);
  CHECK_ERR(err);
  CHECK_EQUAL(related_set, mesh_set);

}

void test_initial_notexist()
{
  int err;
  iRel_PairHandle pair;

  iRel_createPair(rel, geom, iRel_SET, iRel_IGEOM_IFACE, iRel_NOTEXIST,
                       mesh, iRel_SET, iRel_IMESH_IFACE, iRel_ACTIVE,
                  &pair, &err);
  CHECK_ERR(err);

  iBase_EntitySetHandle geom_set;
  iGeom_createEntSet(geom, false, &geom_set, &err);
  CHECK_ERR(err);

  iBase_EntitySetHandle mesh_set;
  iMesh_createEntSet(mesh, false, &mesh_set, &err);
  CHECK_ERR(err);

  iRel_setSetSetRelation(rel, pair, geom_set, mesh_set, &err);
  CHECK_ERR(err);

  iBase_EntitySetHandle related_set;
  iRel_getSetSetRelation(rel, pair, mesh_set, 1, &related_set, &err);
  CHECK_ERR(err);
  CHECK_EQUAL(related_set, geom_set);

  iRel_getSetSetRelation(rel, pair, geom_set, 0, &related_set, &err);
  CHECK(err != iBase_SUCCESS);

  iRel_changePairStatus(rel, pair, iRel_ACTIVE, iRel_ACTIVE, &err);
  CHECK_ERR(err);

  iRel_getSetSetRelation(rel, pair, geom_set, 0, &related_set, &err);
  CHECK_ERR(err);
  CHECK_EQUAL(related_set, mesh_set);

}

int main()
{
  int err;
  int num_fail = 0;

  iGeom_newGeom(0, &geom, &err, 0);
  iMesh_newMesh(0, &mesh, &err, 0);
  iRel_create(0, &rel, &err, 0);

  num_fail += RUN_TEST(test_initial_inactive);
  num_fail += RUN_TEST(test_initial_notexist);

  iRel_destroy(rel, &err);
  iMesh_dtor(mesh, &err);
  iGeom_dtor(geom, &err);

  return num_fail;
}
