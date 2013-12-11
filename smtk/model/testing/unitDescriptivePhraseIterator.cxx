#include "smtk/model/DescriptivePhraseIterator.h"

#include "smtk/model/ImportJSON.h"

#include "smtk/model/testing/helpers.h"

#include <iostream>

#include <assert.h>

using smtk::shared_ptr;
using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  StoragePtr sm = Storage::New();

  UUIDArray uids = createTet(sm);
  Cursor vert(sm, uids[0]);

  DescriptivePhraseIterator dit(vert);
  std::cout << dit.phrase() << "\n";
  return 0;
}
