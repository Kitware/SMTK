//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Links.h"
#include "smtk/common/json/jsonLinks.h"

#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <vector>

// Ignore warning about non-inlined template specializations of smtk::common::Helper<>
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#pragma warning(disable : 4506) /* no definition for inline function */
#endif

namespace
{

struct MyBase
{
  MyBase() = default;
  MyBase(const std::string& val)
    : value(val)
  {
  }
  virtual ~MyBase() = default;
  std::string value;
};

void to_json(nlohmann::json& j, const MyBase& myBase)
{
  j["value"] = myBase.value;
}

void from_json(const nlohmann::json& j, MyBase& myBase)
{
  myBase.value = j["value"].get<std::string>();
}

void UnitTest()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyBase> MyLinks;
  MyLinks links;
  smtkTest(links.empty() == true, "Should be empty.");

  // Try to insert a link (id, left, right) into our links (should succeed).
  auto inserted = links.insert(0, 3, 4, 100);
  smtkTest(inserted.second == true, "Should be able to insert a link.");
  smtkTest(links.empty() == false, "Should not be empty.");
  smtkTest(links.size() == 1, "Should have 1 link.");

  // Try to insert another link with the same id (should fail).
  inserted = links.insert(0, 2, 3, -100);
  smtkTest(inserted.second == false, "Should not be able to insert a link with the same id.");

  // Add a link that reuses one of the original values
  inserted = links.insert(1, 3, 5, 101);
  smtkTest(inserted.second == true, "Should be able to insert a link.");
  smtkTest(links.size() == 2, "Should have 2 links.");
  smtkTest(
    links.get<MyLinks::Right>().find(5)->left == 3,
    "Should be able to access the left value via the right value.");
  smtkTest(
    links.get<MyLinks::Role>().find(101)->left == 3,
    "Should be able to access the left value via the role.");

  // Access values by id
  smtkTest(links.at(1).left == 3, "Should be set to 3.");
  smtkTest(links.at<MyLinks::Left>(1) == 3, "Should be set to 3.");
  smtkTest(links.at(1).right == 5, "Should be set to 5.");
  smtkTest(links.at<MyLinks::Right>(1) == 5, "Should be set to 5.");
  smtkTest(links.at(1).role == 101, "Should be set to 101.");
  smtkTest(links.at<MyLinks::Role>(1) == 101, "Should be set to 101.");

  // Try to modify a value using an id that is not in the links (should throw).
  bool modified = true;
  try
  {
    modified = links.set<MyLinks::Right>(2, 6);
  }
  catch (std::out_of_range& /*outofrange*/)
  {
    modified = false;
  }
  smtkTest(modified == false, "Should through out_of_range exception.");

  // Modify a value using an id that is in the links (should succeed).
  modified = links.set<MyLinks::Right>(1, 6);
  smtkTest(modified == true, "Should be modified.");
  smtkTest(links.at<MyLinks::Right>(1) == 6, "Should be set to 6.");

  // Access linked values
  {
    auto idsLinkedTo3 = links.ids<MyLinks::Left>(3);
    auto valuesLinkedTo3 = links.linked_to<MyLinks::Left>(3);
    smtkTest(valuesLinkedTo3.size() == 2, "Should be 2 values linked to 3.");
    smtkTest(links.size<MyLinks::Left>(3) == 2, "Should have 2 links.");
    auto idIt = idsLinkedTo3.begin();
    auto valueIt = valuesLinkedTo3.begin();
    for (; idIt != idsLinkedTo3.end(); ++idIt, ++valueIt)
    {
      // The values in the sets should be in the same order because the right
      // values and indices are both monotonically increasing.
      smtkTest(
        *valueIt == links.at<MyLinks::Right>(*idIt),
        "Values accessed by Id should equal values accessed by left value type.");
    }
    std::vector<short> linkedValues(valuesLinkedTo3.begin(), valuesLinkedTo3.end());
    smtkTest(linkedValues[0] == 4, "First linked value should be 4.");
    smtkTest(linkedValues[1] == 6, "Second linked value should be 6.");
  }

  // Add another link
  links.insert(7, 4, 5, 100);
  smtkTest(links.size() == 3, "Should have 3 links.");
  smtkTest(links.size<MyLinks::Role>(100) == 2, "Should have 2 links.");

  smtkTest(links.contains<MyLinks::Left>(4) == true, "Should have a left value of 4.");
  smtkTest(links.contains<MyLinks::Right>(1) == false, "Should not have a right value of 1.");
  smtkTest(links.contains(7) == true, "Should  have an id value of 7.");

  // Erase all links associated with a "right" value of 5
  bool erased = links.erase_all<MyLinks::Right>(5);
  smtkTest(erased && links.size() == 2, "Should have 2 links.");

  links.insert(MyBase("base_class_value"), 10, std::move(4), std::move(5), 102);
  smtkTest(links.at(10).value == "base_class_value", "Should be able to access base class");
}

void UnitTestLinksEraseAll()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyBase> MyLinks;

  // Verify erase_all() on empty Links should return false.
  {
    MyLinks links;

    smtkTest(links.erase_all<MyLinks::Right>(0) == false, "");
  }

  // When Links has one element, erase_all() for a key present in Links should
  // return true.
  {
    MyLinks links;
    links.insert(MyBase("myBase1"), 0, 0, 0, -100);
    smtkTest(links.erase_all<MyLinks::Right>(0) == true, "");
    smtkTest(links.empty(), "");
  }

  // When Links has one element, erase_all() for a key not present in Links
  // should return false.
  {
    MyLinks links;
    links.insert(MyBase("myBase1"), 1, 1, 1, -100);
    smtkTest(links.erase_all<MyLinks::Right>(0) == false, "");
    smtkTest(links.size() == 1, "");
  }

  // When Links has more than one element, erase_all() for a key not present in
  // Links should return false.
  {
    MyLinks links;
    links.insert(MyBase("myBase1"), 0, 0, 0, -100);
    links.insert(MyBase("myBase2"), 1, 1, 1, -100);
    smtkTest(links.erase_all<MyLinks::Right>(2) == false, "");
    smtkTest(links.size() == 2, "");
  }

  // When Links has more than one element, erase_all() for a key present in
  // Links should return true.
  {
    MyLinks links;
    links.insert(MyBase("myBase1"), 0, 0, 0, -100);
    links.insert(MyBase("myBase2"), 1, 1, 1, -100);
    smtkTest(links.erase_all<MyLinks::Right>(1) == true, "");
    smtkTest(links.size() == 1, "");
  }

  // When Links has more than one element, erase_all() for a key with multiple
  // entries present in Links should return true.
  {
    MyLinks links;
    links.insert(MyBase("myBase1"), 0, 0, 0, -100);
    links.insert(MyBase("myBase2"), 1, 1, 1, -100);
    links.insert(MyBase("myBase2"), 1, 1, 1, -100);
    smtkTest(links.erase_all<MyLinks::Right>(1) == true, "");
    smtkTest(links.size() == 1, "");
  }
}

struct MyMoveOnlyBase
{
  MyMoveOnlyBase() = default;
  MyMoveOnlyBase(const std::string& val)
    : value(val)
  {
  }
  virtual ~MyMoveOnlyBase() = default;
  std::string value;
};
} // namespace

namespace nlohmann
{
template<>
struct adl_serializer<MyMoveOnlyBase>
{
  static MyMoveOnlyBase from_json(const json& j) { return { j.get<std::string>() }; }

  static void to_json(json& j, const MyMoveOnlyBase& t) { j = t.value; }
};
} // namespace nlohmann

namespace
{
void MoveOnlyTest()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyMoveOnlyBase> MyLinks;
  MyLinks links;
  smtkTest(links.empty() == true, "Should be empty.");

  // Try to insert a link (id, left, right) into our links (should succeed).
  auto inserted = links.insert(MyMoveOnlyBase("m"), 0, 3, 4, 100);
  smtkTest(inserted.second == true, "Should be able to insert a link.");
  smtkTest(links.empty() == false, "Should not be empty.");
  smtkTest(links.size() == 1, "Should have 1 link.");

  // Try to insert another link with the same id (should fail).
  inserted = links.insert(MyMoveOnlyBase("m"), 0, 2, 3, -100);
  smtkTest(inserted.second == false, "Should not be able to insert a link with the same id.");

  // Add a link that reuses one of the original values
  inserted = links.insert(MyMoveOnlyBase("m"), 1, 3, 5, 101);
  smtkTest(inserted.second == true, "Should be able to insert a link.");
  smtkTest(links.size() == 2, "Should have 2 links.");
  smtkTest(
    links.get<MyLinks::Right>().find(5)->left == 3,
    "Should be able to access the left value via the right value.");
  smtkTest(
    links.get<MyLinks::Role>().find(101)->left == 3,
    "Should be able to access the left value via the role.");

  // Access values by id
  smtkTest(links.at(1).left == 3, "Should be set to 3.");
  smtkTest(links.at<MyLinks::Left>(1) == 3, "Should be set to 3.");
  smtkTest(links.at(1).right == 5, "Should be set to 5.");
  smtkTest(links.at<MyLinks::Right>(1) == 5, "Should be set to 5.");
  smtkTest(links.at(1).role == 101, "Should be set to \"bar\".");
  smtkTest(links.at<MyLinks::Role>(1) == 101, "Should be set to \"bar\".");

  // Try to modify a value using an id that is not in the links (should throw).
  bool modified = true;
  try
  {
    modified = links.set<MyLinks::Right>(2, 6);
  }
  catch (std::out_of_range& /*outofrange*/)
  {
    modified = false;
  }
  smtkTest(modified == false, "Should through out_of_range exception.");

  // Modify a value using an id that is in the links (should succeed).
  modified = links.set<MyLinks::Right>(1, 6);
  smtkTest(modified == true, "Should be modified.");
  smtkTest(links.at<MyLinks::Right>(1) == 6, "Should be set to 6.");

  // Access linked values
  {
    auto idsLinkedTo3 = links.ids<MyLinks::Left>(3);
    auto valuesLinkedTo3 = links.linked_to<MyLinks::Left>(3);
    smtkTest(valuesLinkedTo3.size() == 2, "Should be 2 values linked to 3.");
    smtkTest(links.size<MyLinks::Left>(3) == 2, "Should have 2 links.");
    auto idIt = idsLinkedTo3.begin();
    auto valueIt = valuesLinkedTo3.begin();
    for (; idIt != idsLinkedTo3.end(); ++idIt, ++valueIt)
    {
      // The values in the sets should be in the same order because the right
      // values and indices are both monotonically increasing.
      smtkTest(
        *valueIt == links.at<MyLinks::Right>(*idIt),
        "Values accessed by Id should equal values accessed by left value type.");
    }
    std::vector<short> linkedValues(valuesLinkedTo3.begin(), valuesLinkedTo3.end());
    smtkTest(linkedValues[0] == 4, "First linked value should be 4.");
    smtkTest(linkedValues[1] == 6, "Second linked value should be 6.");
  }

  // Add another link
  links.insert(MyMoveOnlyBase("m"), 7, 4, 5, 100);
  smtkTest(links.size() == 3, "Should have 3 links.");
  smtkTest(links.size<MyLinks::Role>(100) == 2, "Should have 2 links.");

  smtkTest(links.contains<MyLinks::Left>(4) == true, "Should have a left value of 4.");
  smtkTest(links.contains<MyLinks::Right>(1) == false, "Should not have a right value of 1.");
  smtkTest(links.contains(7) == true, "Should  have an id value of 7.");

  // Erase all links associated with a "right" value of 5
  bool erased = links.erase_all<MyLinks::Right>(5);
  smtkTest(erased && links.size() == 2, "Should have 2 links.");

  links.insert(MyMoveOnlyBase("base_class_value"), 10, std::move(4), std::move(5), 102);
  smtkTest(links.at(10).value == "base_class_value", "Should be able to access base class");
}

void JsonTest()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyBase> MyLinks;
  MyLinks links;
  links.insert(MyBase("base_value"), 0, 3, 4, 100);
  links.insert(7, 4, 5, 100);
  links.insert(MyBase("base_class_value"), 10, std::move(4), std::move(5), 102);

  nlohmann::json j = links;

  MyLinks newLinks = j;

  smtkTest(links.size() == newLinks.size(), "Number of links should be equal.");

  for (const auto& link : links)
  {
    auto id = link.id;
    smtkTest(links.at(id).id == newLinks.at(id).id, "Link ids should be equal.");
    smtkTest(links.at(id).left == newLinks.at(id).left, "Link lefts should be equal.");
    smtkTest(links.at(id).right == newLinks.at(id).right, "Link rights should be equal.");
    smtkTest(links.at(id).role == newLinks.at(id).role, "Link roles should be equal.");
    smtkTest(links.at(id).value == newLinks.at(id).value, "Link base values should be equal.");
  }
}

void SubsetJsonTest()
{
  using UUID = smtk::common::UUID;
  typedef smtk::common::Links<UUID, UUID, UUID, int, MyBase> LocalLinks;
  LocalLinks links;
  // Create some IDs and populate links.
  auto aa = UUID::random();
  auto bb = UUID::random();
  auto cc = UUID::random();
  auto dd = UUID::random();
  links.insert(UUID::random(), aa, bb, 100);
  links.insert(UUID::random(), aa, cc, 100);
  links.insert(UUID::random(), bb, aa, 100);
  links.insert(UUID::random(), bb, dd, 100);
  links.insert(UUID::random(), cc, bb, 100);
  links.insert(UUID::random(), cc, aa, 100);
  links.insert(UUID::random(), cc, dd, 100);

  smtkTest(smtk::common::Helper<>::instance() == nullptr, "Should not have a helper yet.");

  // Serialize the subset of links with aa or bb on the left-hand side:
  auto* helper = smtk::common::Helper<>::activate();
  smtkTest(!!helper, "Should have a helper now.");
  helper->requiredIds().insert(aa);
  helper->requiredIds().insert(bb);
  helper->setLeftPlaceholderId(aa);
  helper->setRightPlaceholderId(aa);
  nlohmann::json j = links;
  smtk::common::Helper<>::deactivate();
  smtkTest(!smtk::common::Helper<>::instance(), "Should not have a helper any longer.");

  // Deserialize links, replacing the placeholder (previously aa) with dd.
  helper = smtk::common::Helper<>::activate();
  smtkTest(!!helper, "Should have a helper again.");
  helper->setLeftPlaceholderId(dd);
  helper->setRightPlaceholderId(dd);
  LocalLinks newLinks = j.get<LocalLinks>();
  smtk::common::Helper<>::deactivate();

  smtkTest(links.size() == 7, "Should start with 7 links.");
  smtkTest(newLinks.size() == 4, "Should subset the 4 links with aa/bb on left.");

  for (const auto& link : newLinks)
  {
    auto id = link.id;
    smtkTest(links.at(id).id == newLinks.at(id).id, "Link ids should be equal.");
    if (links.at(id).left != aa)
    {
      smtkTest(links.at(id).left == newLinks.at(id).left, "Link lefts should be equal.");
    }
    else
    {
      smtkTest(newLinks.at(id).left == dd, "Link lefts should be replaced.");
    }
    if (links.at(id).right != aa)
    {
      smtkTest(links.at(id).right == newLinks.at(id).right, "Link rights should be equal.");
    }
    else
    {
      smtkTest(newLinks.at(id).right == dd, "Link rights should be replaced.");
    }
    smtkTest(links.at(id).role == newLinks.at(id).role, "Link roles should be equal.");
    smtkTest(links.at(id).value == newLinks.at(id).value, "Link roles should be equal.");
  }
}

void MoveOnlyJsonTest()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyMoveOnlyBase> MyLinks;
  MyLinks links;
  links.insert(MyMoveOnlyBase("base_value"), 0, 3, 4, 100);
  links.insert(MyMoveOnlyBase("another_value"), 7, 4, 5, 100);
  links.insert(MyMoveOnlyBase("base_class_value"), 10, std::move(4), std::move(5), 102);

  nlohmann::json j = links;

  MyLinks newLinks = j;

  smtkTest(links.size() == newLinks.size(), "Number of links should be equal.");

  for (const auto& link : links)
  {
    auto id = link.id;
    smtkTest(links.at(id).id == newLinks.at(id).id, "Link ids should be equal.");
    smtkTest(links.at(id).left == newLinks.at(id).left, "Link lefts should be equal.");
    smtkTest(links.at(id).right == newLinks.at(id).right, "Link rights should be equal.");
    smtkTest(links.at(id).role == newLinks.at(id).role, "Link roles should be equal.");
    smtkTest(links.at(id).value == newLinks.at(id).value, "Link base values should be equal.");
  }
}

void RecursionTest()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyBase> MyLinks;
  typedef smtk::common::Links<int, std::size_t, short, int, MyLinks> MyMetaLinks;

  MyMetaLinks metaLinks;
  {
    metaLinks.insert(0, 1, 2, 100);
    auto& links = metaLinks.value(0);
    links.insert(MyBase("test"), 0, 3, 4, 101);
    links.insert(MyBase("test2"), 1, 4, 5, 102);

    metaLinks.insert(1, 7, 8, 200);
    auto& links2 = metaLinks.value(1);
    links2.insert(MyBase("test3"), 8, 6, 1, 201);
    links2.insert(MyBase("test4"), 11, 3, 8, 203);
    links2.insert(MyBase("test5"), 13, 2, 18, 303);
  }

  smtkTest(metaLinks.at(0).size() == 2, "first set of links should have 2 links.");
  smtkTest(metaLinks.at(1).size() == 3, "second set of links should have 3 links.");
}

void MoveOnlyRecursionTest()
{
  typedef smtk::common::Links<int, std::size_t, short, int, MyMoveOnlyBase> MyLinks;
  typedef smtk::common::Links<int, std::size_t, short, int, MyLinks> MyMetaLinks;

  MyMetaLinks metaLinks;
  {
    metaLinks.insert(0, 1, 2, 100);
    auto& links = metaLinks.value(0);
    links.insert(MyMoveOnlyBase("test"), 0, 3, 4, 101);
    links.insert(MyMoveOnlyBase("test2"), 1, 4, 5, 102);

    metaLinks.insert(1, 7, 8, 200);
    auto& links2 = metaLinks.value(1);
    links2.insert(MyMoveOnlyBase("test3"), 8, 6, 1, 201);
    links2.insert(MyMoveOnlyBase("test4"), 11, 3, 8, 203);
    links2.insert(MyMoveOnlyBase("test5"), 13, 2, 18, 303);
  }

  smtkTest(metaLinks.at(0).size() == 2, "first set of links should have 2 links.");
  smtkTest(metaLinks.at(1).size() == 3, "second set of links should have 3 links.");
}
} // namespace

int UnitTestLinks(int /*unused*/, char** const /*unused*/)
{
  UnitTest();
  UnitTestLinksEraseAll();
  MoveOnlyTest();
  JsonTest();
  MoveOnlyJsonTest();
  SubsetJsonTest();
  RecursionTest();
  MoveOnlyRecursionTest();

  return 0;
}
