//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityIterator.h"
#include "smtk/model/Model.h"
#include "smtk/model/Volume.h"

#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <sstream>

using namespace smtk::model;
using namespace smtk::common;
using namespace smtk::model::testing;

void testExplicitTraversal(smtk::io::JSONRecords style, int correctCount)
{
  Manager::Ptr mgr = Manager::create();
  SessionRef sess = mgr->createSession("native");

  UUIDArray uids = createTet(mgr);
  Model model = mgr->addModel();
  model.addCell(Volume(mgr, uids[21]));
  model.setSession(sess);
  mgr->assignDefaultNames();

  EntityRefs justVerts = mgr->entitiesMatchingFlagsAs<EntityRefs>(VERTEX);
  EntityIterator it;
  it.traverse(justVerts.begin(), justVerts.end(), style);
  int count1 = 0;
  std::cout << "\n---\n\n";
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    std::cout << "  " << it->name() << " (" << it->flagSummary() << ")\n";
    ++count1;
    }

  std::cout << "\n---\n\n" << "  " << count1 << "\n";
  std::ostringstream msg;
  msg
    << "Expected to iterate over " << correctCount
    << " entities, did iterate over " << count1 << ".";
  test(count1 == correctCount, msg.str());
}

void testModelTraversal()
{
  Manager::Ptr mgr = Manager::create();
  SessionRef sess = mgr->createSession("native");

  UUIDArray uids = createTet(mgr);
  Model model = mgr->addModel();
  model.addCell(Volume(mgr, uids[21]));
  model.setSession(sess);
  mgr->assignDefaultNames();

  EntityIterator it;
  it.traverse(model);
  int count1 = 0;
  std::cout << "\n---\n\n";
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    std::cout << "  " << it->name() << " (" << it->flagSummary() << ")\n";
    ++count1;
    }

  int count2 = 0;
  std::cout << "\n---\n\n";
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    std::cout << "  " << it->name() << " (" << it->flagSummary() << ")\n";
    ++count2;
    }

  std::cout << "\n---\n\n" << "  " << count1 << "  " << count2 << "\n";
  test(count1 == 78, "Expected to iterate over 78 entities");
  test(count1 == count2, "Expected iterating twice to count the same number of items.");
}

int main()
{
  testModelTraversal();
  testExplicitTraversal(smtk::io::JSON_BARE, 7);
  testExplicitTraversal(smtk::io::JSON_CHILDREN, 14); // Vertex uses are children of vertices
  testExplicitTraversal(smtk::io::JSON_MODELS, 79);
  // TODO: Test iteration while model is being modified.
  return 0;
}
