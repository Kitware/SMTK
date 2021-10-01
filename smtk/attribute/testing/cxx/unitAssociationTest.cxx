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
  DefinitionPtr BaseSupplyDef = attRes->createDefinition("baseSupplyDef");
  DefinitionPtr OtherSupplyDef = attRes->createDefinition("otherSupplyDef", BaseSupplyDef);

  // In the case of Type A - let's allow baseSupplyDef attributes to be associated with
  // this type of attribute but not those derived from otherSupplyDef
  DefinitionPtr A = attRes->createDefinition("A");
  auto associationRule = A->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type = 'baseSupplyDef']", true);
  associationRule->setRejectsEntries(
    "smtk::attribute::Resource", "attribute[type='otherSupplyDef']", true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);

  // In the case of Type B - lets allow baseSupplyDef attributes to be associated with it
  // but also make B belong to category foo
  DefinitionPtr B = attRes->createDefinition("B");
  associationRule = B->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type='baseSupplyDef']", true);
  associationRule->setNumberOfRequiredValues(1);
  associationRule->setIsExtensible(true);
  B->localCategories().insertInclusion("foo");

  // In the case of Type C - lets allow baseSupplyDef attribute that contain integer property alpha
  // to be associated with it
  DefinitionPtr C = attRes->createDefinition("C");
  associationRule = C->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type='baseSupplyDef', long {'alpha'}]", true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);

  // In the case of Type D - lets allow baseSupplyDef attribute that contain integer property alpha
  // with value 4 to be associated with it
  DefinitionPtr D = attRes->createDefinition("D");
  associationRule = D->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type='baseSupplyDef', long {'alpha' = 4}]", true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);

  // In the case of Type E - lets allow baseSupplyDef attribute whose definition contains base
  // in the type name
  DefinitionPtr E = attRes->createDefinition("E");
  associationRule = E->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource", "attribute[type= /base.*/]", true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);

  // In the case of Type F - lets allow baseSupplyDef attribute that contain both an integer and long property
  DefinitionPtr F = attRes->createDefinition("F");
  associationRule = F->createLocalAssociationRule();
  associationRule->setAcceptsEntries(
    "smtk::attribute::Resource",
    "attribute[type='baseSupplyDef', long{'alpha'}, double{'beta'}]",
    true);
  associationRule->setNumberOfRequiredValues(0);
  associationRule->setIsExtensible(true);

  attRes->finalizeDefinitions();
  auto a = attRes->createAttribute("a", A);
  auto b = attRes->createAttribute("b", B);
  auto c = attRes->createAttribute("c", C);
  auto d = attRes->createAttribute("d", D);
  auto e = attRes->createAttribute("e", E);
  auto f = attRes->createAttribute("f", F);
  auto t = attRes->createAttribute("test", BaseSupplyDef);
  auto t1 = attRes->createAttribute("test1", OtherSupplyDef);

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

  attRes->setActiveCategoriesEnabled(false);
  //Lets try associating test1 to c, d, e and f - c, d, and f should fail because test 1 does not have the
  // correct property alpha (to be associated with ) set to 4 (to be associated with d) and f will fail since
  // it requires both a long and double property to be set
  smtkTest(!c->associate(t1), "Succeeded in associating c to test1 w/o having alpha property");
  smtkTest(!d->associate(t1), "Succeeded in associating d to test1 w/o having alpha property = 4");
  smtkTest(e->associate(t1), "Failed in associating e to test1");
  smtkTest(
    !f->associate(t1), "Succeeded in associating f to test1 w/o having a double and long property");

  // Lets now set property alpha to t1
  t1->properties().emplace<long>("alpha", 2);

  // Lets retest c, d, f - c should pass but d should still fail due to alpha not set to 4 and f still requires
  // a double property
  smtkTest(c->associate(t1), "Failed in associating c to test1 having alpha property");
  smtkTest(!d->associate(t1), "Succeeded in associating d to test1 w/o having alpha property = 4");
  smtkTest(!f->associate(t1), "Succeeded in associating f to test1 w/o having double property");
  // Lets set alpha to 4
  t1->properties().get<long>()["alpha"] = 4;
  smtkTest(d->associate(t1), "Failed in associating d to test1 having alpha property = 4");
  smtkTest(
    d->isValid(),
    "d with test1 (having alpha property  = 4) associated with it was considered invalid.");
  // Lets set alpha to 10
  t1->properties().get<long>()["alpha"] = 10;
  smtkTest(
    !d->isValid(),
    "d with test1 (having alpha property  = 10) associated with it was considered valid.");
  t1->properties().emplace<double>("beta", 2);
  smtkTest(f->associate(t1), "Failed in associating f to test1 having double and long properties");
  return 0;
}
