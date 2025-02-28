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
#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Shell.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/geometry/Cache.h"
#include "smtk/geometry/Generator.h"
#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Manager.h"

#include "smtk/resource/Manager.h"

#include <fstream>
#include <sstream>

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;
using namespace smtk::io;

namespace
{

using TestResource = smtk::model::Resource;

// Example of a custom "geometry" format for a backend.
// We cannot use smtk::geometry::GeometryForBackend<int>
// because integers are not automatically initialized.
// Here, we just wrap an integer so that Format is
// initialized by default and convertible to a bool.
// Usually, you will GeometryForBackend a shared or
// smart pointer to some more complex structure.
struct Format
{
  int m_data = 0;
  Format() = default;
  Format(int data)
    : m_data(data)
  {
  }
  Format(const Format& other) = default;

  // This *must* return false when m_data is the default value.
  // It *may* return false under other conditions.
  operator bool() const { return m_data > 0; }

  // Return some nonsense "geometry":
  operator int() const { return m_data <= 0 ? -1 : m_data; }
};

class TestBackend : public smtk::geometry::Backend
{
public:
  TestBackend() = default;
  ~TestBackend() override = default;

  [[nodiscard]] std::string name() const override { return "TestBackend"; }
};

class TestGeometry : public smtk::geometry::Cache<smtk::geometry::GeometryForBackend<Format>>
{
public:
  TestGeometry(const TestResource::Ptr& parent)
    : m_parent(parent)
  {
  }
  ~TestGeometry() override = default;

  const smtk::geometry::Backend& backend() const override
  {
    static TestBackend data;
    return data;
  }

  smtk::geometry::Resource::Ptr resource() const override { return m_parent.lock(); }

  void queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
    const override
  {
    if (obj)
    {
      entry.m_generation = Initial;
      entry.m_geometry = Format(1);
    }
    else
    {
      entry.m_generation = Invalid;
    }
  }

  void geometricBounds(const Format& value, BoundingBox& bds) const override
  {
    bds[0] = bds[2] = bds[4] = +0.0;
    bds[1] = bds[3] = bds[5] = static_cast<double>((int)value);
  }

  // Ensure every component with renderable geometry has a cache entry.
  void update() const override {}

  TestResource::WeakPtr m_parent;
};

class RegisterTestResourceToBackend : public smtk::geometry::Supplier<RegisterTestResourceToBackend>
{
public:
  [[nodiscard]] bool valid(const smtk::geometry::Specification& in) const override
  {
    TestBackend backend;
    auto rsrc = std::dynamic_pointer_cast<TestResource>(std::get<0>(in));
    return rsrc && std::get<1>(in).index() == backend.index();
  }

  smtk::geometry::GeometryPtr operator()(const smtk::geometry::Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<TestResource>(std::get<0>(in));
    auto* provider = new TestGeometry(rsrc);
    return smtk::geometry::GeometryPtr(provider);
  }
};

} // anonymous namespace

static int numberOfInclusionsRemoved = 0;
static int numberOfFreeCellsRemoved = 0;
static int numberOfSubmodelsRemoved = 0;
static int numberOfGroupsRemoved = 0;

int didRemove(
  ResourceEventType event,
  const EntityRef& /*unused*/,
  const EntityRef& /*unused*/,
  void* /*unused*/)
{
  switch (event.second)
  {
    case MODEL_INCLUDES_MODEL:
      ++numberOfSubmodelsRemoved;
      break;
    case MODEL_INCLUDES_GROUP:
      ++numberOfGroupsRemoved;
      break;
    case MODEL_INCLUDES_FREE_CELL:
      ++numberOfFreeCellsRemoved;
      break;
    case CELL_INCLUDES_CELL:
      ++numberOfInclusionsRemoved;
      break;
    default:
      break;
  }
  return 0;
}

void testBoundingBoxFromTessellation(const smtk::model::Model& model)
{
  std::cout << "testBoundingBoxFromTessellation\n";
  {
    auto& geom = model.resource()->geometry();
    std::cout << "  Geometry " << geom.get() << " (should be null).\n";
    test(geom == nullptr, "Null geometry expected.");
  }
  std::vector<double> bbox = model.boundingBox();
  std::cout << "  Bounds (via property) " << bbox[0] << " " << bbox[2] << " " << bbox[4] << " -- "
            << bbox[1] << " " << bbox[3] << " " << bbox[5] << "\n";
  test(bbox.size() == 6, "Bounds not sized properly.");
  test(bbox[1] >= bbox[0], "Bounds invalid.");

  // Now erase the bounding box property and ensure that
  // the bbox is now reported as invalid.
  smtk::model::Model mutableModel(model);
  mutableModel.removeFloatProperty(SMTK_BOUNDING_BOX_PROP);
  for (auto cell : model.cells())
  {
    cell.removeFloatProperty(SMTK_BOUNDING_BOX_PROP);
  }
  bbox = model.boundingBox();
  test(bbox.size() == 6, "Bounds not sized properly.");
  test(bbox[1] < bbox[0], "Bounds should not be valid.");

  // Now, re-run the bounding box query but this time,
  // force it to use a Geometry object instead of the
  // property system to fetch bounds. (Using a property
  // to hold bounds is being phased out along with
  // smtk::model::Tessellation in favor of Geometry.)
  //
  // Create resource and geometry managers.
  auto resourceManager = smtk::resource::Manager::create();
  auto geometryManager = smtk::geometry::Manager::create();
  geometryManager->registerResourceManager(resourceManager);

  // Register resource type and then add resource to manager:
  bool didRegisterResource = resourceManager->registerResource<TestResource>();
  test(didRegisterResource, "Could not register TestResource type.");
  bool didAddResource = resourceManager->add(model.resource());
  test(didAddResource, "Could not add resource to manager.");

  // Register geometry backend type and then register generator for resource type:
  // Note: We must register the generator (geometryRegistered) before the backend
  // (didRegisterBackend) in order for geometry to be constructed. This is required
  // because the generator registration has no knowledge of the geometry or resource
  // managers that reference it.
  bool geometryRegistered = RegisterTestResourceToBackend::registerClass();
  test(geometryRegistered, "Could not register geometry provider for backend.");
  bool didRegisterBackend = geometryManager->registerBackend<TestBackend>();
  test(didRegisterBackend, "Could not register TestBackend type.");
  {
    auto& geom = model.resource()->geometry();
    std::cout << "  Geometry " << geom.get() << " (should be non-null).\n";
    test(!!geom, "Non-null geometry expected.");
  }
  bbox = model.boundingBox();
  std::cout << "  Bounds (via geometry) " << bbox[0] << " " << bbox[2] << " " << bbox[4] << " -- "
            << bbox[1] << " " << bbox[3] << " " << bbox[5] << "\n";
  test(bbox.size() == 6, "Bounds not sized properly.");
  test(bbox[1] >= bbox[0], "Bounds invalid.");
  std::cout << "testBoundingBoxFromTessellation... done\n\n";
}

void testComplexVertexChain()
{
  std::cout << "testComplexVertexChain\n";
  ResourcePtr sm = Resource::create();
  Vertices verts;
  VertexUses vu;
  for (int i = 0; i < 6; ++i)
  {
    verts.push_back(sm->addVertex());
    vu.push_back(sm->addVertexUse(verts.back(), 0));
  }
  VertexUses rvu; // reversed vertex-uses for testing eun.
  for (int i = 0; i < 6; ++i)
  {
    rvu.push_back(vu[6 - (i + 1)]);
  }
  // Create an edge and uses to hold our chains.
  Edge e = sm->addEdge();
  EdgeUse eun = sm->addEdgeUse(e, 0, NEGATIVE);
  EdgeUse eup = sm->addEdgeUse(e, 0, POSITIVE);
  // Create 2 top-level segments and add a subsegment to the second.
  // First top-level chain:
  Chain t0p = sm->addChain(eup).addUse(vu[0]).addUse(vu[1]);
  // Second top-level chain:
  Chain t1p = sm->addChain(eup).addUse(vu[2]).addUse(vu[5]);
  // Sub-chain:
  Chain tsp = sm->addChain(t1p).addUse(vu[3]).addUse(vu[4]);

  // Do the same in reverse order:
  Chain t1n = sm->addChain(eun).addUse(vu[5]).addUse(vu[2]);
  Chain tsn = sm->addChain(t1n).addUse(vu[4]).addUse(vu[3]);
  Chain t0n = sm->addChain(eun).addUse(vu[1]).addUse(vu[0]);

  // Now, ask the positive edge use for all of its vertex uses.
  // It should return an ordered list verts[0]...verts[5].
  test(eup.vertexUses() == vu, "Complex chain traversal failed.");
  /*
  VertexUses trvu = eun.vertexUses();
  for (int i = 0; i < 6; ++i)
    {
    std::cout << trvu[i] << "  " << rvu[i] << "\n";
    }
    */
  test(eun.vertexUses() == rvu, "Complex chain traversal failed (reversed).");
  std::cout << "testComplexVertexChain... done\n\n";
}

void testMiscConstructionMethods()
{
  std::cout << "testMiscConstructionMethods\n";
  ResourcePtr sm = Resource::create();
  Vertices verts;
  Edges edges;
  for (int i = 0; i < 6; ++i)
  {
    verts.push_back(sm->addVertex());
    if (i > 0)
    {
      edges.push_back(sm->addEdge());
      edges[i - 1].addRawRelation(verts[i - 1]);
      edges[i - 1].addRawRelation(verts[i]);
      test(
        edges[i - 1].relationsAs<EntityRefArray>().size() == 2,
        "Expected addRawRelation() to add a relation.");
    }
  }
  edges.push_back(sm->addEdge());
  edges[5].addRawRelation(verts[5]);
  edges[5].addRawRelation(verts[0]);
  test(
    edges[5].relationsAs<EntityRefArray>().size() == 2,
    "Expected addRawRelation() to add a relation.");

  // Should have no effect:
  edges[0].findOrAddRawRelation(verts[0]);
  test(
    edges[0].relationsAs<EntityRefArray>().size() == 2,
    "Expected findOrAddRawRelation() to skip adding a duplicate relation.");

  // Should have an effect:
  edges[0].addRawRelation(verts[0]);
  test(
    edges[0].relationsAs<EntityRefArray>().size() == 3,
    "Expected addRawRelation() to add a duplicate relation.");

  // Should not include duplicates:
  test(
    edges[0].relationsAs<EntityRefs>().size() == 2,
    "Expected relationsAs<EntityRefs> (a set) to remove duplicates.");

  std::cout << "testMiscConstructionMethods... done\n\n";
}

void testVolumeEntityRef()
{
  std::cout << "testVolumeEntityRef\n";
  ResourcePtr sm = Resource::create();
  createTet(sm);
  Volumes vols = sm->entitiesMatchingFlagsAs<Volumes>(VOLUME, true);
  test(vols.size() == 1, "Expected a single volume in the test model.");
  Volume vol = vols[0];
  Faces faces = vol.faces();
  test(faces.size() == 5, "Expected 5 faces in the test model.");
  test(vol.use().isValid(), "Expected a valid volume use.");
  Shells shells = vol.shells();
  test(shells.size() == 1, "Expected a single shell (use) in the test model.");
  std::cout << "testVolumeEntityRef... done\n\n";
}

void testModelMethods()
{
  std::cout << "testModelMethods\n";
  // Test methods specific to the Model subclass of EntityRef
  ResourcePtr sm = Resource::create();
  Session::Ptr session = Session::create();
  sm->registerSession(session);
  SessionRef sess(sm, session);
  Model m0 = sm->addModel();
  test(!m0.session().isValid(), "Expected invalid session for new, \"blank\" model.");
  m0.setSession(sess);
  test(m0.session() == sess, "Expected valid session when assigned to model.");

  Model m1 = sm->addModel();
  m0.addSubmodel(m1);
  std::cout << "  m1 " << m1.name() << " m0 " << m0.name() << " m1p " << m1.parent().name() << "\n";
  std::cout << "  m1 " << m1.session().name() << " m0 " << m0.session().name() << "\n";
  test(m1.session() == m0.session(), "Expected sessions to match for model and its submodel.");
  std::cout << "testModelMethods... done\n\n";
}

void testResourceComponentConversion()
{
  std::cout << "testResourceComponentConversion\n";
  ResourcePtr sm = Resource::create();
  Session::Ptr session = Session::create();
  sm->registerSession(session);
  SessionRef sess(sm, session);
  Model m0 = sm->addModel();
  smtk::model::EntityPtr mep = m0.entityRecord();
  test(!!mep, "No component for Model entity-ref.");
  test(mep->referenceAs<smtk::model::Model>() == m0, "Could not convert from component to ref.");
  test(smtk::model::Model(mep) == m0, "Could not convert from ref to component.");
  smtk::resource::ComponentPtr cmp = m0.component();
  test(
    cmp == smtk::dynamic_pointer_cast<smtk::resource::Component>(mep),
    "Component/Entity mismatch.");
  std::cout << "testResourceComponentConversion... done\n\n";
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  try
  {
    ResourcePtr sm = Resource::create();
    sm->observe(ResourceEventType(DEL_EVENT, CELL_INCLUDES_CELL), didRemove, nullptr);
    sm->observe(ResourceEventType(DEL_EVENT, MODEL_INCLUDES_FREE_CELL), didRemove, nullptr);
    sm->observe(ResourceEventType(DEL_EVENT, MODEL_INCLUDES_GROUP), didRemove, nullptr);
    sm->observe(ResourceEventType(DEL_EVENT, MODEL_INCLUDES_MODEL), didRemove, nullptr);

    UUIDArray uids = createTet(sm);

    std::cout << "testConstruction\n";
    Model model = sm->addModel(3, 3, "TestModel");
    // Test Model::parent()
    test(!model.parent().isValid(), "Toplevel model should have an invalid parent.");
    // Test that free cells may be added to a model,
    // and that they are only reported back as free
    // cells (and not other related entities via
    // casts to invalid entityref types).
    // Also, check lowerDimensionalBoundaries and higherDimensionalBordants.
    Volume tet = Volume(sm, uids[21]);
    test(
      tet.lowerDimensionalBoundaries(2).size() == 5,
      "The test model volume should have had 5 surface boundaries.");
    test(
      tet.lowerDimensionalBoundaries(1).size() == 9,
      "The test model volume should have had 9 surface+edge boundaries.");
    test(
      tet.lowerDimensionalBoundaries(0).size() == 7,
      "The test model volume should have had 7 surface+edge+vertex boundaries.");
    test(
      tet.lowerDimensionalBoundaries(-1).size() == 21,
      "The test model volume should have had 21 boundaries (total).");

    Vertex vrt = Vertex(sm, uids[0]);
    test(
      vrt.higherDimensionalBordants(1).size() == 3,
      "The test model vert should have had 3 edge boundaries.");
    test(
      vrt.higherDimensionalBordants(2).size() == 3,
      "The test model vert should have had 3 surface+edge boundaries.");
    test(
      vrt.higherDimensionalBordants(3).size() == 1,
      "The test model vert should have had 1 edge+surface+volume boundaries.");
    test(
      vrt.higherDimensionalBordants(-1).size() == 7,
      "The test model vert should have had 7 boundaries (total).");

    // Test Model::addCell()
    model.addCell(tet);
    test(model.isEmbedded(tet), "Tetrahedron not embedded in model");
    test(!model.cells().empty() && model.cells()[0] == tet, "Tetrahedron not a free cell of model");
    // Test Model::groups() when empty
    test(model.groups().empty(), "Tetrahedron should not be reported as a group.");
    // Test Model::submodels()
    test(model.submodels().empty(), "Tetrahedron should not be reported as a submodel.");

    sm->assignDefaultNames();
    EntityRefs entities;
    EntityRef::EntityRefsFromUUIDs(entities, sm, uids);
    std::cout << "  " << uids.size() << " uids, " << entities.size() << " entities\n";
    test(
      uids.size() == entities.size(),
      "Translation from UUIDs to entityrefs should not omit entries");
    Group bits = sm->addGroup(0, "Bits'n'pieces");
    // Test Group::addEntities()
    bits.addEntities(entities);
    // Test Model::addGroup()
    model.addGroup(bits);
    // Test Group::parent()
    test(bits.parent() == model, "Parent of group incorrect.");
    // Test Model::groups() when non-empty
    test(
      !model.groups().empty() && model.groups()[0] == bits,
      "Group should be reported as member of model.");
    // Test Group::members().
    CellEntities groupCells = bits.members<CellEntities>();
    UseEntities groupUses = bits.members<UseEntities>();
    ShellEntities groupShells = bits.members<ShellEntities>();
    std::cout << "  " << uids.size() << " entities = " << groupCells.size() << " cells + "
              << groupUses.size() << " uses + " << groupShells.size() << " shells\n";
    test(groupCells.size() == 22, "Cell group has wrong number of members.");
    test(groupUses.size() == 30, "Use group has wrong number of members.");
    test(groupShells.size() == 25, "Shell group has wrong number of members.");

    Instance ie = sm->addInstance(model);
    test(ie.prototype() == model, "Instance parent should be its prototype.");
    InstanceEntities ies = model.instances<InstanceEntities>();
    test(ies.size() == 1 && ies[0] == ie, "Prototype should list its instances.");

    EntityRef entity(sm, uids[0]);
    test(entity.isValid());
    test(entity.dimension() == 0);
    test((entity.entityFlags() & ANY_ENTITY) == (CELL_ENTITY | DIMENSION_0));

    for (int dim = 1; dim <= 3; ++dim)
    {
      entities = entity.bordantEntities(dim);
      test(entities.size() == sm->bordantEntities(uids[0], dim).size());
      entities = entity.higherDimensionalBordants(dim);
      test(entities.size() == sm->higherDimensionalBordants(uids[0], dim).size());
    }

    test(entity.isCellEntity(), "isCellEntity() incorrect");
    test(!entity.isUseEntity(), "isUseEntity() incorrect");
    test(!entity.isShellEntity(), "isShellEntity() incorrect");
    test(!entity.isGroup(), "isGroup() incorrect");
    test(!entity.isModel(), "isModel() incorrect");
    test(!entity.isInstance(), "isInstance() incorrect");

    test(entity.isVertex(), "isVertex() incorrect");
    test(!entity.isEdge(), "isEdge() incorrect");
    test(!entity.isFace(), "isFace() incorrect");
    test(!entity.isVolume(), "isVolume() incorrect");
    test(!entity.isChain(), "isChain() incorrect");
    test(!entity.isLoop(), "isLoop() incorrect");
    test(!entity.isShell(), "isShell() incorrect");
    test(!entity.isVertexUse(), "isVertexUse() incorrect");
    test(!entity.isEdgeUse(), "isEdgeUse() incorrect");
    test(!entity.isFaceUse(), "isFaceUse() incorrect");

    // Test that "cast"ing to EntityRef subclasses works
    // and that they are valid or invalid as appropriate.
    CellEntity cell = entity.as<CellEntity>();
    test(cell.model() == model, "Vertex has incorrect owning-model");

    UseEntity use = entity.as<UseEntity>();
    Vertex vert = entity.as<Vertex>();
    std::cout << "  " << vert << "\n";
    //std::cout << "  " << vert.coordinates().transpose() << "\n";
    test(cell.isValid(), "CellEntity::isValid() incorrect");
    test(!use.isValid(), "UseEntity::isValid() incorrect");
    // Test obtaining uses from cells. Currently returns an empty set
    // because we are not properly constructing/arranging the solid.
    VertexUses vertexUses = cell.uses<VertexUses>();

    entity = EntityRef(sm, uids[21]);
    test(entity.isValid());
    test(entity.dimension() == 3);

    for (int dim = 2; dim >= 0; --dim)
    {
      entities = entity.boundaryEntities(dim);
      test(entities.size() == sm->boundaryEntities(uids[21], dim).size());
      entities = entity.lowerDimensionalBoundaries(dim);
      test(entities.size() == sm->lowerDimensionalBoundaries(uids[21], dim).size());
    }
    std::cout << "testConstruction... done\n\n";

    testBoundingBoxFromTessellation(model);

    std::cout << "testProperties\n";
    entity.setFloatProperty("perpendicular", 1.57);
    test(
      entity.floatProperty("perpendicular").size() == 1 &&
      entity.floatProperty("perpendicular")[0] == 1.57);

    entity.setStringProperty("name", "Tetrahedron");
    test(
      entity.stringProperty("name").size() == 1 &&
      entity.stringProperty("name")[0] == "Tetrahedron");

    entity.setIntegerProperty("7beef", 507631);
    test(
      entity.integerProperty("7beef").size() == 1 && entity.integerProperty("7beef")[0] == 507631);

    std::cout << "  " << entity << "\n";

    // Test exclusion properties
    test(
      entity.exclusions() == Exclusions::Nothing, "Entity's exclusion status should be Nothing.");
    entity.setExclusions(true); // Exclude it from everything
    test(
      entity.exclusions() == Exclusions::Everything,
      "Entity's exclusion status should be Everything");
    entity.setExclusions(false); // Reset exclusion status
    test(entity.exclusions() == Exclusions::Nothing, "Entity's exclusion status should be Nothing");
    // Set excluded from tessellation generation
    entity.setExclusions(true, Exclusions::Rendering);
    test(
      entity.exclusions(Exclusions::Rendering) == 1,
      "Entity should be excluded"
      " from rendering");
    // Set excluded from view representation
    entity.setExclusions(true, Exclusions::ViewPresentation);
    test(
      entity.exclusions(Exclusions::ViewPresentation) == 1,
      "Entity should be excluded"
      " from view presentation");

    // Test color/hasColor/setColor
    test(!entity.hasColor(), "Entity should not have had a color assigned to it.");
    smtk::model::FloatList rgba = entity.color();
    test(rgba.size() == 4, "Colors (even undefined) should have 4 components.");
    test(rgba[3] == -1., "Undefined color should have negative alpha.");
    entity.setColor(1., 0., 0.);
    rgba = entity.color();
    test(rgba[3] == 1., "Default alpha should be opaque.");
    rgba[3] = 0.5;
    entity.setColor(rgba);
    rgba = entity.color();
    test(rgba[3] == .5, "Alpha not set.");

    entity = EntityRef(sm, UUID::null());
    test(entity.dimension() == -1);
    test(!entity.isValid());

    // Verify that setting properties on an invalid entityref works.
    entity.setFloatProperty("perpendicular", 1.57);
    entity.setStringProperty("name", "Tetrahedron");
    entity.setIntegerProperty("7beef", 507631);
    // The above should have had no effect since the entityref is invalid:
    test(!entity.hasFloatProperty("perpendicular"));
    test(!entity.hasStringProperty("name"));
    test(!entity.hasIntegerProperty("7beef"));
    std::cout << "testProperties... done\n\n";

    std::cout << "testModeling\n";
    // Test that face entity was created with invalid (but present) face uses.
    Face f(sm, uids[20]);
    test(f.volumes().size() == 1 && f.volumes()[0].isVolume());

    test(!f.positiveUse().isValid(), "Positive use");
    test(f.negativeUse().isValid(), "Negative use" + f.entity().toString());

    Vertex v = sm->addVertex();
    sm->addVertexUse(v, 0);
    v.setStringProperty("name", "Loopy");
    std::cout << "  " << v << "\n";
    sm->findOrAddInclusionToCellOrModel(uids[21], v.entity());
    // Now perform the same operation another way to ensure that
    // the existing arrangement blocks the new one from being
    // redundantly created:
    Volume vol(sm, uids[21]);
    vol.embedEntity(v);

    test(
      !vol.inclusions<CellEntities>().empty() && vol.inclusions<CellEntities>()[0] == v,
      "Volume should have an included vertex.");

    // Test removing cell inclusions and model members.
    // a. Cell inclusion
    vol.unembedEntity(v);
    test(numberOfInclusionsRemoved == 1, "Did not unembed vertex from volume");

    // b. Free cell in model
    model.addCell(v);
    model.removeCell(v);
    test(numberOfFreeCellsRemoved == 1, "Did not remove free vertex from model");

    // c. Group in model
    model.removeGroup(bits);
    test(numberOfGroupsRemoved == 1, "Did not remove group from model");

    // d. Submodel of model
    Model submodel = sm->addModel(3, 3, "Surplus model");
    model.addSubmodel(submodel);
    model.removeSubmodel(submodel);
    test(numberOfSubmodelsRemoved == 1, "Did not remove submodel from model");

    // Test Volume::shells()
    Shells shells = vol.shells();
    test(shells == vol.use().shells(), "Volume::use() test failed.");
    test((shells.size() == 1) && shells[0].isValid(), "Volume should have 1 top-level shell.");

    Shell sh(shells[0]);

    test(!sh.containingShell().isValid(), "Top-level shell should have no container.");
    test(sh.containedShells().empty(), "Top-level shell should no containees.");
    test(sh.containedShells().empty(), "Top-level shell should no containees.");
    test(sh.volume() == vol, "Top-level shell's volume does not match volume it came from.");

    FaceUses fu(sh.faceUses());

    test(fu.size() == 5, "Shell should have 5 faces.");
    test(
      fu[0].orientation() == NEGATIVE,
      "Face-uses of sample tet should all have negative orientation.");
    test(fu[0].sense() == 0, "Face-uses of sample tet should be sense 0.");
    test(fu[0].volume() == vol, "Face-use did not report the correct volume it bounds.");

    // createTet generates the first face of the tet with a hole containing
    // another triangular face.
    // We test that ShellEntity::parentCell() properly traverses the
    // containment relationship to find the toplevel Loop when we start with
    // the inner loop of the face.
    Loops loops = fu[0].loops();
    test(loops.size() == 1, "FaceUse should only report 1 outer loop.");
    loops = loops[0].containedLoops();
    test(loops.size() == 1, "First face-use should have 1 inner loop.");
    Loop innerLoop = loops[0];
    test(
      innerLoop.face() == fu[0].face(),
      "Inner loop of face's loop should report face as parent cell.");
    EdgeUses ieus = innerLoop.edgeUses();
    test(ieus.size() == 3, "Inner loop of face 0 should have 3 edges.");
    test(
      ieus[0].faceUse() == fu[0],
      "EdgeUse failed to report proper FaceUse associated with its Loop.");

    // Now test EdgeUse methods.
    // Test EdgeUse::vertices(); the first edge of the inner loop should
    // connect vertices 3-4. The order is reversed because fu[0] is a
    // negatively-oriented face-use and the inner loop is a hole in the face.
    Vertices ieverts = ieus[0].vertices();
    test(ieverts.size() == 2, "Bad number of innerLoop edge vertices.");
    test(
      ieverts[0].entity() == uids[4] && ieverts[1].entity() == uids[3],
      "Bad innerLoop, edge 0 vertices.");

    // Let's look at uses of the same edge from different triangles.
    EdgeUses oeus = fu[1].loops()[0].edgeUses(); // edge uses of outer loop of "hole" face-use
    test(oeus.size() == 3, "Expecting a triangle.");
    for (int i = 0; i < 3; ++i)
    {
      std::ostringstream msg;
      msg << "Edge-uses sharing the same edge in different contexts should be different.\n"
          << "Loop entry " << i << ":  " << ieus[i] << " (" << ieus[i].edge() << " "
          << ieus[i].sense() << " " << (ieus[i].orientation() == POSITIVE ? "+" : "-") << ")  "
          << oeus[i] << " (" << oeus[i].edge() << " " << oeus[i].sense() << " "
          << (oeus[i].orientation() == POSITIVE ? "+" : "-") << ")\n";
      test(
        oeus[2 - i].edge() == ieus[i].edge() && oeus[2 - i].sense() != ieus[i].sense() &&
          oeus[2 - i].orientation() != ieus[i].orientation(),
        msg.str());
      test(ieus[i].loop() == innerLoop, "EdgeUse did not point to proper parent loop.");
    }

    // Test Edge and Chain methods:
    Edges allEdges;
    EntityRef::EntityRefsFromUUIDs(allEdges, sm, sm->entitiesMatchingFlags(EDGE, true));
    for (Edges::iterator edgeIt = allEdges.begin(); edgeIt != allEdges.end(); ++edgeIt)
    {
      Edge curEdge(*edgeIt);
      EdgeUses curUses(curEdge.edgeUses());
      //std::cout << "Edge \"" << curEdge << "\" has " << curUses.size() << " uses\n";
      for (EdgeUses::iterator useIt = curUses.begin(); useIt != curUses.end(); ++useIt)
      {
        //std::cout << "    " << *useIt << " chains:\n";
        Chains curChains(useIt->chains());
        test(!curChains.empty(), "EdgeUses should not have empty chains.");
        for (Chains::iterator chainIt = curChains.begin(); chainIt != curChains.end(); ++chainIt)
        {
          test(
            !chainIt->containingChain().isValid(),
            "Top-level chains should not have parent chain.");
          Chains subchains = chainIt->containedChains();
          //std::cout << "        " << *chainIt << " with " << subchains.size() << " subchains\n";
          VertexUses vu = chainIt->vertexUses();
          test(vu.size() == 2, "All sample tet edges should have 2 vertex uses.");
        }
      }
      // Every edge should have at least 2 uses and they should come in pairs
      // with opposite orientations.
      test(!curUses.empty() && curUses.size() % 2 == 0);
    }

    // Test Vertex and VertexUse methods:
    Vertices allVerts;
    EntityRef::EntityRefsFromUUIDs(allVerts, sm, sm->entitiesMatchingFlags(VERTEX, true));
    int n6 = 0;
    int n4 = 0;
    for (Vertices::iterator vertIt = allVerts.begin(); vertIt != allVerts.end(); ++vertIt)
    {
      test(
        vertIt->uses<VertexUses>().size() == 1,
        "All sample tet vertices should have a single use.");
      VertexUse vu(vertIt->uses<VertexUses>()[0]);
      int n = static_cast<int>(vu.chains().size());
      test(
        n == 0 || n == 4 || n == 6, "VertexUses on sample tet should have 0, 4, or 6 chains each.");
      n6 += (n == 6 ? 1 : 0);
      n4 += (n == 4 ? 1 : 0);
      //std::cout << vu << " (" << vu.vertex() << " sense " << vu.sense() << ") " << n << "\n";
    }
    test(n6 == 4, "4 corner vertex-uses should have 6 chains each.");
    test(n4 == 3, "3 inner-face vertex-uses should have 4 chains each.");
    std::cout << "testModeling... done\n\n";

    testComplexVertexChain();
    testMiscConstructionMethods();
    testVolumeEntityRef();
    testModelMethods();
    testResourceComponentConversion();
  }
  catch (const std::string& msg)
  {
    (void)msg; // the test will already have printed the message.
    return 1;
  }

  return 0;
}
