#include "smtk/model/ModelBody.h"
#include "smtk/model/ExportJSON.h"

#include "cJSON.h"

using namespace smtk::model;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  UUIDsToLinks smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  ModelBody sm(&smTopology, &smArrangements, &smTessellation);

  UUID uc00 = sm.InsertCellOfDimension(0)->first; // keep just the UUID around.
  UUID uc01 = sm.InsertCellOfDimension(0)->first;
  UUID uc02 = sm.InsertCellOfDimension(0)->first;
  UUID uc03 = sm.InsertCellOfDimension(0)->first;
  UUID uc04 = sm.InsertCellOfDimension(0)->first;
  UUID uc05 = sm.InsertCellOfDimension(0)->first;
  UUID uc06 = sm.InsertCellOfDimension(0)->first;

  UUID uc07 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc01))->first;
  UUID uc08 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc02))->first;
  UUID uc09 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc00))->first;
  UUID uc10 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc03).appendRelation(uc04))->first;
  UUID uc11 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc04).appendRelation(uc05))->first;
  UUID uc12 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc05).appendRelation(uc03))->first;
  UUID uc13 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc06))->first;
  UUID uc14 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc06))->first;
  UUID uc15 = sm.InsertLink(Link(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc06))->first;

  UUID uc16 = sm.InsertLink(
    Link(CELL_ENTITY, 2)
    .appendRelation(uc07)
    .appendRelation(uc08)
    .appendRelation(uc09)
    .appendRelation(uc10)
    .appendRelation(uc11)
    .appendRelation(uc12)
    )->first;
  UUID uc17 = sm.InsertLink(Link(CELL_ENTITY, 2).appendRelation(uc10).appendRelation(uc11).appendRelation(uc12))->first;
  UUID uc18 = sm.InsertLink(Link(CELL_ENTITY, 2).appendRelation(uc07).appendRelation(uc13).appendRelation(uc14))->first;
  UUID uc19 = sm.InsertLink(Link(CELL_ENTITY, 2).appendRelation(uc08).appendRelation(uc14).appendRelation(uc15))->first;
  UUID uc20 = sm.InsertLink(Link(CELL_ENTITY, 2).appendRelation(uc09).appendRelation(uc15).appendRelation(uc13))->first;

  UUID uc21 = sm.InsertLink(
    Link(CELL_ENTITY, 3)
    .appendRelation(uc16)
    .appendRelation(uc17)
    .appendRelation(uc18)
    .appendRelation(uc19)
    .appendRelation(uc20))->first;

  sm.SetTessellation(uc21, Tessellation()
    .AddCoords(0., 0., 0.)
    .AddCoords(4., 0., 0.)
    .AddCoords(2., 4., 0.)
    .AddCoords(1., 1., 0.)
    .AddCoords(2., 3., 0.)
    .AddCoords(3., 1., 0.)
    .AddCoords(2., 0.,-4.)
    .AddTriangle(0, 3, 5)
    .AddTriangle(0, 5, 1)
    .AddTriangle(1, 5, 4)
    .AddTriangle(1, 4, 2)
    .AddTriangle(2, 4, 3)
    .AddTriangle(2, 3, 0)
    .AddTriangle(3, 5, 4)
    .AddTriangle(0, 6, 1)
    .AddTriangle(1, 6, 2)
    .AddTriangle(2, 6, 0));

  UUIDs nodes = sm.Entities(0);
  UUIDs edges = sm.Entities(1);
  UUIDs faces = sm.Entities(2);
  UUIDs zones = sm.Entities(3);

  cJSON* root = cJSON_CreateObject();
  ExportJSON::FromModel(root, &sm);
  cJSON_AddItemToObject(root, "nodes", ExportJSON::FromUUIDs(nodes));
  cJSON_AddItemToObject(root, "edges", ExportJSON::FromUUIDs(edges));
  cJSON_AddItemToObject(root, "faces", ExportJSON::FromUUIDs(faces));
  cJSON_AddItemToObject(root, "zones", ExportJSON::FromUUIDs(zones));
  cJSON_AddItemToObject(root, "bdy(brd(uc13,2),1)", ExportJSON::FromUUIDs(sm.BoundaryEntities(sm.BordantEntities(uc13,2),1)));
  cJSON_AddItemToObject(root, "bdy(uc20,2)", ExportJSON::FromUUIDs(sm.BoundaryEntities(uc20,2)));
  cJSON_AddItemToObject(root, "brd(uc20,2)", ExportJSON::FromUUIDs(sm.BordantEntities(uc20,2)));
  cJSON_AddItemToObject(root, "bdy(uc20,1)", ExportJSON::FromUUIDs(sm.BoundaryEntities(uc20,1)));
  cJSON_AddItemToObject(root, "brd(uc20,3)", ExportJSON::FromUUIDs(sm.BordantEntities(uc20,3)));
  cJSON_AddItemToObject(root, "lower(uc21,1)", ExportJSON::FromUUIDs(sm.LowerDimensionalBoundaries(uc21,1)));
  cJSON_AddItemToObject(root, "upper(uc00,3)", ExportJSON::FromUUIDs(sm.HigherDimensionalBordants(uc00,3)));
  std::cout << cJSON_Print(root) << "\n";
  cJSON_Delete(root);

  return 0;
}
