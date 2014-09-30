//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/Volume.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "cJSON.h"

using smtk::shared_ptr;
using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;
using namespace smtk::io;

static int entCount = 0;
static int subgroups = 0;
static int subcells = 0;
static int submodels = 0;

int entityManagerEvent(ManagerEventType evt, const smtk::model::Cursor&, void*)
{
  if (evt.first == ADD_EVENT)
    ++entCount;
  else if (evt.first == DEL_EVENT)
    --entCount;
  return 0;
}

int addEntityToModel(ManagerEventType evt, const smtk::model::Cursor& src, const smtk::model::Cursor& related, void*)
{
  if (evt.first == ADD_EVENT)
    {
    if (src.isModelEntity())
      {
      if (related.isGroupEntity())
        ++subgroups;
      else if (related.isCellEntity())
        ++subcells;
      else if (related.isModelEntity())
        ++submodels;
      }
    }
  return 0;
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  ManagerPtr sm = Manager::create();
  sm->observe(std::make_pair(ANY_EVENT,ENTITY_ENTRY), &entityManagerEvent, NULL);
  sm->observe(std::make_pair(ANY_EVENT,MODEL_INCLUDES_FREE_CELL), &addEntityToModel, NULL);
  sm->observe(std::make_pair(ANY_EVENT,MODEL_INCLUDES_GROUP), &addEntityToModel, NULL);
  sm->observe(std::make_pair(ANY_EVENT,MODEL_INCLUDES_MODEL), &addEntityToModel, NULL);

  UUIDArray uids = createTet(sm);

  BitFlags uc00Flags = sm->findEntity(uids[0])->entityFlags();
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
  sm->setStringProperty(uids[0], "velocity", components);
  sm->stringProperty(uids[21], "name")[0] = "Ignatius";
  sm->stringProperty(uids[21], "name").push_back("J");
  sm->stringProperty(uids[21], "name").push_back("Fumblemumbler");
  sm->setStringProperty(uids[21], "name", "Tetrahedron"); // Resets name to length 1.
  test(sm->stringProperty(uids[0], "velocity")[0] == "vx");
  test(sm->stringProperty(uids[0], "velocity")[1] == "vy");
  test(sm->stringProperty(uids[0], "velocity").size() == 3); // Test multi-entry length.
  test(sm->stringProperty(uids[21], "velocity").size() == 0); // Test missing entry length.
  test(sm->stringProperty(uids[21], "name").size() == 1); // Test length of reset property.
  double d3vals[] = { 1.0, 0.0, 2.0 };
  int i2vals[] = { 100, 200 };
  FloatList v3(d3vals, d3vals + 3);
  IntegerList v2(i2vals, i2vals + 2);
  sm->setFloatProperty(uids[0], "velocity", v3);
  sm->setIntegerProperty(uids[0], "velocity", v2);
  sm->setFloatProperty(uids[21], "velocity", 42.03125);
  sm->setIntegerProperty(uids[21], "velocity", 42);

  // Test finding entities by property
  Volumes search1 = sm->findEntitiesByPropertyAs<Volumes>("name", "Tetrahedron");
  test(!search1.empty() && search1.begin()->name() == "Tetrahedron");
  StringList ijfumbler;
  ijfumbler.push_back("Ignatius");
  ijfumbler.push_back("Jeremiah");
  ijfumbler.push_back("Fumblemumbler");
  search1.begin()->setStringProperty("name", ijfumbler);
  search1 = sm->findEntitiesByPropertyAs<Volumes>("name", ijfumbler);
  test(!search1.empty() && search1.begin()->stringProperty("name") == ijfumbler);
  search1.begin()->setStringProperty("name", "Tetrahedron");

  CursorArray search2;
  search2 = sm->findEntitiesByProperty("velocity", v2);
  test(search2.size() == 1 && search2.begin()->entity() == uids[0], "search2 i2vals");
  search2 = sm->findEntitiesByProperty("velocity", v3);
  test(search2.size() == 1 && search2.begin()->entity() == uids[0], "search2 i2vals");
  search2 = sm->findEntitiesByProperty("velocity", static_cast<Integer>(42));
  test(search2.size() == 1 && search2.begin()->entity() == uids[21], "search2 42");
  search2 = sm->findEntitiesByProperty("velocity", 42.03125);
  test(search2.size() == 1 && search2.begin()->entity() == uids[21], "search2 42.03125");

  // Test Cursors-return version of entitiesMatchingFlagsAs<T>
  search2 = sm->findEntitiesOfType(smtk::model::VOLUME, true);
  test(search2.size() == 1 && search2.begin()->entity() == uids[21]);

  // Test addModel
  UUIDArray::size_type modelStart = uids.size();
  for (int i = 0; i < 53; ++i)
    {
    uids.push_back(sm->addModel().entity());
    test(sm->hasIntegerProperty(uids.back(), "cell_counters"));
    test(sm->hasStringProperty(uids.back(), "name"));
    }
  sm->findEntity(uids[21])->relations().push_back(uids[modelStart]);
  // Correct computation of hexavigesimal name strings:
  test(sm->stringProperty(uids[modelStart + 0], "name")[0] == "Model A");
  test(sm->stringProperty(uids[modelStart + 26], "name")[0] == "Model AA");
  test(sm->stringProperty(uids[modelStart + 52], "name")[0] == "Model BA");
  ModelEntity model(sm, uids[modelStart]);
  for (int i = 0; i < 22; ++i)
    model.addCell(Cursor(sm, uids[i]));
  model.addSubmodel(ModelEntity(sm, uids[modelStart + 26]));

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
  cJSON_AddItemToObject(root, "upper(uc00,3)", ExportJSON::fromUUIDs(sm->higherDimensionalBordants(uids[0],3)));
  char* json = cJSON_Print(root);
  std::cout << json << "\n";
  free(json);
  cJSON_Delete(root);

  // Test attribute assignment (model-side only; no attributes are
  // created, but we can make up attribute IDs and assign them to
  // entities). See also: attributeAssociationTest.
  test( sm->attachAttribute(/*attribId*/0, uids[0]), "Inserting a new attribute should succeed");
  test( sm->attachAttribute(/*attribId*/1, uids[0]), "Inserting a new attribute should succeed");
  test( sm->attachAttribute(/*attribId*/0, uids[0]), "Inserting an attribute twice should succeed");
  test( sm->detachAttribute(/*attribId*/0, uids[0]), "Removing a non-existent attribute should fail");
  test(!sm->detachAttribute(/*attribId*/0, uids[1]), "Removing a non-existent attribute should fail");

  // Test removal of arrangement information and entities.
  // Remove a volume from its volume use and the model containing it.
  test(sm->unarrangeEntity(uids[21], EMBEDDED_IN, 0, true) == 1, "Detaching a Volume from its parent Model did not succeed.");
  test(sm->unarrangeEntity(uids[21], EMBEDDED_IN, 0, true) == 0, "Detaching a Volume from a non-existent Model did not fail.");
  test(sm->unarrangeEntity(uids[21], HAS_USE, 0, true) == 2, "Detaching a Volume/VolumeUse failed.");
  test(sm->findEntity(uids[21]) == NULL, "unarrangeEntity(..., true) failed to remove the entity afterwards.");
  test(sm->erase(uids[0]), "Failed to erase a vertex.");

  std::cout << entCount << " total entities:\n";
  std::cout << "subgroups " << subgroups << "\n";
  std::cout << "submodels " << submodels << "\n";
  std::cout << "subcells " << subcells << "\n";

  return 0;
}
