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
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/attribute/operators/Associate.h"
#include "smtk/attribute/operators/Dissociate.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

int unitAttributeAssociation(int /*unused*/, char* /*unused*/[])
{
  // ----
  // I. First see how things work when Resource is not yet set.
  attribute::ResourcePtr resptr = attribute::Resource::create();
  smtkTest(
    resptr->associations().empty() == true, "Resource should not have model storage by default.");

  DefinitionPtr def = resptr->createDefinition("testDef");
  auto arule = def->createLocalAssociationRule();
  def->setLocalAssociationMask(smtk::model::VERTEX);
  arule->setIsExtensible(true);
  arule->setMaxNumberOfValues(2);
  AttributePtr att = resptr->createAttribute("testAtt", "testDef");

  UUID fakeEntityId = UUID::random();
  smtkTest(
    !att->associateEntity(fakeEntityId),
    "Was able to associate a \"fake\" object with an attribute.");

  // Attempt to disassociate an entity that was never associated.
  UUID anotherFakeId = UUID::random();
  att->disassociateEntity(anotherFakeId);

  // ----
  // II. Now see how things work when the attribute resource has
  //     a valid model modelMgr pointer.
  model::Resource::Ptr modelMgr = model::Resource::create();
  resptr->associate(modelMgr);
  smtkTest(
    *resptr->associations().begin() == modelMgr,
    "Could not set attribute resource's model-resource.");

  smtk::model::Vertex v0 = modelMgr->addVertex();
  smtk::model::Vertex v1 = modelMgr->addVertex();
  att->associateEntity(v0);
  smtkTest(
    att->associatedModelEntityIds().count(v0.entity()) == 1,
    "Could not associate a vertex to an attribute.");

  att->disassociateEntity(v0);
  smtkTest(!v0.hasAttributes(resptr), "Disassociating an attribute did not notify the entity.");

  att->disassociateEntity(v1.entity());
  smtkTest(
    !v1.hasAttributes(resptr), "Disassociating a non-existent attribute appears to associate it.");

  v1.associateAttribute(att->attributeResource(), att->id());
  att->removeAllAssociations();
  smtkTest(
    att->associatedModelEntityIds().empty(),
    "Removing all attribute associations did not empty association list.");

  smtk::model::Vertex v2 = modelMgr->addVertex();
  v0.associateAttribute(att->attributeResource(), att->id());
  v1.associateAttribute(att->attributeResource(), att->id());
  smtkTest(
    v2.associateAttribute(att->attributeResource(), att->id()) == false,
    "Should not have been able to associate more than 2 entities.");

  att->removeAllAssociations();
  smtk::model::Edge e0 = modelMgr->addEdge();
  smtkTest(
    e0.associateAttribute(att->attributeResource(), att->id()) == false,
    "Should not have been able to associate entity of wrong type.");

  att->removeAllAssociations();
  att->associateEntity(v2);
  auto assocObj = att->associations()->value(0);
  smtkTest(
    assocObj->id() == v2.entity(),
    "Associated to wrong entity. Should be " << v2.entity() << " not " << assocObj->id());

  {
    auto associateOperation = smtk::attribute::Associate::create();

    resptr = attribute::Resource::create();
    associateOperation->parameters()->associate(resptr);

    modelMgr = model::Resource::create();
    smtkTest(
      associateOperation->parameters()->findResource("associate to") != nullptr,
      "Cannot access associate operation's input resource parameter.");
    associateOperation->parameters()->findResource("associate to")->setValue(modelMgr);

    auto result = associateOperation->operate();

    smtkTest(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Associate operator failed");

    smtkTest(
      *(resptr->associations().begin()) == modelMgr,
      "Could not set attribute resource's model-resource.");

    smtkTest(
      resptr->hasAssociations(),
      "Attribute Resource::hasAssociations() did not return true after association");
    auto dissociateOperation = smtk::attribute::Dissociate::create();

    dissociateOperation->parameters()->associate(resptr);

    smtkTest(
      dissociateOperation->parameters()->findResource("dissociate from") != nullptr,
      "Cannot access dissociate operation's input resource parameter.");
    dissociateOperation->parameters()->findResource("dissociate from")->setValue(modelMgr);

    smtkTest(dissociateOperation->ableToOperate() == true, "Dissociate operator cannot operate");

    result = dissociateOperation->operate();

    smtkTest(
      result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Dissociate operator failed");

    smtkTest(
      !resptr->hasAssociations(),
      "Attribute Resource::hasAssociations() did return true after disassociation");
    smtkTest(
      resptr->associations().empty() == true,
      "Could not dissociate model-resource from attribute resource.");
  }

  return 0;
}
