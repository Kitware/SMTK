//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "vtkNew.h"
#include "vtkPolyData.h"

#include "smtk/common/testing/cxx/helpers.h"

using UUID = smtk::common::UUID;
using SequenceType = vtkResourceMultiBlockSource::SequenceType;
constexpr SequenceType invalid = vtkResourceMultiBlockSource::InvalidSequence;

namespace
{

void TestCache()
{
  std::cout << "Verify that dataset caching works.\n";
  vtkNew<vtkPolyData> d1;
  vtkNew<vtkPolyData> d2;
  UUID u1 = UUID::random();
  UUID u2 = UUID::random();

  bool ok;
  SequenceType seq;
  vtkDataObject* obj;

  // This block exists so we can test that resources are freed when src is destroyed.
  {
    vtkNew<vtkModelMultiBlockSource> src;

    // Test methods on empty cache.
    seq = src->GetCachedDataSequenceNumber(u1);
    test(seq == invalid, "Expect invalid sequence number for invalid UUID.");

    obj = src->GetCachedDataObject(u1);
    test(obj == nullptr, "Expect null pointer for invalid UUID.");

    ok = src->RemoveCacheEntry(u1);
    test(!ok, "Expect removal of non-existent entry to fail.");

    ok = src->SetCachedData(u1, nullptr, 1);
    test(!ok, "Expect insertion of nullptr to fail.");

    seq = src->GetCachedDataSequenceNumber(u1);
    test(seq == invalid, "Expect sequence number to be invalid after failed insertion.");

    // Test methods that modify cache.
    ok = src->SetCachedData(u1, d1, 1);
    test(ok, "Expect insertion to succeed.");
    test(d1->GetReferenceCount() == 2, "Expect cache to take ownership of data.");

    obj = src->GetCachedDataObject(u1);
    test(obj == d1, "Expect valid pointer for valid UUID.");

    seq = src->GetCachedDataSequenceNumber(u1);
    test(seq == 1, "Expect sequence number to be updated on insertion.");

    ok = src->SetCachedData(u1, d2, 0);
    test(!ok, "Expect out-of-order insertion to fail.");
    test(d1->GetReferenceCount() == 2,
      "Expect cache to maintain data ownership on insertion failure.");

    seq = src->GetCachedDataSequenceNumber(u1);
    test(seq == 1, "Expect sequence number to be unchanged on insertion failure.");

    ok = src->SetCachedData(u1, d2, 2);
    test(ok, "Expection in-order replacement to succeed.");
    test(d1->GetReferenceCount() == 1,
      "Expect cache to drop ownership of existing data on insertion success.");
    test(d2->GetReferenceCount() == 2,
      "Expect cache to take ownership of new data on insertion success.");

    // Test methods that remove cache entries.
    ok = src->SetCachedData(u1, d1, 3);
    ok = src->SetCachedData(u2, d2, 0);
    test(ok, "Expect second cache insertion to succeed.");
    test(d2->GetReferenceCount() == 2, "Expect second cache insertion to take ownership of data.");

    src->ClearCache();
    test(d1->GetReferenceCount() == 1,
      "Expect cache to drop ownership of existing data on insertion success.");
    test(d2->GetReferenceCount() == 1,
      "Expect cache to take ownership of new data on insertion success.");

    ok = src->SetCachedData(u1, d1, 0);
    test(ok, "Expect insertion with stale sequenceto succeed after clear.");
    ok = src->SetCachedData(u2, d2, 0);
    test(ok, "Expect insertion with stale sequenceto succeed after clear.");

    ok = src->RemoveCacheEntry(u1);
    test(ok, "Expect removal of individual entry to succeed.");
    test(d1->GetReferenceCount() == 1, "Expect cache to drop ownership on removal.");

    ok = src->RemoveCacheEntry(u1);
    test(!ok, "Expect duplicate removal of individual entry to fail.");

    ok = src->SetCachedData(u1, d1, 0);
    ok = src->SetCachedData(u2, d2, 0);
    std::set<UUID> exceptions;
    exceptions.insert(u1);
    exceptions.insert(u2);
    ok = src->RemoveCacheEntriesExcept(exceptions);
    test(!ok, "Expected no removals with full set.");

    exceptions.erase(u2);
    ok = src->RemoveCacheEntriesExcept(exceptions);
    test(ok, "Expected to remove an entry.");
    test(d1->GetReferenceCount() == 2, "Expect cache to keep ownership of exceptional entry.");
    test(d2->GetReferenceCount() == 1, "Expect cache to drop ownership of unexceptional entry.");

    exceptions.erase(u1);
    ok = src->RemoveCacheEntriesExcept(exceptions);
    test(ok, "Expected to remove an entry.");
    test(d1->GetReferenceCount() == 1, "Expect cache to drop ownership of unexceptional entry.");

    // Test that destruction of src releases resources.
    ok = src->SetCachedData(u1, d1, 0);
    ok = src->SetCachedData(u2, d2, 0);
    test(d1->GetReferenceCount() == 2, "Expect cache to keep ownership before destruction.");
    test(d2->GetReferenceCount() == 2, "Expect cache to keep ownership before destruction.");
  }
  test(d1->GetReferenceCount() == 1, "Expect cache to drop ownership on destruction.");
  test(d2->GetReferenceCount() == 1, "Expect cache to drop ownership on destruction.");

  std::cout << "  ... Done.\n";
}
}

int unitResourceMultiBlockSource(int, char** const)
{
  TestCache();

  return 0;
}
