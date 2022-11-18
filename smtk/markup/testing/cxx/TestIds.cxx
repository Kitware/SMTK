//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/Component.h"
#include "smtk/markup/Group.h"
#include "smtk/markup/IndirectAssignedIds.h"
#include "smtk/markup/Label.h"
#include "smtk/markup/Resource.h"
#include "smtk/markup/SequentialAssignedIds.h"

#include "smtk/string/Token.h"

#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

using namespace smtk::markup;

namespace
{

void printAssignments(
  const std::set<std::shared_ptr<AssignedIds>>& assignments,
  const std::string& summary)
{
  std::cout << summary << "\n";
  for (const auto& assignment : assignments)
  {
    std::cout << "  " << assignment.get() << " " << static_cast<int>(assignment->nature()) << " ["
              << assignment->range()[0] << ", " << assignment->range()[1] << "["
              << "\n";
  }
}

void testSequentialAssignedIdsIterator()
{
  using namespace smtk::string::literals;

  std::cout << "Test creation of SequentialAssignedIds Foo.\n";
  auto bad = SequentialAssignedIds::Iterator<SequentialAssignedIds::Forwardness::Forward>::Invalid;
  auto dummyDomain = std::make_shared<IdSpace>("dummy"_token);
  auto foo =
    std::make_shared<SequentialAssignedIds>(dummyDomain, IdNature::Primary, 10, 15, nullptr);
  test(foo->size() == 5, "Wrong size for SequentialAssignedIds.");
  test(!foo->empty(), "SequentialAssignedIds was empty.");
  test(foo->begin() + 5 == foo->end(), "Iterator addition failed.");
  test(foo->rbegin() + 5 == foo->rend(), "Iterator reverse-addition failed.");
  test(*foo->begin() == 10, "Iterator dereference failed.");
  test(*foo->end() == bad, "End iterator should be invalid.");
  test(foo->begin().index() == 0, "Bad iterator index 0.");
  test((foo->begin() + 2).index() == 2, "Bad iterator index 2.");
  test(*(foo->begin() + 2) == 12, "Bad iterator value 2.");
  test(foo->begin() != foo->end(), "Equal bookend iterators for non-empty range.");
  test(foo->contains(10, 13) == 3, "Bad range-count.");

  std::cout << "Test iterating over Foo\n";
  auto it = foo->begin();
  std::cout << "  UpDn (fwd)  =  " << *it;
  test(*(++it) == 10, "Bad pre-increment.");
  std::cout << " " << *it;
  test(*it == 11, "Failed to pre-increment.");
  test(*(it++) == 12, "Failed to post-increment.");
  std::cout << " " << *it;
  test(*it == 12, "Bad post-increment state.");
  test(*(it += 2) == 14, "Bad plus-equal increment.");
  std::cout << " " << *it;
  test(*it == 14, "Bad post-plus-equal-increment state.");
  auto it2 = it;
  test(it2++ == foo->end(), "Bad beyond-range increment.");
  test(it2++ == foo->end(), "Increment of end iterator succeeded.");
  test(*(--it) == 14, "Bad pre-decrement.");
  std::cout << " " << *it;
  test(*it == 13, "Failed to pre-decrement.");
  test(*(it--) == 12, "Failed to post-decrement.");
  std::cout << " " << *it;
  test(*it == 12, "Bad post-decrement state.");
  test(*(it -= 2) == 10, "Bad minus-equal decrement.");
  std::cout << " " << *it;
  test(*it == 10, "Bad post-minus-equal-decrement state.");
  std::cout << "\n";
  std::cout << "  Up   (fwd)  = ";
  smtk::markup::IdType expectedValue = 10;
  for (const auto& id : *foo)
  {
    std::cout << " " << id;
    test(expectedValue == id, "Bad forward increment iteration.");
    ++expectedValue;
  }
  std::cout << "\n";
  smtk::markup::IdType expectedIndex = 5;
  std::cout << "  Dn   (fwd)  = ";
  for (it = foo->begin() + foo->maxId(); it != foo->end(); --it)
  {
    std::cout << " " << *it << " (" << it.index() << ")";
    --expectedValue;
    --expectedIndex;
    test(expectedValue == *it, "Bad forward decrement iteration.");
    test(expectedIndex == it.index(), "Bad forward decrement index.");
  }
  std::cout << "\n";

  auto ri = foo->rbegin();
  std::cout << "  DnUp (rev)  =  " << *ri;
  test(*(++ri) == 14, "Bad reverse pre-increment.");
  std::cout << " " << *ri;
  test(*ri == 13, "Failed to reverse pre-increment.");
  test(*(ri++) == 12, "Failed to reverse post-increment.");
  std::cout << " " << *ri;
  test(*ri == 12, "Bad reverse post-increment state.");
  test(*(ri += 2) == 10, "Bad reverse plus-equal increment.");
  std::cout << " " << *ri;
  test(*ri == 10, "Bad reverse post-plus-equal-increment state.");
  auto ri2 = ri;
  test(ri2++ == foo->rend(), "Bad reverse beyond-range increment.");
  test(ri2++ == foo->rend(), "Increment of reverse end iterator succeeded.");
  test(*(--ri) == 10, "Bad pre-decrement.");
  std::cout << " " << *ri;
  test(*ri == 11, "Failed to pre-decrement.");
  test(*(ri--) == 12, "Failed to post-decrement.");
  std::cout << " " << *ri;
  test(*ri == 12, "Bad post-decrement state.");
  test(*(ri -= 2) == 14, "Bad minus-equal decrement.");
  std::cout << " " << *ri;
  test(*ri == 14, "Bad post-minus-equal-decrement state.");
  std::cout << "\n";
  std::cout << "  Dn   (rev)  = ";
  expectedValue = 14;
  for (ri = foo->rbegin(); ri != foo->rend(); ++ri)
  {
    std::cout << " " << *ri;
    test(expectedValue == *ri, "Bad reverse increment iteration.");
    --expectedValue;
  }
  std::cout << "\n";
  std::cout << "  Up   (rev)  = ";
  for (ri = foo->rbegin() + foo->maxId(); ri != foo->rend(); --ri)
  {
    std::cout << " " << *ri << " (" << ri.index() << ")";
    ++expectedValue;
    test(expectedValue == *ri, "Bad reverse decrement iteration.");
    test(expectedIndex == ri.index(), "Bad reverse decrement index.");
    ++expectedIndex;
  }
  std::cout << "\n";

  std::cout << "Test creation of SequentialAssignedIds Bar.\n";
  auto bar =
    std::make_shared<SequentialAssignedIds>(dummyDomain, IdNature::Primary, 10, 10, nullptr);
  // NOLINTNEXTLINE(readability-container-size-empty)
  test(bar->size() == 0, "Wrong size for empty SequentialAssignedIds.");
  test(bar->empty(), "SequentialAssignedIds was not empty.");
  test(*bar->begin() == bad, "Valid begin iterator for empty range.");
  test(bar->begin() == bar->end(), "Empty range but unequal bookend iterators.");
}

void testIndirectAssignedIdsIterator()
{
  using namespace smtk::string::literals;

  std::cout << "Test creation of IndirectAssignedIds Foo.\n";
  auto bad = IndirectAssignedIds::Iterator<IndirectAssignedIds::Forwardness::Forward>::Invalid;
  auto dummyDomain = std::make_shared<IdSpace>("dummy"_token);
  vtkNew<vtkIdTypeArray> idArray;
  std::array<vtkIdType, 7> values{ { 10, 19, 10, 16, 12, 15, 12 } };
  std::array<vtkIdType, 5> uniqueOrderedValues{ { 10, 12, 15, 16, 19 } };
  std::array<vtkIdType, 5> expectedIndices{ { 0, 4, 5, 3, 1 } };
  idArray->SetArray(values.data(), 7, /*save*/ 1);
  auto foo = std::make_shared<IndirectAssignedIds>(dummyDomain, IdNature::Primary, 10, 20, nullptr);
  foo->setIdArray(idArray);
  test(foo->size() == 5, "Wrong size for IndirectAssignedIds.");
  test(!foo->empty(), "IndirectAssignedIds was empty.");
  test(foo->begin() + 5 == foo->end(), "Iterator addition failed.");
  test(foo->rbegin() + 5 == foo->rend(), "Iterator reverse-addition failed.");
  test(*foo->begin() == 10, "Iterator dereference failed.");
  test(*foo->end() == bad, "End iterator should be invalid.");
  test(foo->begin().index() == 0, "Bad iterator index 0.");
  test((foo->begin() + 2).index() == 5, "Bad iterator index 2.");
  test(*(foo->begin() + 2) == 15, "Bad iterator value 2.");
  test(foo->begin() != foo->end(), "Equal bookend iterators for non-empty range.");
  test(foo->contains(10, 13) == 2, "Bad range-count.");

  std::cout << "Test iterating over Foo\n";
  auto it = foo->begin();
  std::cout << "  UpDn (fwd)  =  " << *it;
  test(*(++it) == 10, "Bad pre-increment.");
  std::cout << " " << *it;
  test(*it == 12, "Failed to pre-increment.");
  test(*(it++) == 15, "Failed to post-increment.");
  std::cout << " " << *it;
  test(*it == 15, "Bad post-increment state.");
  test(*(it += 2) == 19, "Bad plus-equal increment.");
  std::cout << " " << *it;
  test(*it == 19, "Bad post-plus-equal-increment state.");
  auto it2 = it;
  test(it2++ == foo->end(), "Bad beyond-range increment.");
  test(it2++ == foo->end(), "Increment of end iterator succeeded.");
  test(*(--it) == 19, "Bad pre-decrement.");
  std::cout << " " << *it;
  test(*it == 16, "Failed to pre-decrement.");
  test(*(it--) == 15, "Failed to post-decrement.");
  std::cout << " " << *it;
  test(*it == 15, "Bad post-decrement state.");
  test(*(it -= 2) == 10, "Bad minus-equal decrement.");
  std::cout << " " << *it;
  test(*it == 10, "Bad post-minus-equal-decrement state.");
  std::cout << "\n";
  std::cout << "  Up   (fwd)  = ";
  int ii = 0;
  for (const auto& id : *foo)
  {
    std::cout << " " << id;
    test(uniqueOrderedValues[ii] == static_cast<vtkIdType>(id), "Bad forward increment iteration.");
    ++ii;
  }
  std::cout << "\n";
  std::cout << "  Dn   (fwd)  = ";
  ii = 5;
  for (it = foo->begin() + foo->maxId(); it != foo->end(); --it)
  {
    std::cout << " " << *it << " (" << it.index() << ")";
    --ii;
    test(
      uniqueOrderedValues[ii] == static_cast<vtkIdType>(*it), "Bad forward decrement iteration.");
    test(expectedIndices[ii] == it.index(), "Bad forward decrement index.");
  }
  std::cout << "\n";

  auto ri = foo->rbegin();
  std::cout << "  DnUp (rev)  =  " << *ri;
  test(*(++ri) == 19, "Bad reverse pre-increment.");
  std::cout << " " << *ri;
  test(*ri == 16, "Failed to reverse pre-increment.");
  test(*(ri++) == 15, "Failed to reverse post-increment.");
  std::cout << " " << *ri;
  test(*ri == 15, "Bad reverse post-increment state.");
  test(*(ri += 2) == 10, "Bad reverse plus-equal increment.");
  std::cout << " " << *ri;
  test(*ri == 10, "Bad reverse post-plus-equal-increment state.");
  auto ri2 = ri;
  test(ri2++ == foo->rend(), "Bad reverse beyond-range increment.");
  test(ri2++ == foo->rend(), "Increment of reverse end iterator succeeded.");
  test(*(--ri) == 10, "Bad pre-decrement.");
  std::cout << " " << *ri;
  test(*ri == 12, "Failed to pre-decrement.");
  test(*(ri--) == 15, "Failed to post-decrement.");
  std::cout << " " << *ri;
  test(*ri == 15, "Bad post-decrement state.");
  test(*(ri -= 2) == 19, "Bad minus-equal decrement.");
  std::cout << " " << *ri;
  test(*ri == 19, "Bad post-minus-equal-decrement state.");
  std::cout << "\n";
  std::cout << "  Dn   (rev)  = ";
  ii = 4;
  for (ri = foo->rbegin(); ri != foo->rend(); ++ri)
  {
    std::cout << " " << *ri;
    test(
      uniqueOrderedValues[ii] == static_cast<vtkIdType>(*ri), "Bad reverse increment iteration.");
    --ii;
  }
  std::cout << "\n";
  std::cout << "  Up   (rev)  = ";
  ii = 0;
  for (ri = foo->rbegin() + foo->maxId(); ri != foo->rend(); --ri)
  {
    std::cout << " " << *ri << " (" << ri.index() << ")";
    test(
      uniqueOrderedValues[ii] == static_cast<vtkIdType>(*ri), "Bad reverse decrement iteration.");
    test(expectedIndices[ii] == ri.index(), "Bad reverse decrement index.");
    ++ii;
  }
  std::cout << "\n";

  std::cout << "Test creation of IndirectAssignedIds Bar.\n";
  auto bar = std::make_shared<IndirectAssignedIds>(dummyDomain, IdNature::Primary, 10, 10, nullptr);
  // NOLINTNEXTLINE(readability-container-size-empty)
  test(bar->size() == 0, "Wrong size for empty IndirectAssignedIds.");
  test(bar->empty(), "IndirectAssignedIds was not empty.");
  test(*bar->begin() == bad, "Valid begin iterator for empty range.");
  test(bar->begin() == bar->end(), "Empty range but unequal bookend iterators.");
}

} // anonymous namespace

int TestIds(int, char** const)
{
  using namespace smtk::string::literals; // for ""_token

  testSequentialAssignedIdsIterator();
  testIndirectAssignedIdsIterator();

  // Create an IdSpace:
  auto pointIds = std::make_shared<IdSpace>("points"_token);

  // Allocate some IDs (of varying IdNature) in the space:
  IdSpace::IdType nn;
  auto primary1 = pointIds->requestRange(IdNature::Primary, 10);
  auto primary2 = pointIds->requestRange(IdNature::Primary, 10, 11);
  auto primary3 = pointIds->requestRange(IdNature::Primary, 10);
  auto reference1 = pointIds->requestRange(IdNature::Referential, 10, 5);
  auto reference2 = pointIds->requestRange(IdNature::Referential, 20, 5);

  nn = pointIds->numberOfIdsInRangeOfNature(1, 31, IdNature::Primary);
  std::cout << nn << " primary IDs in [1,31[\n";
  test(nn == 30, "Expected 30 primary IDs.");

  nn = pointIds->numberOfIdsInRangeOfNature(1, 31, IdNature::Referential);
  std::cout << nn << " referential IDs in [1,31[\n";
  test(nn == 20, "Expected 20 referential IDs.");

  auto reference3 = pointIds->requestRange(IdNature::Referential, 20, 20);  // should fail
  auto nonexclsv1 = pointIds->requestRange(IdNature::NonExclusive, 20, 20); // should fail

  test(!reference3, "Expected reference beyond end of assignments to fail.");
  test(!nonexclsv1, "Expected non-exclusive allocation overlapping primary assignments to fail.");

  auto nonexclsv2 = pointIds->requestRange(IdNature::NonExclusive, 10, 40); // should succeed
  auto nonexclsv3 = pointIds->requestRange(IdNature::NonExclusive, 5, 45);  // should succeed
                                                                            //
  nn = pointIds->numberOfIdsInRangeOfNature(1, 51, IdNature::NonExclusive);
  std::cout << nn << " non-exclusive IDs in [1,51[\n";
  test(nn == 10, "Expected 10 non-exclusive IDs.");

  nn = pointIds->numberOfIdsInRangeOfNature(1, 31, IdNature::Unassigned);
  std::cout << nn << " unassigned IDs in [1,31[\n";
  test(nn == 0, "Expected 0 unassigned IDs.");

  std::cout << "IdSpace <" << pointIds->name().data() << ">"
            << " range [" << pointIds->range()[0] << ", " << pointIds->range()[1] << "[\n";

  test(!reference3, "Expected referential request not fully covered by primary IDs to fail.");
  test(!nonexclsv1, "Expected non-exclusive request not fully covered by primary Ids to fail.");

  auto assignments = pointIds->assignedIds(10, 12);
  printAssignments(assignments, "[10, 12[");
  std::array<std::size_t, 4> counts{ 0, 0, 0, 0 };
  for (const auto& assignment : assignments)
  {
    ++counts[static_cast<int>(assignment->nature())];
  }
  test(
    counts[static_cast<int>(IdNature::Primary)] == 2,
    "Expected [10,12[ to overlap two primary assignments.");
  test(
    counts[static_cast<int>(IdNature::Referential)] == 2,
    "Expected [10,12[ to overlap two referential assignments.");
  test(
    counts[static_cast<int>(IdNature::NonExclusive)] == 0,
    "Expected [10,12[ to overlap zero non-exclusive assignments.");

  assignments = pointIds->assignedIds(1, 2);
  printAssignments(assignments, "[1, 2[");
  test(assignments.size() == 1, "Expected [1,2[ to overlap a single set of assigned IDs.");

  test(primary1->range()[0] == 1, "Expected first range to start at 1.");
  test(primary1->range()[1] == 11, "Expected first range to end at 11.");

  test(primary2->range()[0] == 11, "Expected second range to start at 11.");
  test(primary2->range()[1] == 21, "Expected second range to end at 21.");

  test(primary3->range()[0] == 21, "Expected second range to start at 21.");
  test(primary3->range()[1] == 31, "Expected second range to end at 31.");

  // Now verify that deleting assigned IDs removes them from the id-space.
  reference2 = nullptr; // This should cause the assigned IDs to be destroyed.
  nn = pointIds->numberOfIdsInRangeOfNature(1, 31, IdNature::Referential);
  std::cout << nn << " referential IDs in [1,31[ after removing reference2.\n";
  test(nn == 10, "Expected 10 referential IDs.");

  // TODO: Warn or fail if referential entries would remain in a range when
  //       removing primary or non-exclusive entries.
  return 0;
}
