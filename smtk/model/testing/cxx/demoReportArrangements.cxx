//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/json/jsonResource.h"

#include "smtk/model/testing/cxx/helpers.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;
using namespace smtk::io;

smtk::model::BitFlags maskOrder[] = { MODEL_ENTITY, INSTANCE_ENTITY, GROUP_ENTITY, CELL_3D, CELL_2D,
  CELL_1D, CELL_0D, SHELL_2D, USE_2D, SHELL_1D, USE_1D, SHELL_0D, USE_0D };

void ReportEntity(ResourcePtr sm, UUIDWithEntityPtr& eit)
{
  const KindsToArrangements& kwa(eit->second->arrangementMap());
  UUIDWithFloatProperties fpit;
  UUIDWithStringProperties spit;
  UUIDWithIntegerProperties ipit;
  if (!kwa.empty())
  {
    std::cout << sm->name(eit->first) << " (" << eit->second->flagSummary();
    if (eit->second->dimension() >= 0)
    {
      std::cout << ", dim " << eit->second->dimension();
    }
    std::cout << ")\n";
    for (auto kit = kwa.begin(); kit != kwa.end(); ++kit)
    {
      if (!kit->second.empty())
      {
        if (kit->second.size() == 1)
        {
          std::cout << "        1 " << NameForArrangementKind(kit->first) << "s ("
                    << kit->second.begin()->details().size() << " entries)\n";
        }
        else
        {
          std::cout << "        " << kit->second.size() << " " << NameForArrangementKind(kit->first)
                    << "s\n";
        }
      }
    }
  }
}

int main(int argc, char* argv[])
{
  std::ifstream file(argc > 1 ? argv[1] : "smtkModel.json");
  char* endMask;
  smtk::model::BitFlags mask =
    static_cast<smtk::model::BitFlags>(strtol(argc > 2 ? argv[2] : "0xffffffff", &endMask, 16));
  std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

  ResourcePtr sm = Resource::create();

  int status = 0;
  smtk::model::from_json(data, sm);
  if (status)
  {
    if (argc > 3)
    {
      sm->assignDefaultNames();
    }
    size_t numSections = sizeof(maskOrder) / sizeof(maskOrder[0]);
    for (size_t section = 0; section < numSections; ++section)
    {
      if ((mask & maskOrder[section]) != maskOrder[section])
        continue; // skip sections that do not overlap the mask.
      std::cout << "\n## " << Entity::flagSummary(maskOrder[section], 1) << " ##\n\n";
      UUIDWithEntityPtr eit;
      for (eit = sm->topology().begin(); eit != sm->topology().end(); ++eit)
      {
        if ((eit->second->entityFlags() & maskOrder[section]) != maskOrder[section])
        { // Skip entities that don't overlap our mask
          continue;
        }
        ReportEntity(sm, eit);
      }
    }
  }

  return status == 0;
}
