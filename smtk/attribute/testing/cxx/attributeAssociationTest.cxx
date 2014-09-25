#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Vertex.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

int main()
{
  // ----
  // I. First see how things work when Manager is not yet set.
  attribute::Manager mgr;
  test(
    !mgr.refModelManager(),
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
  //     a valid model modelMgr pointer.
  model::Manager::Ptr modelMgr = model::Manager::create();
  mgr.setRefModelManager(modelMgr);
  test(
    mgr.refModelManager() == modelMgr,
    "Could not set attribute manager's model-manager.");

  test(
    att->modelManager() == modelMgr,
    "Attribute's idea of model manager incorrect.");

  smtk::model::Vertex v0 = modelMgr->addVertex();
  smtk::model::Vertex v1 = modelMgr->addVertex();
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
  // III. Test corner cases when switch model managers on the attribute manager.
  model::Manager::Ptr auxModelManager = model::Manager::create();
  mgr.setRefModelManager(auxModelManager);
  test(
    mgr.refModelManager() == auxModelManager,
    "Attribute manager's modelMgr not changed.");
  test(
    auxModelManager->attributeManager() == &mgr,
    "Second model manager's attribute manager not changed.");
  test(
    modelMgr->attributeManager() == NULL,
    "Original model manager's attribute manager not reset.");

  return 0;
}
