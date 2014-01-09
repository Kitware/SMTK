#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Vertex.h"

#include "smtk/util/Testing/helpers.h"

using namespace smtk::attribute;
using namespace smtk::model;
using namespace smtk::util;

int main()
{
  // ----
  // I. First see how things work when Storage is not yet set.
  Manager mgr;
  test(
    !mgr.refStorage(),
    "Manager should not have model storage by default.");

  DefinitionPtr def = mgr.createDefinition("testDef");
  AttributePtr att = mgr.createAttribute("testAtt", "testDef");

  UUID fakeEntityId = UUID::random();
  att->associateEntity(fakeEntityId);
  test(
    att->associatedModelEntityIds().count(fakeEntityId) == 1,
    "Could not associated a \"fake\" entity with this attribute.");

  // Attempt to disassociate an entity that was never associated.
  UUID anotherFakeId = UUID::random();
  att->disassociateEntity(anotherFakeId);

  att->disassociateEntity(fakeEntityId);
  test(
    att->isEntityAssociated(fakeEntityId) == false,
    "Could not disassociate a \"fake\" entity from this attribute.");

  // ----
  // II. Now see how things work when the attribute manager has
  //     a valid model storage pointer.
  Storage::Ptr storage = Storage::create();
  mgr.setRefStorage(storage);
  test(
    mgr.refStorage() == storage,
    "Could not set manager's model storage.");

  test(
    att->modelStorage() == storage,
    "Attribute's idea of model storage incorrect.");

  Vertex v0 = storage->addVertex();
  Vertex v1 = storage->addVertex();
  v0.attachAttribute(att->id());
  test(
    att->associatedModelEntityIds().count(v0.entity()) == 1,
    "Could not associate a vertex to an attribute.");

  att->disassociateEntity(v0.entity());
  test(
    !v0.hasAttributes(),
    "Disassociating an attribute did not notify the entity.");

  att->disassociateEntity(v1.entity());
  test(
    !v1.hasAttributes(),
    "Disassociating a non-existent attribute appears to associate it.");

  v1.attachAttribute(att->id());
  att->removeAllAssociations();
  test(
    att->associatedModelEntityIds().empty(),
    "Removing all attribute associations did not empty association list.");

  // ----
  // III. Test corner cases when switch model storage on the manager.
  Storage::Ptr auxStorage = Storage::create();
  mgr.setRefStorage(auxStorage);
  test(
    mgr.refStorage() == auxStorage,
    "Attribute manager's storage not changed.");
  test(
    auxStorage->attributeManager() == &mgr,
    "Second storage's attribute manager not changed.");
  test(
    storage->attributeManager() == NULL,
    "Original storage's attribute manager not reset.");

  return 0;
}
