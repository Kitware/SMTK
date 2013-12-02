#include "smtk/model/Cursor.h"

#include "smtk/model/ExportJSON.h"
#include "smtk/model/Storage.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"

#include "smtk/model/testing/helpers.h"

#include <assert.h>

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
  assert(entity.isValid() == true);
  assert(entity.dimension() == 0);
  assert((entity.entityFlags() & ANY_ENTITY) == (CELL_ENTITY | DIMENSION_0));

  for (int dim = 1; dim <= 3; ++dim)
    {
    entities = entity.bordantEntities(dim);
    assert(entities.size() == sm->bordantEntities(uids[0], dim).size());
    entities = entity.higherDimensionalBordants(dim);
    assert(entities.size() == sm->higherDimensionalBordants(uids[0], dim).size());
    }

  assert( entity.isCellEntity()     && "isCellEntity() incorrect");
  assert(!entity.isUseEntity()      && "isUseEntity() incorrect");
  assert(!entity.isShellEntity()    && "isShellEntity() incorrect");
  assert(!entity.isGroupEntity()    && "isGroupEntity() incorrect");
  assert(!entity.isModelEntity()    && "isModelEntity() incorrect");
  assert(!entity.isInstanceEntity() && "isInstanceEntity() incorrect");

  assert( entity.isVertex()    && "isVertex() incorrect");
  assert(!entity.isEdge()      && "isEdge() incorrect");
  assert(!entity.isFace()      && "isFace() incorrect");
  assert(!entity.isVolume()    && "isVolume() incorrect");
  assert(!entity.isChain()     && "isChain() incorrect");
  assert(!entity.isLoop()      && "isLoop() incorrect");
  assert(!entity.isShell()     && "isShell() incorrect");
  assert(!entity.isVertexUse() && "isVertexUse() incorrect");
  assert(!entity.isEdgeUse()   && "isEdgeUse() incorrect");
  assert(!entity.isFaceUse()   && "isFaceUse() incorrect");

  // Test that "cast"ing to Cursor subclasses works
  // and that they are valid or invalid as appropriate.
  CellEntity cell = entity.as<CellEntity>();
  UseEntity use = entity.as<UseEntity>();
  Vertex vert = entity.as<Vertex>();
  //std::cout << vert.coordinates().transpose() << "\n";
  assert(cell.isValid() && "CellEntity::isValid() incorrect");
  assert(!use.isValid() && "UseEntity::isValid() incorrect");
  // Test obtaining uses from cells. Currently returns an empty set
  // because we are not properly constructing/arranging the solid.
  UseEntities vertexUses = cell.uses();

  entity = Cursor(sm, uids[21]);
  assert(entity.isValid() == true);
  assert(entity.dimension() == 3);

  for (int dim = 2; dim >= 0; --dim)
    {
    entities = entity.boundaryEntities(dim);
    assert(entities.size() == sm->boundaryEntities(uids[21], dim).size());
    entities = entity.lowerDimensionalBoundaries(dim);
    assert(entities.size() == sm->lowerDimensionalBoundaries(uids[21], dim).size());
    }

  entity.setFloatProperty("perpendicular", 1.57);
  assert(
    entity.floatProperty("perpendicular").size() == 1 &&
    entity.floatProperty("perpendicular")[0] == 1.57);

  entity.setStringProperty("name", "Tetrahedron");
  assert(
    entity.stringProperty("name").size() == 1 &&
    entity.stringProperty("name")[0] == "Tetrahedron");

  entity.setIntegerProperty("deadbeef", 3735928559);
  assert(
    entity.integerProperty("deadbeef").size() == 1 &&
    entity.integerProperty("deadbeef")[0] == 3735928559);

  entity = Cursor(sm, UUID::null());
  assert(entity.dimension() == -1);
  assert(entity.isValid() == false);

  // Verify that setting properties on an invalid cursor works.
  entity.setFloatProperty("perpendicular", 1.57);
  entity.setStringProperty("name", "Tetrahedron");
  entity.setIntegerProperty("deadbeef", 3735928559);
  // The above should have had no effect since the cursor is invalid:
  assert(entity.hasFloatProperty("perpendicular") == false);
  assert(entity.hasStringProperty("name") == false);
  assert(entity.hasIntegerProperty("deadbeef") == false);

  return 0;
}
