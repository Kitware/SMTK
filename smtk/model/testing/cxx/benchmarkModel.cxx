//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Resource.h"
#include "smtk/model/json/jsonResource.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <fstream>
#include <iostream>

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  ResourcePtr sm = Resource::create();
  Timer t;
  double deltaT;

  // ### Benchmark entity creation ###
  // This includes generating new UUIDs which takes up the bulk of the time.
  int numObj = 1000;
  t.mark();
  for (int i = 0; i < numObj; ++i)
  {
    createTet(sm);
  }
  deltaT = t.elapsed();
  std::cout << numObj << " objects " << deltaT << " seconds " << (numObj / deltaT) << " objs/sec\n"
            << "  " << sm->topology().size() << " entities " << deltaT << " seconds "
            << (sm->topology().size() / deltaT) << " entities/sec\n";

  // ### Benchmark entity lookup ###
  // #### Misses
  int numMisses = 2000000;
  t.mark();
  UUID nil;
  for (int i = 0; i < numMisses; ++i)
  {
    EntityPtr ent = sm->findEntity(nil);
    (void)ent;
    (*nil.begin())++; // twiddling bits should still result in a missing UUID.
  }
  deltaT = t.elapsed();
  std::cout << numMisses << " missed lookups " << deltaT << " seconds " << (numMisses / deltaT)
            << " missed lookups/sec\n";

  // #### Hits
  UUIDWithEntityPtr it;
  it = sm->topology().begin();
  do
    ++it;
  while (it != sm->topology().end() && it->second->relations().empty());
  int numHits = 2000000;
  t.mark();
  for (int i = 0; i < numHits; ++i)
  {
    EntityPtr ent = sm->findEntity(it->second->relations().front());
    (void)ent;
    do
      ++it;
    while (it != sm->topology().end() && it->second->relations().empty());
    if (it == sm->topology().end())
      it = sm->topology().begin();
  }
  deltaT = t.elapsed();
  std::cout << numHits << " missed lookups " << deltaT << " seconds " << (numHits / deltaT)
            << " good lookups/sec.\n";

  // ### Benchmark JSON export ###
  t.mark();
  nlohmann::json json = sm;
  deltaT = t.elapsed();
  std::cout << deltaT << " seconds to export " << sm->topology().size() << " entity records and "
            << sm->tessellations().size() << " tessellations.";
  t.mark();

  // ### Benchmark JSON import ###
  {
    ResourcePtr sm2 = Resource::create();
    t.mark();
    smtk::model::from_json(json, sm2);
    deltaT = t.elapsed();
  }
  std::cout << deltaT << " seconds to ingest JSON.\n";

  return 0;
}
