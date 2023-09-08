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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

using namespace smtk::attribute;

int main()
{
  ResourcePtr resource = smtk::attribute::Resource::create();
  DefinitionPtr def = resource->createDefinition("testDef");
  auto assocRule = def->createLocalAssociationRule("associations");
  // Lets add some item definitions
  // First create a string item def that has 2 possible discrete values
  // a-a (String) and a-b (Group).  For a-a duplicate this structure again.
  auto s_idef = def->addItemDefinition<StringItemDefinitionPtr>("a");
  auto s_idef1 = s_idef->addItemDefinition<StringItemDefinitionPtr>("a-a");
  auto g_idef1 = s_idef->addItemDefinition<GroupItemDefinitionPtr>("a-b");
  g_idef1->addItemDefinition<StringItemDefinitionPtr>("a-b-a");
  s_idef->addDiscreteValue("a-a");
  s_idef->addDiscreteValue("a-b");
  s_idef->addConditionalItem("a-a", "a-a");
  s_idef->addConditionalItem("a-b", "a-b");
  auto s_idef2 = s_idef1->addItemDefinition<StringItemDefinitionPtr>("a-a-a");
  auto g_idef2 = s_idef1->addItemDefinition<GroupItemDefinitionPtr>("a-a-b");
  s_idef1->addDiscreteValue("a-a-a");
  s_idef1->addDiscreteValue("a-a-b");
  s_idef1->addConditionalItem("a-a-a", "a-a-a");
  s_idef1->addConditionalItem("a-a-b", "a-a-b");
  g_idef2->addItemDefinition<StringItemDefinitionPtr>("a-a-b-a");

  // Now lets create a top level group item
  auto g_idef = def->addItemDefinition<GroupItemDefinitionPtr>("b");
  s_idef1 = g_idef->addItemDefinition<StringItemDefinitionPtr>("b-a");
  s_idef2 = s_idef1->addItemDefinition<StringItemDefinitionPtr>("b-a-a");
  g_idef2 = s_idef1->addItemDefinition<GroupItemDefinitionPtr>("b-a-b");
  g_idef1 = g_idef->addItemDefinition<GroupItemDefinitionPtr>("b-b");
  s_idef1->addDiscreteValue("b-a-a");
  s_idef1->addDiscreteValue("b-a-b");
  s_idef1->addConditionalItem("b-a-a", "b-a-a");
  s_idef1->addConditionalItem("b-a-b", "b-a-b");
  g_idef1->addItemDefinition<StringItemDefinitionPtr>("b-b-a");
  // an item that starts with a potentially valid index.
  g_idef->addItemDefinition<StringItemDefinitionPtr>("0 not index");
  // item with a name like an integer
  g_idef->addItemDefinition<StringItemDefinitionPtr>("3");
  // test creating a second group
  g_idef->setIsExtensible(true);

  // There was an issue with searching empty groups that caused a crash
  // Let add one to be make sure we don't regress.
  g_idef = def->addItemDefinition<GroupItemDefinitionPtr>("empty");
  g_idef->setIsExtensible(true);
  g_idef->setNumberOfRequiredGroups(0);

  // Ok now create an attribute
  AttributePtr att = resource->createAttribute("testAtt", "testDef");

  // Lets do some finds
  auto item = att->find("a");
  smtkTest((item != nullptr), "Could not find a");
  smtkTest((item->name() == "a"), "Founded \"a\" item not named a");
  smtkTest((att->findAs<StringItem>("a") != nullptr), "a was not a StringItem");

  item = att->find("b");
  smtkTest((item != nullptr), "Could not find b");
  smtkTest((item->name() == "b"), "Founded \"b\" item not named b");
  smtkTest((att->findAs<GroupItem>("b") != nullptr), "b was not a GroupItem");

  item = att->find("c");
  smtkTest((item == nullptr), "Could find c");

  item = att->find("a-a");
  smtkTest((item == nullptr), "Could find a-a when using RECURSIVE_ACTIVE");

  item = att->find("a-b");
  smtkTest((item == nullptr), "Could find a-b when using RECURSIVE_ACTIVE");

  item = att->find("a-a", RECURSIVE);
  smtkTest((item != nullptr), "Could not find a-a using RECURSIVE");
  smtkTest((item->name() == "a-a"), "Founded \"a-a\" item not named a-a");
  smtkTest((att->findAs<StringItem>("a-a", RECURSIVE) != nullptr), "a-a was not a StringItem");

  auto itemA = att->findAs<StringItem>("a");
  itemA->setValue("a-a");

  item = att->find("a-a");
  smtkTest((item != nullptr), "Could not find a-a using RECURSIVE_ACTIVE after setting a-a");
  smtkTest((item->name() == "a-a"), "Founded \"a-a\" item not named a-a after setting a-a");
  smtkTest(
    (att->findAs<StringItem>("a-a") != nullptr),
    "a-a was not a StringItem using RECURSIVE_ACTIVE after setting a-a");

  item = att->find("a-a-b-a");
  smtkTest((item == nullptr), "Could find a-a-b-a using RECURSIVE_ACTIVE after setting a-a");

  item = att->find("a-a-b-a", RECURSIVE);
  smtkTest((item != nullptr), "Could not find a-a-b-a using RECURSIVE after setting a-a");
  smtkTest((item->name() == "a-a-b-a"), "Founded \"a-a-b-a\" item not named a-a after setting a-a");
  smtkTest(
    (att->findAs<StringItem>("a-a-b-a", RECURSIVE) != nullptr),
    "a-a-b-a was not a StringItem using RECURSIVE after setting a-a");

  att->findAs<StringItem>("a-a")->setValue("a-a-b");
  item = att->find("a-a-b-a");
  smtkTest((item != nullptr), "Could not find a-a-b-a using after setting a-a and a-a-b");
  smtkTest(
    (item->name() == "a-a-b-a"),
    "Founded \"a-a-b-a\" item not named a-a-b-a after setting a-a and a-a-b");
  smtkTest(
    (att->findAs<StringItem>("a-a-b-a") != nullptr),
    "a-a-b-a was not a StringItem using after setting a-a and a-a-b");

  item = att->find("a-a", IMMEDIATE);
  smtkTest((item == nullptr), "Could find a-a when using IMMEDIATE");

  item = att->find("a-b");
  smtkTest((item == nullptr), "Could find a-b when using RECURSIVE_ACTIVE after setting a-a");

  itemA->setValue("a-b");
  item = att->find("a-b");
  smtkTest((item != nullptr), "Could not find a-b using RECURSIVE_ACTIVE after setting a-b");
  smtkTest((item->name() == "a-b"), "Founded \"a-b\" item not named a-a after setting a-b");
  smtkTest(
    (att->findAs<GroupItem>("a-b") != nullptr),
    "a-b was not a GroupItem using RECURSIVE_ACTIVE after setting a-a");

  item = att->find("a-a-a", RECURSIVE);
  smtkTest((item != nullptr), "Could not find a-a-a using RECURSIVE");
  smtkTest((item->name() == "a-a-a"), "Founded \"a-a\" item not named a-a");
  smtkTest((att->findAs<StringItem>("a-a-a", RECURSIVE) != nullptr), "a-a-a was not a StringItem");

  item = att->find("b-a");
  smtkTest((item != nullptr), "Could not find b-a");
  smtkTest((item->name() == "b-a"), "Founded \"b-a\" item not named b-a");
  smtkTest((att->findAs<StringItem>("b-a") != nullptr), "b-a was not a StringItem");

  item = att->find("b-b");
  smtkTest((item != nullptr), "Could not find b-b");
  smtkTest((item->name() == "b-b"), "Founded \"b-b\" item not named b-b");
  smtkTest((att->findAs<GroupItem>("b-b") != nullptr), "b-b was not a GroupItem");

  item = att->find("b-b-a");
  smtkTest((item != nullptr), "Could not find b-b-a");
  smtkTest((item->name() == "b-b-a"), "Founded \"b-b-a\" item not named b-b-a");
  smtkTest((att->findAs<StringItem>("b-b-a") != nullptr), "b-b-a was not a StringItem");

  // Lets test some finds on an item
  item = itemA->find("a-a", IMMEDIATE_ACTIVE);
  smtkTest((item == nullptr), "Could find a-a when using IMMEDIATE_ACTIVE after setting a-b");

  item = itemA->find("a-b", IMMEDIATE_ACTIVE);
  smtkTest((item != nullptr), "Could not find a-b when using IMMEDIATE_ACTIVE after setting a-b");

  // Lets do some path tests
  item = att->itemAtPath("a");
  smtkTest((item != nullptr), "Could not find using path: a");
  item = att->itemAtPath("/a");
  smtkTest((item != nullptr), "Could not find using path: /a");
  item = att->itemAtPath("a/a-a/a-a-b");
  smtkTest((item != nullptr), "Could not find using path: a/a-a/a-a-b");
  item = att->itemAtPath("/a/a-a/a-a-b");
  smtkTest((item != nullptr), "Could not find using path: /a/a-a/a-a-b");
  item = att->itemAtPath("a/a-a/a-a-b", "/", true);
  smtkTest((item == nullptr), "Could find using path: a/a-a/a-a-b when asking for active");
  item = att->itemAtPath("a/a-b/a-b-a", "/", true);
  smtkTest((item != nullptr), "Could not find using path: a/a-b/a-b-a when asking for active");
  item = att->itemAtPath("/associations");
  smtkTest(!!item, "Could not reference associations.");
  // for groups, test adding the index for the sub-group.
  item = att->itemAtPath("b/b-b/b-b-a");
  smtkTest((item != nullptr), "Could not find using path: b/b-b/b-b-a");
  item = att->itemAtPath("b/0/b-b/0/b-b-a");
  smtkTest((item != nullptr), "Could not find using path: b/0/b-b/0/b-b-a");
  item = att->itemAtPath("b/1/b-b");
  smtkTest((item == nullptr), "Found item using path: b/1/b-b");
  item = att->itemAtPath("b/3");
  smtkTest((item == nullptr), "Found item using path, should use index: b/3");
  item = att->itemAtPath("b/0/3");
  smtkTest((item != nullptr), "Could not find using path: b/0/3");
  // with and without index
  item = att->itemAtPath("b/0 not index");
  smtkTest((item != nullptr), "Could not find using path: b/0 not index");
  item = att->itemAtPath("b/0/0 not index");
  smtkTest((item != nullptr), "Could not find using path: b/0/0 not index");

  // create a second group
  auto grp2 = att->findAs<GroupItem>("b");
  smtkTest((grp2 != nullptr), "Could not find b");
  grp2->appendGroup();
  smtkTest(grp2->numberOfGroups() == 2, "Expect two groups");
  auto str1 = grp2->findAs<StringItem>(1, "b-a");
  smtkTest((str1 != nullptr), "Could not find newly added sub-group item");
  str1->setValue("b-a-b");
  // retrieve via path and check value
  auto str2 = att->itemAtPathAs<StringItem>("b/1/b-a");
  smtkTest((str2 != nullptr), "Could not find using path: b/1/b-a");
  smtkTest((str2->value() == "b-a-b"), "Item value not set")

    // Recurse through the children of a single item.
    typedef smtk::attribute::StringItem MyItemType;
  std::vector<MyItemType::Ptr> myItems;

  struct RecursiveAccumulate
  {
    RecursiveAccumulate(std::vector<MyItemType::Ptr>& myItems)
      : m_myItems(myItems)
    {
    }

    void operator()(smtk::attribute::ItemPtr visited, bool activeChildren)
    {
      if (auto myItem = std::dynamic_pointer_cast<MyItemType>(visited))
      {
        m_myItems.push_back(myItem);
      }
      visited->visitChildren(*this, activeChildren);
    }

    std::vector<MyItemType::Ptr>& m_myItems;
  };

  item = att->find("a");
  RecursiveAccumulate recursiveAccumulate(myItems);
  item->visitChildren(recursiveAccumulate, true);
  smtkTest(myItems.size() == 1, "Failed to recurse through a single item's children");
  smtkTest((myItems[0]->name() == "a-b-a"), "Failed to find child item");
}
