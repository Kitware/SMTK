//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*!\file unitGroupItem.cxx - Unit tests for GroupItem. */

#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;

// Mocks an Attribute Resource with a single GroupItem.
//
// Instantiate the class, then modify the GroupItemDefinition via
// mp_groupItemDef and get a GroupItem to test by calling getGroupItem().
class GroupItemTest
{
public:
  GroupItemTest()
  {
    mp_attRes = Resource::create();
    mp_def = mp_attRes->createDefinition("def");
    mp_groupItemDef = GroupItemDefinition::New("testGroupItem");
    mp_groupItemDef->setIsExtensible(true);
    mp_def->addItemDefinition(mp_groupItemDef);
  }

  GroupItemPtr getGroupItem()
  {
    return mp_attRes->createAttribute(mp_def)->findGroup("testGroupItem");
  }

  ResourcePtr mp_attRes;
  DefinitionPtr mp_def;
  GroupItemDefinitionPtr mp_groupItemDef;
};

void testNumberOfGroups()
{
  const std::size_t startingNumber = 5;
  const std::size_t increasedNumber = 8;
  const std::size_t decreasedNumber = 3;

  GroupItemTest groupItemTest;
  GroupItemPtr item = groupItemTest.getGroupItem();
  item->setNumberOfGroups(startingNumber);

  smtkTest(
    item->numberOfGroups() == startingNumber,
    "Expected starting number of groups to be " << startingNumber << ". ")
    smtkTest(
      smtk::io::Logger::instance().hasErrors() == false,
      "Expected global logger to have no errors.")

      smtk::io::Logger::instance()
        .reset();

  item->setNumberOfGroups(increasedNumber);

  smtkTest(
    item->numberOfGroups() == increasedNumber,
    "Expected increased number of groups to be " << increasedNumber << ". ")
    smtkTest(
      smtk::io::Logger::instance().hasErrors() == false,
      "Expected global logger to have no errors.")

      smtk::io::Logger::instance()
        .reset();

  item->setNumberOfGroups(decreasedNumber);

  smtkTest(
    item->numberOfGroups() == decreasedNumber,
    "Expected decreased number of groups to be " << decreasedNumber << ". ")
    smtkTest(
      smtk::io::Logger::instance().hasErrors() == false,
      "Expected global logger to have no errors.")
}

int unitGroupItem(int /*argc*/, char** const /*argv*/)
{
  smtk::io::Logger::instance().reset();
  testNumberOfGroups();
  smtk::io::Logger::instance().reset();
  return 0;
}
