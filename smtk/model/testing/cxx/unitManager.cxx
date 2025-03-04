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
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;
using namespace smtk::io;

static int entCount = 0;
static int subgroups = 0;
static int subcells = 0;
static int submodels = 0;

int entityResourceEvent(
  ResourceEventType evt,
  const smtk::model::EntityRef& /*unused*/,
  void* /*unused*/)
{
  if (evt.first == ADD_EVENT)
    ++entCount;
  else if (evt.first == DEL_EVENT)
    --entCount;
  return 0;
}

int addEntityToModel(
  ResourceEventType evt,
  const smtk::model::EntityRef& src,
  const smtk::model::EntityRef& related,
  void* /*unused*/)
{
  if (evt.first == ADD_EVENT)
  {
    if (src.isModel())
    {
      if (related.isGroup())
        ++subgroups;
      else if (related.isCellEntity())
        ++subcells;
      else if (related.isModel())
        ++submodels;
    }
  }
  return 0;
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  ResourcePtr sm = Resource::create();
  sm->observe(std::make_pair(ANY_EVENT, ENTITY_ENTRY), &entityResourceEvent, nullptr);
  sm->observe(std::make_pair(ANY_EVENT, MODEL_INCLUDES_FREE_CELL), &addEntityToModel, nullptr);
  sm->observe(std::make_pair(ANY_EVENT, MODEL_INCLUDES_GROUP), &addEntityToModel, nullptr);
  sm->observe(std::make_pair(ANY_EVENT, MODEL_INCLUDES_MODEL), &addEntityToModel, nullptr);

  UUIDArray uids = createTet(sm);

  BitFlags uc00Flags = sm->findEntity(uids[0])->entityFlags();
  test(smtk::model::isVertex(uc00Flags), "isVertex(vertexFlags) incorrect");
  test(!smtk::model::isEdge(uc00Flags), "isEdge(vertexFlags) incorrect");
  test(!smtk::model::isFace(uc00Flags), "isFace(vertexFlags) incorrect");
  test(!smtk::model::isVolume(uc00Flags), "isVolume(vertexFlags) incorrect");
  test(!smtk::model::isChain(uc00Flags), "isChain(vertexFlags) incorrect");
  test(!smtk::model::isLoop(uc00Flags), "isLoop(vertexFlags) incorrect");
  test(!smtk::model::isShell(uc00Flags), "isShell(vertexFlags) incorrect");
  test(!smtk::model::isVertexUse(uc00Flags), "isVertexUse(vertexFlags) incorrect");
  test(!smtk::model::isEdgeUse(uc00Flags), "isEdgeUse(vertexFlags) incorrect");
  test(!smtk::model::isFaceUse(uc00Flags), "isFaceUse(vertexFlags) incorrect");

  test(smtk::model::isCellEntity(uc00Flags), "isCellEntity(vertexFlags) incorrect");
  test(!smtk::model::isUseEntity(uc00Flags), "isUseEntity(vertexFlags) incorrect");
  test(!smtk::model::isShellEntity(uc00Flags), "isShellEntity(vertexFlags) incorrect");
  test(!smtk::model::isGroup(uc00Flags), "isGroup(vertexFlags) incorrect");
  test(!smtk::model::isModel(uc00Flags), "isModel(vertexFlags) incorrect");
  test(!smtk::model::isInstance(uc00Flags), "isInstance(vertexFlags) incorrect");

  UUIDs nodes = sm->entitiesOfDimension(0);
  UUIDs edges = sm->entitiesOfDimension(1);
  UUIDs faces = sm->entitiesOfDimension(2);
  UUIDs zones = sm->entitiesOfDimension(3);

  // Test the methods used to set/get string properties
  sm->setStringProperty(uids[21], "name", "Tetrahedron");
  smtk::model::StringList components;
  components.emplace_back("vx");
  components.emplace_back("vy");
  components.emplace_back("vz");
  sm->setStringProperty(uids[0], "velocity", components);
  sm->stringProperty(uids[21], "name")[0] = "Ignatius";
  sm->stringProperty(uids[21], "name").emplace_back("J");
  sm->stringProperty(uids[21], "name").emplace_back("Fumblemumbler");
  sm->setStringProperty(uids[21], "name", "Tetrahedron"); // Resets name to length 1.
  test(sm->stringProperty(uids[0], "velocity")[0] == "vx");
  test(sm->stringProperty(uids[0], "velocity")[1] == "vy");
  test(sm->stringProperty(uids[0], "velocity").size() == 3); // Test multi-entry length.
  test(sm->stringProperty(uids[21], "velocity").empty());    // Test missing entry length.
  test(sm->stringProperty(uids[21], "name").size() == 1);    // Test length of reset property.
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
  ijfumbler.emplace_back("Ignatius");
  ijfumbler.emplace_back("Jeremiah");
  ijfumbler.emplace_back("Fumblemumbler");
  search1.begin()->setStringProperty("name", ijfumbler);
  search1 = sm->findEntitiesByPropertyAs<Volumes>("name", ijfumbler);
  test(!search1.empty() && search1.begin()->stringProperty("name") == ijfumbler);
  search1.begin()->setStringProperty("name", "Tetrahedron");

  EntityRefArray search2;
  search2 = sm->findEntitiesByProperty("velocity", v2);
  test(search2.size() == 1 && search2.begin()->entity() == uids[0], "search2 i2vals");
  search2 = sm->findEntitiesByProperty("velocity", v3);
  test(search2.size() == 1 && search2.begin()->entity() == uids[0], "search2 i2vals");
  search2 = sm->findEntitiesByProperty("velocity", static_cast<Integer>(42));
  test(search2.size() == 1 && search2.begin()->entity() == uids[21], "search2 42");
  search2 = sm->findEntitiesByProperty("velocity", 42.03125);
  test(search2.size() == 1 && search2.begin()->entity() == uids[21], "search2 42.03125");

  // Test EntityRefs-return version of entitiesMatchingFlagsAs<T>
  search2 = sm->findEntitiesOfType(smtk::model::VOLUME, true);
  test(search2.size() == 1 && search2.begin()->entity() == uids[21]);

  // Test session creation (and create a session to own the model)
  Session::Ptr session = Session::create();
  sm->registerSession(session);
  SessionRef sref(sm, session);

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
  Model model(sm, uids[modelStart]);
  for (int i = 0; i < 22; ++i)
    model.addCell(EntityRef(sm, uids[i]));
  model.addSubmodel(Model(sm, uids[modelStart + 26]));
  model.setSession(sref);

  test(sm->sessionOwningEntity(uids[0]) == sref.entity());
  test(sm->sessionOwningEntity(model.entity()) == sref.entity());

  sm->assignDefaultNames();
  // Verify we don't overwrite existing names
  test(sm->stringProperty(uids[21], "name")[0] == "Tetrahedron");
  // Verify we do give everything a name
  test(sm->hasStringProperty(uids[11], "name"));

  // Test attribute assignment (model-side only; no attributes are
  // created, but we can make up attribute IDs and assign them to
  // entities). See also: attributeAssociationTest.
  smtk::common::UUID aid1, aid2;
  aid1 = smtk::common::UUID::random();
  aid2 = smtk::common::UUID::random();
  test(
    sm->associateAttribute(nullptr, /*attribId*/ aid1, uids[0]),
    "Inserting a new attribute should succeed");
  test(
    sm->associateAttribute(nullptr, /*attribId*/ aid2, uids[0]),
    "Inserting a new attribute should succeed");
  test(
    sm->associateAttribute(nullptr, /*attribId*/ aid1, uids[0]),
    "Inserting an attribute twice should succeed");
  test(
    sm->disassociateAttribute(nullptr, /*attribId*/ aid1, uids[0]),
    "Removing a non-existent attribute should fail");
  test(
    !sm->disassociateAttribute(nullptr, /*attribId*/ aid1, uids[1]),
    "Removing a non-existent attribute should fail");

  // Test removal of arrangement information and entities.
  // Remove a volume from its volume use and the model containing it.
  test(
    sm->unarrangeEntity(uids[21], EMBEDDED_IN, 0, true) == 1,
    "Detaching a Volume from its parent Model did not succeed.");
  test(
    sm->unarrangeEntity(uids[21], EMBEDDED_IN, 0, true) == 0,
    "Detaching a Volume from a non-existent Model did not fail.");
  test(
    sm->unarrangeEntity(uids[21], HAS_USE, 0, true) == 2, "Detaching a Volume/VolumeUse failed.");
  test(
    sm->findEntity(uids[21]) == nullptr,
    "unarrangeEntity(..., true) failed to remove the entity afterwards.");
  test(sm->erase(uids[0]), "Failed to erase a vertex.");

  std::cout << entCount << " total entities:\n";
  std::cout << "subgroups " << subgroups << "\n";
  std::cout << "submodels " << submodels << "\n";
  std::cout << "subcells " << subcells << "\n";

  return 0;
}
