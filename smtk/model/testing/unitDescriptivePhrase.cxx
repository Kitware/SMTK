#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/PropertyListPhrase.h"

#include "smtk/model/ImportJSON.h"

#include "smtk/model/testing/helpers.h"

#include <iostream>

#include <assert.h>

using smtk::shared_ptr;
using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

void prindent(std::ostream& os, int indent, DescriptivePhrase::Ptr p)
{
  os << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")" << "\n";
  DescriptivePhrases sub = p->subphrases();
  indent += 2;
  for (DescriptivePhrases::iterator it = sub.begin(); it != sub.end(); ++it)
    {
    prindent(os, indent, *it);
    }
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  StoragePtr sm = Storage::New();

  UUIDArray uids = createTet(sm);
  Cursor vert(sm, uids[0]);
  sm->assignDefaultNames();

  DescriptivePhrase::Ptr dit;
  PropertyListPhrase::Ptr ppf = PropertyListPhrase::create()->setup(vert, FLOAT_PROPERTY, dit);
  PropertyListPhrase::Ptr pps = PropertyListPhrase::create()->setup(vert, STRING_PROPERTY, dit);
  PropertyListPhrase::Ptr ppi = PropertyListPhrase::create()->setup(vert, INTEGER_PROPERTY, dit);
  prindent(std::cout, 0, ppf);
  prindent(std::cout, 0, pps);
  prindent(std::cout, 0, ppi);
  return 0;
}
