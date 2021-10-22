//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/Manager.h"
#include "smtk/string/json/jsonManager.h"

#include "smtk/common/testing/cxx/helpers.h"

int TestManager(int, char*[])
{
  using namespace smtk::string;

  std::size_t ocount = 0;
  std::function<void(Manager::Event, Hash, const std::string&, Hash)> observer =
    [&ocount](Manager::Event event, Hash h, const std::string& s, Hash set) {
      ++ocount;
      std::cout << "Observed event " << event << " hash " << h << " set " << set << " \"" << s
                << "\"\n";
    };

  auto manager = Manager::create();
  auto observerKey =
    manager->observers().insert(observer, /*priority*/ 0, /*initialize*/ false, "Test observer");

  std::size_t vcount = 0;
  std::function<smtk::common::Visit(Hash entry)> visitor = [&vcount](Hash entry) {
    ++vcount;
    std::cout << "  Hash " << std::hex << entry << std::dec << "\n";
    return smtk::common::Visit::Continue;
  };

  std::array<std::string, 5> s1 = { "foo", "fi", "fum", "bar", "baz" };
  std::array<std::string, 5> s2 = { "scudge", "freen", "frell", "norzum", "freen" };

  std::array<Hash, 12> hashes;
  hashes[0] = manager->manage("fooset");
  hashes[1] = manager->manage("unixy");
  std::cout << "fooset " << hashes[0] << "\n"
            << "unixy " << hashes[1] << "\n";
  test(hashes[0] != hashes[1], "Hashes of non-equal strings should always be non-equal.");

  for (std::size_t ii = 0; ii < s1.size(); ++ii)
  {
    hashes[2 + ii] = manager->manage(s1[ii]);
    test(hashes[2 + ii] != Manager::Invalid, "Zero hashes are not OK.");
    hashes[7 + ii] = manager->manage(s2[ii]);
    test(hashes[7 + ii] != Manager::Invalid, "Zero hashes are not OK.");
    auto hs1 = manager->insert("fooset", hashes[2 + ii]);
    std::cout << "Inserted into set " << hs1 << "\n";
    test(hs1 == hashes[0], "Expected hash equivalence (hs1).");
    bool hs2 = manager->insert(hashes[1], hashes[7 + ii]);
    test(ii == 4 ? !hs2 : hs2, "Expected valid insertion except for final duplicate (hs2).");
  }

  std::cout << "\nobserved " << ocount << " events\n\n";
  test(ocount == 21, "Expected 21 observer firings (11 for fooset, 10 for unixy).");
  ocount = 0;

  // Test to_json.
  nlohmann::json j = manager;
  std::cout << "\n" << j.dump(2) << "\n\n";

  // Test set-contains.
  for (std::size_t ii = 0; ii < s1.size(); ++ii)
  {
    test(manager->contains("fooset", hashes[2 + ii]), "Expected fooset to contain " + s1[ii]);
    test(!manager->contains(hashes[0], hashes[7 + ii]), "Expected fooset to not contain " + s2[ii]);

    test(!manager->contains("unixy", hashes[2 + ii]), "Expected fooset to not contain " + s1[ii]);
    test(manager->contains(hashes[1], hashes[7 + ii]), "Expected fooset to contain " + s2[ii]);
  }

  // Test visitation of entire manager.
  smtk::common::Visit didHalt;
  vcount = 0;
  didHalt = manager->visitMembers(nullptr, Manager::Invalid);
  test(didHalt == smtk::common::Visit::Halt, "Expected barfage when passing a null visitor.");
  test(vcount == 0, "Expected to visit 0 entries.");

  vcount = 0;
  didHalt = manager->visitMembers(visitor, Manager::Invalid);
  test(
    didHalt == smtk::common::Visit::Continue,
    "Not expecting barfage when passing a valid visitor.");
  test(vcount == 11, "Expected to visit 11 entries (unixy has a duplicate).");

  vcount = 0;
  didHalt = manager->visitMembers(visitor, hashes[0]);
  test(vcount == 5, "Expected to visit 5 entries.");

  vcount = 0;
  didHalt = manager->visitMembers(visitor, hashes[1]);
  test(vcount == 4, "Expected to visit 4 entries (unixy has a duplicate).");

  vcount = 0;
  didHalt = manager->visitMembers(visitor, /* not a set*/ hashes[2]);
  test(vcount == 0, "Expected to visit 0 entries.");

  vcount = 0;
  didHalt = manager->visitMembers(visitor, /* unmanaged hash */ manager->compute("not there"));
  test(didHalt == smtk::common::Visit::Continue, "Expected barfage when passing bad set-hash.");
  test(vcount == 0, "Expected to visit 0 entries.");
  // Test visitation of set hash-names.
  // smtk::common::Visit visitSets(Visitor visitor);

  // Test that removal of a set removes set members.
  std::string name = manager->value(hashes[0]);
  auto numRemoved = manager->unmanage(hashes[0]);
  std::cout << "Removing " << name << " erased " << numRemoved << " entries.\n";
  test(numRemoved == 6, "Expected to remove fooset and all its members.");
  test(manager->find("fooset") == Manager::Invalid, "Expected fooset to be removed.");
  for (int ii = 0; ii < 5; ++ii)
  {
    test(manager->find(s1[ii]) == Manager::Invalid, "Expected " + s1[ii] + " to be removed.");
  }

  // Test removal from set but not "un-manage-ment."
  // This also tests removal of something that has already been removed (s2[4] == s2[1]).
  for (int ii = 0; ii < 5; ++ii)
  {
    bool didRemove = manager->remove(hashes[1], hashes[7 + ii]);
    test(ii == 4 ? !didRemove : didRemove, "Expected to remove " + s2[ii] + ".");
  }

  // Test lookup from hash and that a bad hash fails.
  test(manager->value(smtk::string::Manager::Invalid).empty(), "Expected an empty string.");

  // Test lookup and that an unmanaged string returns Invalid.
  test(
    manager->find("not there") == Manager::Invalid,
    "Expected an unmanaged string to return an invalid hash.");

  // Test that compute() will never return Manager::Invalid.
  // We can't test all unputs, but we can try a few.
  test(!!manager->compute(std::string()), "Expected a valid hash from an empty string.");

  /*
  // Test removal (by string and by hash)
  bool remove(const std::string& set, Hash h);
  bool remove(Hash set, Hash h);
  */
  std::cout << "\nobserved " << ocount << " events\n\n";
  test(ocount == 15, "Expected 15 observer firings.");
  ocount = 0;

  // Test JSON deserialization.
  std::cout << "Resetting manager from JSON above\n";
  manager = j;

  vcount = 0;
  didHalt = manager->visitMembers(visitor, Manager::Invalid);
  std::cout << vcount << " members\n";
  test(vcount == 11, "Expected to deserialize 11 members.");

  vcount = 0;
  didHalt = manager->visitSets(visitor);
  std::cout << vcount << " sets\n";
  test(vcount == 2, "Expected to deserialize 2 sets.");

  return 0;
}
