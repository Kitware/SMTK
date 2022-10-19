//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/TypeName.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/common/update/Factory.h"

#include <iostream>

namespace
{
class Blob
{
public:
  virtual ~Blob() = default;
};

class OldBlob : public Blob
{
public:
  OldBlob() = default;
  ~OldBlob() override = default;
  int BlobVersion = 1;
  int Data = 2;
};

class NewBlob : public Blob
{
public:
  NewBlob() = default;
  ~NewBlob() override = default;
  int BlobVersion = 10;
  int Data = 5;
};

class ReallyNewBlob : public Blob
{
public:
  ReallyNewBlob() = default;
  ~ReallyNewBlob() override = default;
  int BlobVersion = 100;
  int Data = 20;
};

} // anonymous namespace

int UnitTestUpdateFactory(int /*unused*/, char** const /*unused*/)
{
  using BlobUpdateSignature = std::function<bool(const Blob*, Blob*)>;
  using BlobUpdateFactory = smtk::common::update::Factory<BlobUpdateSignature>;
  BlobUpdateFactory updaters;
  bool didRegister;
  std::string oldBlobType = smtk::common::typeName<OldBlob>();
  std::string newBlobType = smtk::common::typeName<NewBlob>();
  std::string reallyNewBlobType = smtk::common::typeName<ReallyNewBlob>();

  didRegister = updaters.registerUpdater("", 1, 2, 3, [](const Blob*, Blob*) { return false; });
  test(!didRegister, "Should refuse to register an empty target.");

  didRegister =
    updaters.registerUpdater(oldBlobType, 1, 0, 3, [](const Blob*, Blob*) { return false; });
  test(!didRegister, "Should refuse to register an invalid input-version range.");

  didRegister =
    updaters.registerUpdater(oldBlobType, 1, 2, 2, [](const Blob*, Blob*) { return false; });
  test(!didRegister, "Should refuse to register an invalid output version.");

  BlobUpdateSignature nullUpdater;
  didRegister = updaters.registerUpdater(oldBlobType, 1, 2, 3, nullUpdater);
  test(!didRegister, "Should refuse to register an null updater.");

  didRegister =
    updaters.registerUpdater(oldBlobType, 1, 2, 5, [](const Blob* fromBase, Blob* toBase) {
      const auto* from = dynamic_cast<const OldBlob*>(fromBase);
      auto* to = dynamic_cast<NewBlob*>(toBase);
      if (!from || !to)
      {
        return false;
      }

      std::cout << "Updating OldBlob v1–2 to NewBlob v5\n";
      to->Data = 3 * from->Data - 1;
      to->BlobVersion = 5;
      return true;
    });
  test(didRegister, "Should register an valid updater (1).");

  didRegister =
    updaters.registerUpdater(oldBlobType, 1, 3, 10, [](const Blob* fromBase, Blob* toBase) {
      const auto* from = dynamic_cast<const OldBlob*>(fromBase);
      auto* to = dynamic_cast<NewBlob*>(toBase);
      if (!from || !to)
      {
        return false;
      }

      std::cout << "Updating OldBlob v1–3 to NewBlob v10\n";
      to->Data = 2 * from->Data + 1;
      to->BlobVersion = 10;
      return true;
    });
  test(didRegister, "Should register an valid updater (2).");

  didRegister =
    updaters.registerUpdater(newBlobType, 10, 11, 12, [](const Blob* fromBase, Blob* toBase) {
      const auto* from = dynamic_cast<const NewBlob*>(fromBase);
      auto* to = dynamic_cast<ReallyNewBlob*>(toBase);
      if (!from || !to)
      {
        return false;
      }

      std::cout << "Updating NewBlob v10–11 to ReallyNewBlob v100\n";
      to->Data = 4 * from->Data + 2;
      to->BlobVersion = 12;
      return true;
    });

  OldBlob oldBlob1;
  OldBlob oldBlob2;
  OldBlob oldBlob3;
  NewBlob newBlob1;
  NewBlob newBlob2;
  NewBlob newBlob3;
  ReallyNewBlob reallyNewBlob;
  bool didUpdate = false;

  // Test a valid updater to NewBlob's default version.
  if (
    const auto& updater = updaters.find(
      smtk::common::typeName<decltype(oldBlob1)>(), oldBlob1.BlobVersion, newBlob1.BlobVersion))
  {
    didUpdate = updater.Update(&oldBlob1, &newBlob1);
    test(didUpdate, "Expected to update valid inputs.");
    test(newBlob1.BlobVersion == 10, "Expected version 10.");
    test(newBlob1.Data == 5, "Expected data 5 (perhaps bad updater selected?).");
  }

  // Test a valid updater to a non-default NewBlob version.
  if (
    const auto& updater =
      updaters.find(smtk::common::typeName<decltype(oldBlob2)>(), oldBlob2.BlobVersion, 5))
  {
    didUpdate = updater.Update(&oldBlob1, &newBlob1);
    test(didUpdate, "Expected to update valid inputs.");
    test(newBlob1.BlobVersion == 5, "Expected version 10.");
    test(newBlob1.Data == 5, "Expected data 5 (perhaps bad updater selected?).");
  }

  // Test that updating fails when no updater is present.
  if (
    const auto& updater = updaters.find(
      smtk::common::typeName<decltype(newBlob1)>(),
      newBlob1.BlobVersion,
      reallyNewBlob.BlobVersion))
  {
    test(!updater, "Returned updater should be invalid.");
    test(false, "No updaters should match target name and version.");
  }

  auto oldVersionsIn = updaters.canAccept(smtk::common::typeName<OldBlob>());
  auto newVersionsOut = updaters.canProduce(smtk::common::typeName<OldBlob>());
  std::cout << "OldBlob updaters accept versions:";
  for (const auto& vv : oldVersionsIn)
  {
    std::cout << "  " << vv;
  }
  std::cout << "\n";
  std::cout << "OldBlob updaters produce versions:";
  for (const auto& vv : newVersionsOut)
  {
    std::cout << "  " << vv;
  }
  std::cout << "\n";
  test(oldVersionsIn == std::set<std::size_t>{ 1, 2, 3 }, "Unexpected old version numbers in.");
  test(newVersionsOut == std::set<std::size_t>{ 5, 10 }, "Unexpected new version numbers out.");

  auto newVersionsIn = updaters.canAccept(smtk::common::typeName<NewBlob>());
  auto btrVersionsOut = updaters.canProduce(smtk::common::typeName<NewBlob>());
  std::cout << "NewBlob updaters accept versions:";
  for (const auto& vv : newVersionsIn)
  {
    std::cout << "  " << vv;
  }
  std::cout << "\n";
  std::cout << "NewBlob updaters produce versions:";
  for (const auto& vv : btrVersionsOut)
  {
    std::cout << "  " << vv;
  }
  std::cout << "\n";
  test(newVersionsIn == std::set<std::size_t>{ 10, 11 }, "Unexpected new version numbers in.");
  test(btrVersionsOut == std::set<std::size_t>{ 12 }, "Unexpected really new version numbers out.");

  return 0;
}
