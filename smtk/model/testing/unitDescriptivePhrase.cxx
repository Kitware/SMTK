#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/SimpleModelSubphrases.h"

#include "smtk/model/ImportJSON.h"

#include "smtk/model/testing/helpers.h"

#include <fstream>
#include <iostream>
#include <string>

#include <stdlib.h>

using smtk::shared_ptr;
using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

static int maxIndent = 10;

void prindent(std::ostream& os, int indent, DescriptivePhrase::Ptr p)
{
  // Do not descend too far, as infinite recursion is possible,
  // even with the SimpleSubphraseGenerator
  if (indent > maxIndent)
    return;

  os << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")";
  FloatList rgba = p->relatedColor();
  if (rgba[3] >= 0.)
    os << " rgba(" << rgba[0] << "," << rgba[1] << "," << rgba[2] << "," << rgba[3] << ")";
  os << "\n";
  DescriptivePhrases sub = p->subphrases();
  indent += 2;
  for (DescriptivePhrases::iterator it = sub.begin(); it != sub.end(); ++it)
    {
    prindent(os, indent, *it);
    }
}

int main(int argc, char* argv[])
{
  StoragePtr sm = Storage::create();
  if (argc > 2)
    maxIndent = atol(argv[2]);

  // Block to ensure timely destruction of JSON data.
    {
    std::string fname(argc > 1 ? argv[1] : "smtkModel.json");
    std::ifstream file(fname.c_str());
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

  CursorArray faces;
  Cursor::CursorsFromUUIDs(
    faces, sm, sm->entitiesMatchingFlags(CELL_2D, true));
  for (CursorArray::iterator it = faces.begin(); it != faces.end(); ++it)
    {
    it->setColor(0.5, 0.5, 0.5, 1.); // Make every face grey.
    }

  if (!ents.empty())
    {
    DescriptivePhrase::Ptr dit;
    EntityListPhrase::Ptr elist = EntityListPhrase::create()->setup(ents, dit);
    SimpleModelSubphrases::Ptr spg = SimpleModelSubphrases::create();
    elist->setDelegate(spg);
    prindent(std::cout, 0, elist);
    }
  else
    {
    std::cerr << "No model entities in storage\n";
    }
  return 0;
}
