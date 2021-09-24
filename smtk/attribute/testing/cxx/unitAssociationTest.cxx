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

int unitAssociationTest(int /*unused*/, char* /*unused*/[])
{
  //
  // I. Let's create an attribute resource and some definitions
  attribute::ResourcePtr attRes = attribute::Resource::create();
  DefinitionPtr TestDef = attRes->createDefinition("testDef");
  DefinitionPtr TestDef1 = attRes->createDefinition("testDef1", TestDef);
  // In the case of Type A - let's allow testDef attributes to be associated with
  // this type of attribute but not those derived from testDef1
  DefinitionPtr A = attRes->createDefinition("A");
  auto associationRule = A->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type='testDef']", true);
  associationRule->setRejectsEntries(
    "smtk::attribute::Resource", "attribute[type='testDef1']", true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);
  DefinitionPtr B = attRes->createDefinition("B");
  auto associationRule1 = B->createLocalAssociationRule();
  associationRule1->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type='testDef']", true);
  associationRule1->setNumberOfRequiredValues(1);
  associationRule->setIsExtensible(true);
  B->localCategories().insertInclusion("foo");
  attRes->finalizeDefinitions();
  auto a = attRes->createAttribute("a", A);
  auto b = attRes->createAttribute("b", B);
  auto t = attRes->createAttribute("test", TestDef);
  auto t1 = attRes->createAttribute("test1", TestDef1);

  // Let's associate a to t
  smtkTest(a->associate(t), "Failed to associate a to test");
  smtkTest(
    a->associatedObjects()->numberOfValues() == 1,
    "Incorrect number of associated objects returned - should be 1");
  smtkTest(a->isObjectAssociated(t), "a did not indicate that it was associated to test");
  smtkTest(
    !a->associatedObjects()->removeInvalidValues(),
    "a's associations said there was an invalid value - it should have been ok");

  // Let's try to associate a to t1
  smtkTest(!a->associate(t1), "Succeeded in associating a to test1");

  // Now remove t and clean up the invalid association
  attRes->removeAttribute(t);
  smtkTest(
    a->associatedObjects()->removeInvalidValues(),
    "a's associations said there were no invalid values - there should have been");
  smtkTest(
    a->associatedObjects()->numberOfValues() == 0,
    "Incorrect number of associated objects returned - should be 0");

  // Let's test validity based on categories
  std::set<std::string> cats1 = { "foo" };
  std::set<std::string> cats2 = { "bar" };
  smtkTest(
    !b->isValid(),
    "b with no categories specified was considered valid but it should have been invalid");

  // with foo set, the attribute will pass its category filtering process but the fact
  // that the attribute's association fails its validity check should make it invalid
  attRes->setActiveCategories(cats1);
  attRes->setActiveCategoriesEnabled(true);
  smtkTest(
    !b->isValid(), "b with foo specified was considered valid but it should have been invalid");

  // with bar set, the attribute will fail its category filtering process so it will be
  // considered valid though its association is not
  attRes->setActiveCategories(cats2);
  attRes->setActiveCategoriesEnabled(true);
  smtkTest(
    b->isValid(), "b with bar specified was considered invalid but it should have been valid");

  return 0;
}
