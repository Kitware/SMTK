//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUIDGenerator.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <thread>

namespace
{
class ResourceA : public smtk::resource::DerivedFrom<ResourceA, smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  smtk::resource::ComponentPtr find(const smtk::common::UUID& /*compId*/) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& /*v*/) const override {}

protected:
  ResourceA() {}
};
} // namespace

int TestLock(int /*unused*/, char** const /*unused*/)
{
  using namespace smtk::resource;

  // Create a resource manager and register ResourceA
  ManagerPtr resourceManager = Manager::create();
  resourceManager->registerResource<ResourceA>();

  // Create a new ResourceA type
  auto a1 = resourceManager->create<ResourceA>();
  auto a2 = resourceManager->create<ResourceA>();
  auto a3 = resourceManager->create<ResourceA>();

  // Test nested Block/Try calls.
  {
    auto guard1 = ScopedLockSetGuard::Block({ a1, a2 }, { a3 });
    // Verify that we hold the locks:
    smtkTest(a1->locked() == LockType::Read, "Resource a1 not read-locked.");
    smtkTest(a2->locked() == LockType::Read, "Resource a2 not read-locked.");
    smtkTest(a3->locked() == LockType::Write, "Resource a3 not write-locked.");

    // Test that immediate locking of locked resources fails.
    if (auto guard2 = ScopedLockSetGuard::Try({ a1, a3 }, { a2 }))
    {
      smtkTest(false, "Locking write-locked resources should never succeed.");
    }

    // Uncomment this if you want to verify that a deadlock occurs.
    // auto guard3 = ScopedLockSetGuard::Block({}, {a3});
    // smtkTest(false, "Succeeded in double-write-locking resource a3. Oops.");

    (void)guard1;
  }

  // Test that locks were released:
  smtkTest(a1->locked() == LockType::Unlocked, "Resource a1 not unlocked.");
  smtkTest(a2->locked() == LockType::Unlocked, "Resource a2 not unlocked.");
  smtkTest(a3->locked() == LockType::Unlocked, "Resource a3 not unlocked.");

  // Test nested Try calls.
  if (auto guard = ScopedLockSetGuard::Try({ a3 }, { a1, a2 }))
  {
    // Verify that we hold the locks:
    smtkTest(a1->locked() == LockType::Write, "Resource a1 not write-locked.");
    smtkTest(a2->locked() == LockType::Write, "Resource a2 not write-locked.");
    smtkTest(a3->locked() == LockType::Read, "Resource a3 not read-locked.");

    // Verify that read-locking the same resource multiple times works:
    if (auto guard2 = ScopedLockSetGuard::Try({ a3 }, {}))
    {
      smtkTest(a3->locked() == LockType::Read, "Resource a3 not read-locked.");
    }
    else
    {
      smtkTest(false, "Unable to multiply-lock read-locked resource.");
    }
  }
  else
  {
    smtkTest(false, "Locking unlocked resources should succeed but did not.");
  }

  return 0;
}
