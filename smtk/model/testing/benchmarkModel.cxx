#include "smtk/model/Storage.h"
#include "smtk/model/ImportJSON.h"
#include "smtk/model/testing/helpers.h"

#include "cJSON.h"

#include <assert.h>

using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  UUIDsToEntities smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  Storage sm(&smTopology, &smArrangements, &smTessellation);

  Timer t;
  double deltaT;
  int numObj = 100;
  t.mark();
  for (int i = 0; i < numObj; ++i)
    {
    createTet(sm);
    }
  deltaT = t.elapsed();
  std::cout << numObj << " objects " << deltaT << " seconds " << (numObj / deltaT) << " objs/sec\n";

  int numMisses = 10000;
  t.mark();
  for (int i = 0; i < numMisses; ++i)
    {
    UUID nil;
    Entity* ent = sm.findEntity(nil);
    (void) ent;
    }
  deltaT = t.elapsed();
  std::cout << numMisses << " missed lookups " << deltaT << " seconds " << (numMisses / deltaT) << " missed lookups/sec\n";

  UUIDWithEntity it;
  it = sm.topology().begin();
  do
    ++it;
  while (it != sm.topology().end() && it->second.relations().empty());
  int numHits = 10000;
  t.mark();
  for (int i = 0; i < numHits; ++i)
    {
    Entity* ent = sm.findEntity(it->second.relations().front());
    do
      ++it;
    while (it != sm.topology().end() && it->second.relations().empty());
    if (it == sm.topology().end()) it = sm.topology().begin();
    }
  deltaT = t.elapsed();
  std::cout << numHits << " missed lookups " << deltaT << " seconds " << (numHits / deltaT) << " good lookups/sec\n";

  return 0;
}
