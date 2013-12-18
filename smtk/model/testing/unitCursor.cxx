#include "smtk/model/Cursor.h"

#include "smtk/model/ExportJSON.h"
#include "smtk/model/Storage.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"

#include "smtk/util/Testing/helpers.h"
#include "smtk/model/testing/helpers.h"

using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;
using smtk::shared_ptr;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  StoragePtr sm = Storage::New();
  UUIDArray uids = createTet(sm);

  Cursors entities;

  Cursor entity(sm, uids[0]);
  test(entity.isValid() == true);
  test(entity.dimension() == 0);
  test((entity.entityFlags() & ANY_ENTITY) == (CELL_ENTITY | DIMENSION_0));

  for (int dim = 1; dim <= 3; ++dim)
    {
    entities = entity.bordantEntities(dim);
    test(entities.size() == sm->bordantEntities(uids[0], dim).size());
    entities = entity.higherDimensionalBordants(dim);
    test(entities.size() == sm->higherDimensionalBordants(uids[0], dim).size());
    }

  test( entity.isCellEntity()     && "isCellEntity() incorrect");
  test(!entity.isUseEntity()      && "isUseEntity() incorrect");
  test(!entity.isShellEntity()    && "isShellEntity() incorrect");
  test(!entity.isGroupEntity()    && "isGroupEntity() incorrect");
  test(!entity.isModelEntity()    && "isModelEntity() incorrect");
  test(!entity.isInstanceEntity() && "isInstanceEntity() incorrect");

  test( entity.isVertex()    && "isVertex() incorrect");
  test(!entity.isEdge()      && "isEdge() incorrect");
  test(!entity.isFace()      && "isFace() incorrect");
  test(!entity.isVolume()    && "isVolume() incorrect");
  test(!entity.isChain()     && "isChain() incorrect");
  test(!entity.isLoop()      && "isLoop() incorrect");
  test(!entity.isShell()     && "isShell() incorrect");
  test(!entity.isVertexUse() && "isVertexUse() incorrect");
  test(!entity.isEdgeUse()   && "isEdgeUse() incorrect");
  test(!entity.isFaceUse()   && "isFaceUse() incorrect");

  // Test that "cast"ing to Cursor subclasses works
  // and that they are valid or invalid as appropriate.
  CellEntity cell = entity.as<CellEntity>();
  UseEntity use = entity.as<UseEntity>();
  Vertex vert = entity.as<Vertex>();
  //std::cout << vert.coordinates().transpose() << "\n";
  test(cell.isValid() && "CellEntity::isValid() incorrect");
  test(!use.isValid() && "UseEntity::isValid() incorrect");
  // Test obtaining uses from cells. Currently returns an empty set
  // because we are not properly constructing/arranging the solid.
  UseEntities vertexUses = cell.uses();

  entity = Cursor(sm, uids[21]);
  test(entity.isValid() == true);
  test(entity.dimension() == 3);

  for (int dim = 2; dim >= 0; --dim)
    {
    entities = entity.boundaryEntities(dim);
    test(entities.size() == sm->boundaryEntities(uids[21], dim).size());
    entities = entity.lowerDimensionalBoundaries(dim);
    test(entities.size() == sm->lowerDimensionalBoundaries(uids[21], dim).size());
    }

  entity.setFloatProperty("perpendicular", 1.57);
  test(
    entity.floatProperty("perpendicular").size() == 1 &&
    entity.floatProperty("perpendicular")[0] == 1.57);

  entity.setStringProperty("name", "Tetrahedron");
  test(
    entity.stringProperty("name").size() == 1 &&
    entity.stringProperty("name")[0] == "Tetrahedron");

  entity.setIntegerProperty("deadbeef", 3735928559);
  test(
    entity.integerProperty("deadbeef").size() == 1 &&
    entity.integerProperty("deadbeef")[0] == 3735928559);

  entity = Cursor(sm, UUID::null());
  test(entity.dimension() == -1);
  test(entity.isValid() == false);

  // Verify that setting properties on an invalid cursor works.
  entity.setFloatProperty("perpendicular", 1.57);
  entity.setStringProperty("name", "Tetrahedron");
  entity.setIntegerProperty("deadbeef", 3735928559);
  // The above should have had no effect since the cursor is invalid:
  test(entity.hasFloatProperty("perpendicular") == false);
  test(entity.hasStringProperty("name") == false);
  test(entity.hasIntegerProperty("deadbeef") == false);

  // Verify that attribute assignment works (with some
  // made-up attribute IDs)
  test(!entity.hasAttributes(), "Detecting an un-associated attribute");
  test( entity.attachAttribute(1), "Attaching an attribute");
  test(!entity.attachAttribute(1), "Re-attaching a repeated attribute");
  test( entity.detachAttribute(1), "Detaching an associated attribute");
  test(!entity.detachAttribute(2), "Detaching an un-associated attribute");

  return 0;
}
