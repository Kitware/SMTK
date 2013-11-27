#include "smtk/model/Cursor.h"

#include "smtk/model/ExportJSON.h"
#include "smtk/model/Storage.h"

#include "smtk/model/testing/helpers.h"

#include <assert.h>

using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  UUIDsToEntities smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  StoragePtr sm = StoragePtr(new Storage(&smTopology, &smArrangements, &smTessellation));

  UUIDArray uids = createTet(*sm.get());

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

  assert( entity.isVertex()    && "isVertex() incorrect");
  assert(!entity.isEdge()      && "isEdge() incorrect");
  assert(!entity.isFace()      && "isFace() incorrect");
  assert(!entity.isRegion()    && "isRegion() incorrect");
  assert(!entity.isChain()     && "isChain() incorrect");
  assert(!entity.isLoop()      && "isLoop() incorrect");
  assert(!entity.isShell()     && "isShell() incorrect");
  assert(!entity.isVertexUse() && "isVertexUse() incorrect");
  assert(!entity.isEdgeUse()   && "isEdgeUse() incorrect");
  assert(!entity.isFaceUse()   && "isFaceUse() incorrect");

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
