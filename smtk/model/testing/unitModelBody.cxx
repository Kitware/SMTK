#include "smtk/model/ModelBody.h"
#include "smtk/model/ExportJSON.h"

#include "cJSON.h"

using namespace smtk::util;
using namespace smtk::model;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  UUIDsToLinks smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  ModelBody sm(&smTopology, &smArrangements, &smTessellation);

  UUID uc00 = sm.insertCellOfDimension(0)->first; // keep just the UUID around.
  UUID uc01 = sm.insertCellOfDimension(0)->first;
  UUID uc02 = sm.insertCellOfDimension(0)->first;
  UUID uc03 = sm.insertCellOfDimension(0)->first;
  UUID uc04 = sm.insertCellOfDimension(0)->first;
  UUID uc05 = sm.insertCellOfDimension(0)->first;
  UUID uc06 = sm.insertCellOfDimension(0)->first;

  UUID uc07 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc01))->first;
  UUID uc08 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc02))->first;
  UUID uc09 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc00))->first;
  UUID uc10 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc03).appendRelation(uc04))->first;
  UUID uc11 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc04).appendRelation(uc05))->first;
  UUID uc12 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc05).appendRelation(uc03))->first;
  UUID uc13 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc06))->first;
  UUID uc14 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc06))->first;
  UUID uc15 = sm.insertLink(Link(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc06))->first;

  UUID uc16 = sm.insertLink(
    Link(CELL_ENTITY, 2)
    .appendRelation(uc07)
    .appendRelation(uc08)
    .appendRelation(uc09)
    .appendRelation(uc10)
    .appendRelation(uc11)
    .appendRelation(uc12)
    )->first;
  UUID uc17 = sm.insertLink(Link(CELL_ENTITY, 2).appendRelation(uc10).appendRelation(uc11).appendRelation(uc12))->first;
  UUID uc18 = sm.insertLink(Link(CELL_ENTITY, 2).appendRelation(uc07).appendRelation(uc13).appendRelation(uc14))->first;
  UUID uc19 = sm.insertLink(Link(CELL_ENTITY, 2).appendRelation(uc08).appendRelation(uc14).appendRelation(uc15))->first;
  UUID uc20 = sm.insertLink(Link(CELL_ENTITY, 2).appendRelation(uc09).appendRelation(uc15).appendRelation(uc13))->first;

  UUID uc21 = sm.insertLink(
    Link(CELL_ENTITY, 3)
    .appendRelation(uc16)
    .appendRelation(uc17)
    .appendRelation(uc18)
    .appendRelation(uc19)
    .appendRelation(uc20))->first;

  sm.setTessellation(uc21, Tessellation()
    .addCoords(0., 0., 0.)
    .addCoords(4., 0., 0.)
    .addCoords(2., 4., 0.)
    .addCoords(1., 1., 0.)
    .addCoords(2., 3., 0.)
    .addCoords(3., 1., 0.)
    .addCoords(2., 0.,-4.)
    .addTriangle(0, 3, 5)
    .addTriangle(0, 5, 1)
    .addTriangle(1, 5, 4)
    .addTriangle(1, 4, 2)
    .addTriangle(2, 4, 3)
    .addTriangle(2, 3, 0)
    .addTriangle(3, 5, 4)
    .addTriangle(0, 6, 1)
    .addTriangle(1, 6, 2)
    .addTriangle(2, 6, 0));

  UUIDs nodes = sm.entities(0);
  UUIDs edges = sm.entities(1);
  UUIDs faces = sm.entities(2);
  UUIDs zones = sm.entities(3);

  cJSON* root = cJSON_CreateObject();
  ExportJSON::fromModel(root, &sm);
  cJSON_AddItemToObject(root, "nodes", ExportJSON::fromUUIDs(nodes));
  cJSON_AddItemToObject(root, "edges", ExportJSON::fromUUIDs(edges));
  cJSON_AddItemToObject(root, "faces", ExportJSON::fromUUIDs(faces));
  cJSON_AddItemToObject(root, "zones", ExportJSON::fromUUIDs(zones));
  cJSON_AddItemToObject(root, "bdy(brd(uc13,2),1)", ExportJSON::fromUUIDs(sm.boundaryEntities(sm.bordantEntities(uc13,2),1)));
  cJSON_AddItemToObject(root, "bdy(uc20,2)", ExportJSON::fromUUIDs(sm.boundaryEntities(uc20,2)));
  cJSON_AddItemToObject(root, "brd(uc20,2)", ExportJSON::fromUUIDs(sm.bordantEntities(uc20,2)));
  cJSON_AddItemToObject(root, "bdy(uc20,1)", ExportJSON::fromUUIDs(sm.boundaryEntities(uc20,1)));
  cJSON_AddItemToObject(root, "brd(uc20,3)", ExportJSON::fromUUIDs(sm.bordantEntities(uc20,3)));
  cJSON_AddItemToObject(root, "lower(uc21,1)", ExportJSON::fromUUIDs(sm.lowerDimensionalBoundaries(uc21,1)));
  cJSON_AddItemToObject(root, "upper(uc00,3)", ExportJSON::fromUUIDs(sm.higherDimensionalBordants(uc00,3)));
  std::cout << cJSON_Print(root) << "\n";
  cJSON_Delete(root);

  return 0;
}
