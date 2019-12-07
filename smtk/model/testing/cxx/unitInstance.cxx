//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Entity.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Instance.txx"
#include "smtk/model/Tessellation.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <sstream>

using namespace smtk::model;

Instance testInstanceCreation(const EntityRef& box)
{
  Instance result = box.resource()->addInstance(box);
  result.setRule("uniform random");
  result.setFloatProperty("voi", { -10, 10, -10, 10, -10, 10 });
  result.setIntegerProperty("sample size", 50);
  result.setName("box placements");
  auto tess = result.generateTessellation();
  (void)tess;

  return result;
}

Instance testInstanceClone(const Instance& instance, std::size_t num)
{
  const int subset[] = { 0, 1, 2, 4, 8, 16, 32 };
  auto sz = sizeof(subset) / sizeof(subset[0]);
  Instance source(instance);
  Instance result = source.clonePlacements(subset, subset + (num < sz ? num : sz));
  std::cout << "  Cloned " << instance.name() << " to " << result.name() << "\n";
  /*
  std::ostringstream name;
  name << "instance " << num;
  result.setName(name.str());
  */

  smtkTest(result.rule() == "tabular", "Unexpected result rule.");
  smtkTest(result.hasFloatProperty("placements"), "No result placements.");
  smtkTest(result.floatProperty("placements").size() == 3 * (num < sz ? num : sz),
    "Incorrect placement array size.");
  smtkTest(result.memberOf() == source, "Expected relation to source instance.");
  smtkTest(source.isMember(result), "Expected relation to subset instance.");
  smtkTest(source.memberOf() != result, "Unexpected relation to subset instance.");
  smtkTest(!result.isMember(source), "Unexpected relation to source instance.");

  return result;
}

std::set<Instance> testInstanceDivide(const Instance& instance, bool merge)
{
  Instance source(instance);
  std::set<Instance> result = source.divide<std::set<Instance> >(merge);
  if (merge)
  {
    smtkTest(!result.empty(), "Always expect merged division to have at least 1 output.");
    smtkTest(result.size() <= 2, "Always expect merged division to have 1 or 2 outputs.");
  }

  // Test that no returned instance is "empty" (in the sense of having no placements)
  for (auto entry : result)
  {
    auto tess = entry.generateTessellation();
    std::ostringstream msg;
    msg << "Unexpected empty output \"" << entry.name() << "\".";
    smtkTest(tess && !tess->coords().empty(), msg.str());
  }
  return result;
}

template <typename Container>
void printDivideSummary(const char* msg, Container& div)
{
  std::cout << msg << " has " << div.size() << " entries\n";
  int cc = 0;
  for (auto ii : div)
  {
    std::cout << "  " << cc++ << ": " << ii.generateTessellation()->coords().size() / 3
              << " placements"
              << " named \"" << ii.name() << "\""
              << " isValid " << (ii.isValid() ? "T" : "F") << " isClone "
              << (ii.isClone() ? "T" : "F") << "\n";
  }
}

template <typename Container>
std::size_t sumDividedPlacementCounts(const Container& div)
{
  std::size_t np = 0;
  for (auto entry : div)
  {
    np += entry.numberOfPlacements();
  }
  return np;
}

int unitInstance(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  std::size_t np;
  auto resource = smtk::model::Resource::create();
  auto model = resource->addModel(/* parametric dim */ 3, /* embedding dim */ 3, "test");
  auto box = resource->addAuxiliaryGeometry(model, /* dimension */ 3);
  std::string baseURL(SMTK_DATA_DIR);
  box.setURL(baseURL + "/model/3d/obj/box.obj");

  auto instance = testInstanceCreation(box);
  smtkTest(instance.prototype() == box, "Instance should have a prototype.");
  smtkTest(!instance.isClone(), "Non-cloned instances should not be marked as clones.");

  // Test cloning portions of instance placements.
  auto sub1 = testInstanceClone(instance, 7); // This is like selecting 7/50 points from instance.
  auto sub2 = testInstanceClone(sub1, 5);     // This is like selecting 5/7 points from sub1.
  auto sub3 = testInstanceClone(sub2, 3);     // This is like selecting 3/5 points from sub2.
  auto sub4 = testInstanceClone(sub3, 3);     // This is like selecting 3/3 points from sub3.
  smtkTest(sub1.prototype() == box, "Cloned instance should have a prototype.");
  smtkTest(sub1.isClone(), "Cloned instances should be marked as clones.");

  auto div1 = testInstanceDivide(instance, /* merge */ false);
  printDivideSummary("div1", div1);
  smtkTest(div1.size() == 4, "Expecting 4 new instances.");
  smtkTest(div1.begin()->prototype() == box, "Divided instances should have a prototype.");
  np = sumDividedPlacementCounts(div1);
  smtkTest(np == 50, "Divided instances should account for all input placements.");

  auto div2 = testInstanceDivide(instance, /* merge */ true);
  printDivideSummary("div2", div2);
  smtkTest(div2.size() == 2, "Expecting 2 new instances.");
  smtkTest(div2.begin()->prototype() == box, "Divided instances should have a prototype.");
  np = sumDividedPlacementCounts(div2);
  smtkTest(np == 50, "Divided instances should account for all input placements.");

  std::set<smtk::model::Instance> empty;
  Instance merge1 = Instance::merge(empty);
  smtkTest(!merge1.isValid(), "Merging an empty set should fail.");

  std::set<smtk::model::Instance> dummy;
  dummy.insert(instance);
  printDivideSummary("dummy", dummy);
  Instance merge2 = Instance::merge(dummy);
  smtkTest(merge2.isValid(), "Merging a single instance should succeed.");
  smtkTest(merge2.isClone() == false, "Merging instances should not result in a clone.");
  smtkTest(merge2 == instance, "Merging a single instance should just return the instance.");
  smtkTest(merge2.numberOfPlacements() == 50, "Expected merge to have 50 placements.");
  std::cout << "... merged to " << merge2.name() << "\n";

  printDivideSummary("div1", div1);
  Instance merge3 = Instance::merge(div1);
  smtkTest(merge3.isValid(), "Merging a non-empty set should succeed.");
  smtkTest(merge3.rule() == "tabular", "Merging instances should yield a tabular result.");
  smtkTest(merge3.isClone() == false, "Merging instances should not result in a clone.");
  smtkTest(merge3.numberOfPlacements() == 50, "Expected merge to have 50 placements.");
  std::cout << "... merged to " << merge3.name() << "\n";

  dummy.clear();
  std::set<std::size_t> accept{ 3, 43 };
  for (auto entry : div1)
  {
    if (accept.find(entry.numberOfPlacements()) != accept.end())
    {
      dummy.insert(entry);
    }
  }
  printDivideSummary("dummy", dummy);
  Instance merge4 = Instance::merge(dummy);
  smtkTest(merge4.isValid(), "Merging a non-empty set should succeed.");
  smtkTest(merge4.rule() == "tabular", "Merging instances should yield a tabular result.");
  smtkTest(merge4.isClone() == false, "Merging instances should not result in a clone.");
  smtkTest(merge4.numberOfPlacements() == 46, "Expected merge to have 46 placements.");
  std::cout << "... merged to " << merge4.name() << "\n";

  auto cone = resource->addAuxiliaryGeometry(model, /* dimension */ 3);
  cone.setURL(baseURL + "/model/3d/obj/cone.obj");
  smtkTest(cone.isValid(), "Unable to create a new auxiliary geometry prototype.");
  Instance tmp(*dummy.begin());
  tmp.setPrototype(cone);
  Instance merge5 = Instance::merge(dummy);
  smtkTest(!merge5.isValid(), "Merging instances with different prototypes should fail.");

  return 0;
}
