#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/PropertyListPhrase.h"

#include "smtk/model/ImportJSON.h"

#include "smtk/model/testing/helpers.h"

#include <fstream>
#include <iostream>
#include <string>

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
  StoragePtr sm = Storage::New();

  // Block to ensure timely destruction of JSON data.
    {
    std::string fname(argc > 1 ? argv[1] : "smtkModel.json");
    std::ifstream file(fname);
    std::string data(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

    if (data.empty() || !ImportJSON::intoModel(data.c_str(), sm))
      {
      std::cerr << "Error importing model from file \"" << fname << "\"\n";
      return 1;
      }
    }
  sm->assignDefaultNames();

  Cursors ents;
  Cursor::CursorsFromUUIDs(
    ents, sm, sm->entitiesMatchingFlags(MODEL_ENTITY, false));

  if (!ents.empty())
    {
    DescriptivePhrase::Ptr dit;
    EntityListPhrase::Ptr elist = EntityListPhrase::create()->setup(ents, dit);
    prindent(std::cout, 0, elist);
    }
  else
    {
    std::cerr << "No model entities in storage\n";
    }
  return 0;
}
