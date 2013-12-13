#include "smtk/model/Storage.h"
#include "smtk/model/ExportJSON.h"
#include "smtk/model/testing/helpers.h"
#include <smtk/util/Testing/helpers.h>

#include "cJSON.h"

using smtk::shared_ptr;
using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  StoragePtr sm = Storage::New();

  UUIDArray uids = createTet(sm);

  BitFlags uc00Flags = sm->findEntity(uids[00])->entityFlags();
  test( smtk::model::isVertex(uc00Flags),          "isVertex(vertexFlags) incorrect");
  test(!smtk::model::isEdge(uc00Flags),            "isEdge(vertexFlags) incorrect");
  test(!smtk::model::isFace(uc00Flags),            "isFace(vertexFlags) incorrect");
  test(!smtk::model::isVolume(uc00Flags),          "isVolume(vertexFlags) incorrect");
  test(!smtk::model::isChain(uc00Flags),           "isChain(vertexFlags) incorrect");
  test(!smtk::model::isLoop(uc00Flags),            "isLoop(vertexFlags) incorrect");
  test(!smtk::model::isShell(uc00Flags),           "isShell(vertexFlags) incorrect");
  test(!smtk::model::isVertexUse(uc00Flags),       "isVertexUse(vertexFlags) incorrect");
  test(!smtk::model::isEdgeUse(uc00Flags),         "isEdgeUse(vertexFlags) incorrect");
  test(!smtk::model::isFaceUse(uc00Flags),         "isFaceUse(vertexFlags) incorrect");

  test( smtk::model::isCellEntity(uc00Flags),      "isCellEntity(vertexFlags) incorrect");
  test(!smtk::model::isUseEntity(uc00Flags),       "isUseEntity(vertexFlags) incorrect");
  test(!smtk::model::isShellEntity(uc00Flags),     "isShellEntity(vertexFlags) incorrect");
  test(!smtk::model::isGroupEntity(uc00Flags),     "isGroupEntity(vertexFlags) incorrect");
  test(!smtk::model::isModelEntity(uc00Flags),     "isModelEntity(vertexFlags) incorrect");
  test(!smtk::model::isInstanceEntity(uc00Flags),  "isInstanceEntity(vertexFlags) incorrect");

  UUIDs nodes = sm->entitiesOfDimension(0);
  UUIDs edges = sm->entitiesOfDimension(1);
  UUIDs faces = sm->entitiesOfDimension(2);
  UUIDs zones = sm->entitiesOfDimension(3);

  // Test the methods used to set/get string properties
  sm->setStringProperty(uids[21], "name", "Tetrahedron");
  smtk::model::StringList components;
  components.push_back("vx");
  components.push_back("vy");
  components.push_back("vz");
  sm->setStringProperty(uids[00], "velocity", components);
  sm->stringProperty(uids[21], "name")[0] = "Ignatius";
  sm->stringProperty(uids[21], "name").push_back("J");
  sm->stringProperty(uids[21], "name").push_back("Fumblemumbler");
  sm->setStringProperty(uids[21], "name", "Tetrahedron"); // Resets name to length 1.
  test(sm->stringProperty(uids[00], "velocity")[0] == "vx");
  test(sm->stringProperty(uids[00], "velocity")[1] == "vy");
  test(sm->stringProperty(uids[00], "velocity").size() == 3); // Test multi-entry length.
  test(sm->stringProperty(uids[21], "velocity").size() == 0); // Test missing entry length.
  test(sm->stringProperty(uids[21], "name").size() == 1); // Test length of reset property.

  // Test addModel
  for (int i = 0; i < 53; ++i)
    {
    uids.push_back(sm->addModel());
    test(sm->hasIntegerProperty(uids.back(), "cell_counters"));
    test(sm->hasStringProperty(uids.back(), "name"));
    }
  sm->findEntity(uids[21])->relations().push_back(uids[22]);
  // Correct computation of hexavigesimal name strings:
  test(sm->stringProperty(uids[22], "name")[0] == "Model A");
  test(sm->stringProperty(uids[48], "name")[0] == "Model AA");
  test(sm->stringProperty(uids[74], "name")[0] == "Model BA");

  sm->assignDefaultNames();
  // Verify we don't overwrite existing names
  test(sm->stringProperty(uids[21], "name")[0] == "Tetrahedron");
  // Verify we do give everything a name
  test(sm->hasStringProperty(uids[11], "name"));

  cJSON* root = cJSON_CreateObject();
  ExportJSON::fromModel(root, sm);
  cJSON_AddItemToObject(root, "nodes", ExportJSON::fromUUIDs(nodes));
  cJSON_AddItemToObject(root, "edges", ExportJSON::fromUUIDs(edges));
  cJSON_AddItemToObject(root, "faces", ExportJSON::fromUUIDs(faces));
  cJSON_AddItemToObject(root, "zones", ExportJSON::fromUUIDs(zones));
  cJSON_AddItemToObject(root, "bdy(brd(uc13,2),1)", ExportJSON::fromUUIDs(sm->boundaryEntities(sm->bordantEntities(uids[13],2),1)));
  cJSON_AddItemToObject(root, "bdy(uc20,2)", ExportJSON::fromUUIDs(sm->boundaryEntities(uids[20],2)));
  cJSON_AddItemToObject(root, "brd(uc20,2)", ExportJSON::fromUUIDs(sm->bordantEntities(uids[20],2)));
  cJSON_AddItemToObject(root, "bdy(uc20,1)", ExportJSON::fromUUIDs(sm->boundaryEntities(uids[20],1)));
  cJSON_AddItemToObject(root, "brd(uc20,3)", ExportJSON::fromUUIDs(sm->bordantEntities(uids[20],3)));
  cJSON_AddItemToObject(root, "lower(uc21,1)", ExportJSON::fromUUIDs(sm->lowerDimensionalBoundaries(uids[21],1)));
  cJSON_AddItemToObject(root, "upper(uc00,3)", ExportJSON::fromUUIDs(sm->higherDimensionalBordants(uids[00],3)));
  char* json = cJSON_Print(root);
  std::cout << json << "\n";
  free(json);
  cJSON_Delete(root);

  return 0;
}
