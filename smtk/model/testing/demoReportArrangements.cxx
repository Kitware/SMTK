#include "smtk/model/Cursor.h"

#include "smtk/model/ExportJSON.h"
#include "smtk/model/ImportJSON.h"
#include "smtk/model/Storage.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"

#include "smtk/model/testing/helpers.h"

#include <fstream>
#include <string>
#include <iostream>

using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;
using smtk::shared_ptr;

int main(int argc, char* argv[])
{
  std::ifstream file(argc > 1 ? argv[1] : "smtkModel.json");
  char* endMask;
  smtk::model::BitFlags mask =
    static_cast<smtk::model::BitFlags>(
      strtol(argc > 2 ? argv[2] : "0xffffffff", &endMask, 16));
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  StoragePtr sm = Storage::New();

  int status = 0;
  status |= ImportJSON::intoModel(data.c_str(), sm);
  if (status)
    {
    if (argc > 3)
      {
      sm->assignDefaultNames();
      }
    UUIDWithArrangementDictionary ait;
    UUIDWithEntity eit;
    UUIDWithFloatProperties fpit;
    UUIDWithStringProperties spit;
    UUIDWithIntegerProperties ipit;
    for (eit = sm->topology().begin(); eit != sm->topology().end(); ++eit)
      {
      if (!(eit->second.entityFlags() & mask))
        { // Skip entities that don't overlap our mask
        continue;
        }
      ait = sm->arrangements().find(eit->first);
      if (ait != sm->arrangements().end())
        {
        std::cout << sm->name(eit->first) << " (" << eit->second.flagSummary();
        if (eit->second.dimension() >= 0)
          {
          std::cout << ", dim " << eit->second.dimension();
          }
        std::cout << ")\n";
        ArrangementKindWithArrangements kit;
        for (kit = ait->second.begin(); kit != ait->second.end(); ++kit)
          {
          if (!kit->second.empty())
            {
            if (kit->second.size() == 1)
              {
              std::cout
                << "        1 " << NameForArrangementKind(kit->first)
                << "s (" << kit->second.begin()->details().size() << " entries)\n";
              }
            else
              {
              std::cout << "        " << kit->second.size() << " " << NameForArrangementKind(kit->first) << "s\n";
              }
            }
          }
        }
      fpit = sm->floatPropertiesForEntity(eit->first);
      if (fpit != sm->floatProperties().end())
        {
        PropertyNameWithFloats fpval;
        std::cout << "        " << fpit->second.size() << " float properties:\n";
        for (fpval = fpit->second.begin(); fpval != fpit->second.end(); ++fpval)
          {
          if (!fpval->second.empty())
            {
            std::cout << "          " << fpval->second.size() << " values for " << fpval->first << "\n";
            }
          }
        }
      spit = sm->stringPropertiesForEntity(eit->first);
      if (spit != sm->stringProperties().end())
        {
        PropertyNameWithStrings spval;
        std::cout << "        " << spit->second.size() << " string properties:\n";
        for (spval = spit->second.begin(); spval != spit->second.end(); ++spval)
          {
          if (!spval->second.empty())
            {
            std::cout << "          " << spval->second.size() << " values for " << spval->first << "\n";
            }
          }
        }
      ipit = sm->integerPropertiesForEntity(eit->first);
      if (ipit != sm->integerProperties().end())
        {
        PropertyNameWithIntegers ipval;
        std::cout << "        " << ipit->second.size() << " integer properties:\n";
        for (ipval = ipit->second.begin(); ipval != ipit->second.end(); ++ipval)
          {
          if (!ipval->second.empty())
            {
            std::cout << "          " << ipval->second.size() << " values for " << ipval->first << "\n";
            }
          }
        }
      }
    }

  return status == 0;
}
