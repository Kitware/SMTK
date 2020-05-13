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
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

namespace
{

void setupAttributeResource(attribute::ResourcePtr& attRes)
{
}
}

int unitAssociationTest(int /*unused*/, char* /*unused*/ [])
{
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  DefinitionPtr TestDef = attRes->createDefinition("testDef");
  DefinitionPtr A = attRes->createDefinition("A");
  auto associationRule = A->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type='testDef']", true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);
  attRes->finalizeDefinitions();
  auto a = attRes->createAttribute("a", A);
  auto t = attRes->createAttribute("test", TestDef);

  // Let associate a to t
  smtkTest(a->associate(t), "Failed to associate a to test");
  smtkTest(a->associatedObjects()->numberOfValues() == 1,
    "Incorrect number of associated objects returned - should be 1");
  smtkTest(a->isObjectAssociated(t), "a did not indicate that it was associated to test");
  smtkTest(!a->associatedObjects()->removeInvalidValues(),
    "a's associations said there was an invalid value - it should have been ok");

  // Now remove t and clean up the invalid association
  attRes->removeAttribute(t);
  smtkTest(a->associatedObjects()->removeInvalidValues(),
    "a's associations said there were no invalid values - there should have been");
  smtkTest(a->associatedObjects()->numberOfValues() == 0,
    "Incorrect number of associated objects returned - should be 0");
  return 0;
}
