#include "smtk/model/Storage.h"
#include "smtk/model/ExportJSON.h"

#include "cJSON.h"

#include <assert.h>

using namespace smtk::util;
using namespace smtk::model;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  UUIDsToEntities smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  Storage sm(&smTopology, &smArrangements, &smTessellation);

  UUID uc00 = sm.insertCellOfDimension(0)->first; // keep just the UUID around.
  UUID uc01 = sm.insertCellOfDimension(0)->first;
  UUID uc02 = sm.insertCellOfDimension(0)->first;
  UUID uc03 = sm.insertCellOfDimension(0)->first;
  UUID uc04 = sm.insertCellOfDimension(0)->first;
  UUID uc05 = sm.insertCellOfDimension(0)->first;
  UUID uc06 = sm.insertCellOfDimension(0)->first;

  UUID uc07 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc01))->first;
  UUID uc08 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc02))->first;
  UUID uc09 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc00))->first;
  UUID uc10 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc03).appendRelation(uc04))->first;
  UUID uc11 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc04).appendRelation(uc05))->first;
  UUID uc12 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc05).appendRelation(uc03))->first;
  UUID uc13 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc06))->first;
  UUID uc14 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc06))->first;
  UUID uc15 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc06))->first;

  UUID uc16 = sm.insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc07)
    .appendRelation(uc08)
    .appendRelation(uc09)
    .appendRelation(uc10)
    .appendRelation(uc11)
    .appendRelation(uc12)
    )->first;
  UUID uc17 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc10).appendRelation(uc11).appendRelation(uc12))->first;
  UUID uc18 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc07).appendRelation(uc13).appendRelation(uc14))->first;
  UUID uc19 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc08).appendRelation(uc14).appendRelation(uc15))->first;
  UUID uc20 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc09).appendRelation(uc15).appendRelation(uc13))->first;

  UUID uc21 = sm.insertEntity(
    Entity(CELL_ENTITY, 3)
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

  unsigned int uc00Flags = sm.findEntity(uc00)->entityFlags();
  assert( smtk::model::isVertex(uc00Flags)    && "isVertex(vertexFlags) incorrect");
  assert(!smtk::model::isEdge(uc00Flags)      && "isEdge(vertexFlags) incorrect");
  assert(!smtk::model::isFace(uc00Flags)      && "isFace(vertexFlags) incorrect");
  assert(!smtk::model::isRegion(uc00Flags)    && "isRegion(vertexFlags) incorrect");
  assert(!smtk::model::isChain(uc00Flags)     && "isChain(vertexFlags) incorrect");
  assert(!smtk::model::isLoop(uc00Flags)      && "isLoop(vertexFlags) incorrect");
  assert(!smtk::model::isShell(uc00Flags)     && "isShell(vertexFlags) incorrect");
  assert(!smtk::model::isVertexUse(uc00Flags) && "isVertexUse(vertexFlags) incorrect");
  assert(!smtk::model::isEdgeUse(uc00Flags)   && "isEdgeUse(vertexFlags) incorrect");
  assert(!smtk::model::isFaceUse(uc00Flags)   && "isFaceUse(vertexFlags) incorrect");

  UUIDs nodes = sm.entitiesOfDimension(0);
  UUIDs edges = sm.entitiesOfDimension(1);
  UUIDs faces = sm.entitiesOfDimension(2);
  UUIDs zones = sm.entitiesOfDimension(3);

  // Test the methods used to set/get string properties
  sm.setStringProperty(uc21, "name", "Tetrahedron");
  smtk::model::StringList components;
  components.push_back("vx");
  components.push_back("vy");
  components.push_back("vz");
  sm.setStringProperty(uc00, "velocity", components);
  sm.stringProperty(uc21, "name")[0] = "Ignatius";
  sm.stringProperty(uc21, "name").push_back("J");
  sm.stringProperty(uc21, "name").push_back("Fumblemumbler");
  sm.setStringProperty(uc21, "name", "Tetrahedron"); // Resets name to length 1.
  assert(sm.stringProperty(uc00, "velocity")[0] == "vx");
  assert(sm.stringProperty(uc00, "velocity")[1] == "vy");
  assert(sm.stringProperty(uc00, "velocity").size() == 3); // Test multi-entry length.
  assert(sm.stringProperty(uc21, "velocity").size() == 0); // Test missing entry length.
  assert(sm.stringProperty(uc21, "name").size() == 1); // Test length of reset property.

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
