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
#include "smtk/attribute/Resource.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

int unitAttributeAssociation(int, char* [])
{
  // ----
  // I. First see how things work when Resource is not yet set.
  attribute::ResourcePtr resptr = attribute::Resource::create();
  attribute::Resource& res(*resptr.get());
  smtkTest(!res.refModelResource(), "Resource should not have model storage by default.");

  DefinitionPtr def = res.createDefinition("testDef");
  auto arule = def->createLocalAssociationRule();
  def->setLocalAssociationMask(smtk::model::VERTEX);
  arule->setIsExtensible(true);
  arule->setMaxNumberOfValues(2);
  AttributePtr att = res.createAttribute("testAtt", "testDef");

  UUID fakeEntityId = UUID::random();
  smtkTest(!att->associateEntity(fakeEntityId),
    "Was able to associate a \"fake\" object with an attribute.");

  // Attempt to disassociate an entity that was never associated.
  UUID anotherFakeId = UUID::random();
  att->disassociateEntity(anotherFakeId);

  // ----
  // II. Now see how things work when the attribute resource has
  //     a valid model modelMgr pointer.
  model::Resource::Ptr modelMgr = model::Resource::create();
  res.setRefModelResource(modelMgr);
  smtkTest(
    res.refModelResource() == modelMgr, "Could not set attribute resource's model-resource.");

  smtkTest(att->modelResource() == modelMgr, "Attribute's idea of model resource incorrect.");

  smtk::model::Vertex v0 = modelMgr->addVertex();
  smtk::model::Vertex v1 = modelMgr->addVertex();
  att->associateEntity(v0);
  smtkTest(att->associatedModelEntityIds().count(v0.entity()) == 1,
    "Could not associate a vertex to an attribute.");

  att->disassociateEntity(v0);
  smtkTest(!v0.hasAttributes(), "Disassociating an attribute did not notify the entity.");

  att->disassociateEntity(v1.entity());
  smtkTest(!v1.hasAttributes(), "Disassociating a non-existent attribute appears to associate it.");

  v1.associateAttribute(att->attributeResource(), att->id());
  att->removeAllAssociations();
  smtkTest(att->associatedModelEntityIds().empty(),
    "Removing all attribute associations did not empty association list.");

  smtk::model::Vertex v2 = modelMgr->addVertex();
  v0.associateAttribute(att->attributeResource(), att->id());
  v1.associateAttribute(att->attributeResource(), att->id());
  smtkTest(v2.associateAttribute(att->attributeResource(), att->id()) == false,
    "Should not have been able to associate more than 2 entities.");

  att->removeAllAssociations();
  smtk::model::Edge e0 = modelMgr->addEdge();
  smtkTest(e0.associateAttribute(att->attributeResource(), att->id()) == false,
    "Should not have been able to associate entity of wrong type.");

  // ----
  // III. Test corner cases when switch model resources on the attribute resource.
  model::Resource::Ptr auxModelResource = model::Resource::create();
  res.setRefModelResource(auxModelResource);
  smtkTest(
    res.refModelResource() == auxModelResource, "Attribute resource's modelMgr not changed.");

  return 0;
}
