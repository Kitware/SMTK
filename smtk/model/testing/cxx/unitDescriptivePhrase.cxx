//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/BridgeSession.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/SimpleModelSubphrases.h"

#include "smtk/io/ImportJSON.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <fstream>
#include <iostream>
#include <string>

#include <stdlib.h>

using smtk::shared_ptr;
using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  ManagerPtr sm = Manager::create();

  // Block to ensure timely destruction of JSON data.
    {
    std::string fname(argc > 1 ? argv[1] : "smtkModel.json");
    std::ifstream file(fname.c_str());
    std::string data(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

    if (data.empty() || !ImportJSON::intoModelManager(data.c_str(), sm))
      {
      std::cerr << "Error importing model from file \"" << fname << "\"\n";
      return 1;
      }
    }
  sm->assignDefaultNames();

  sm->bridgeForModel(UUID::null());
  BridgeSessions ents = sm->allSessions();
  std::cout << ents.size() << " bridge sessions.\n";
  test(ents.size() == 1, "Expected a single bridge session.");

  // Assign all the models to the lone (default, native) session.
  ModelEntities models =
    sm->entitiesMatchingFlagsAs<ModelEntities>(
      MODEL_ENTITY, false);
  for (ModelEntities::iterator mit = models.begin(); mit != models.end(); ++mit)
    sm->setBridgeForModel(ents[0].bridge(), mit->entity());

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
    printPhrase(std::cout, 0, elist);
    }
  else
    {
    std::cerr << "No model entities in the model manager.\n";
    }
  return 0;
}
