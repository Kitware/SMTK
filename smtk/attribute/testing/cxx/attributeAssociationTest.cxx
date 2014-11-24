//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Vertex.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

int main()
{
  // ----
  // I. First see how things work when System is not yet set.
  attribute::System sys;
  test(
    !sys.refModelManager(),
    "System should not have model storage by default.");

  DefinitionPtr def = sys.createDefinition("testDef");
  def->associationRule()->setMembershipMask(smtk::model::VERTEX);
  def->associationRule()->setIsExtensible(true);
  def->associationRule()->setMaxNumberOfValues(2);
  AttributePtr att = sys.createAttribute("testAtt", "testDef");

  UUID fakeEntityId = UUID::random();
  att->associateEntity(fakeEntityId);
  test(
    att->associatedModelEntityIds().count(fakeEntityId) == 1,
    "Could not associate a \"fake\" entity with this attribute.");

  // Attempt to disassociate an entity that was never associated.
  UUID anotherFakeId = UUID::random();
  att->disassociateEntity(anotherFakeId);

  att->disassociateEntity(fakeEntityId);
  test(
    att->isEntityAssociated(fakeEntityId) == false,
    "Could not disassociate a \"fake\" entity from this attribute.");

  // ----
  // II. Now see how things work when the attribute system has
  //     a valid model modelMgr pointer.
  model::Manager::Ptr modelMgr = model::Manager::create();
  sys.setRefModelManager(modelMgr);
  test(
    sys.refModelManager() == modelMgr,
    "Could not set attribute system's model-manager.");

  test(
    att->modelManager() == modelMgr,
    "Attribute's idea of model manager incorrect.");

  smtk::model::Vertex v0 = modelMgr->addVertex();
  smtk::model::Vertex v1 = modelMgr->addVertex();
  v0.associateAttribute(att->id());
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

  v1.associateAttribute(att->id());
  att->removeAllAssociations();
  test(
    att->associatedModelEntityIds().empty(),
    "Removing all attribute associations did not empty association list.");

  smtk::model::Vertex v2 = modelMgr->addVertex();
  v0.associateAttribute(att->id());
  v1.associateAttribute(att->id());
  test(
    v2.associateAttribute(att->id()) == false,
    "Should not have been able to associate more than 2 entities.");

  att->removeAllAssociations();
  smtk::model::Edge e0 = modelMgr->addEdge();
  test(
    e0.associateAttribute(att->id()) == false,
    "Should not have been able to associate entity of wrong type.");

  // ----
  // III. Test corner cases when switch model managers on the attribute system.
  model::Manager::Ptr auxModelManager = model::Manager::create();
  sys.setRefModelManager(auxModelManager);
  test(
    sys.refModelManager() == auxModelManager,
    "Attribute system's modelMgr not changed.");
  test(
    auxModelManager->attributeSystem() == &sys,
    "Second model manager's attribute system not changed.");
  test(
    modelMgr->attributeSystem() == NULL,
    "Original model manager's attribute system not reset.");

  return 0;
}
