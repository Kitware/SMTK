//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/SaveJSON.h"
#include "smtk/io/SaveJSON.txx"
#include "smtk/io/LoadJSON.h"
#include "smtk/io/Logger.h"

#include "smtk/model/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "cJSON.h"

#include <fstream>
#include <iostream>
#include <string>

#include <string.h>

using namespace smtk::io;
using namespace smtk::model;
using namespace smtk::common;
using smtk::shared_ptr;

void testLoggerSerialization1()
{
  // round-trip an empty log
  smtk::io::Logger log;
  smtk::io::Logger log2;

  cJSON* array = cJSON_CreateArray();
  test(
    smtk::io::SaveJSON::forLog(array, log) == 0,
    "Exporting an empty log should return 0");
  test(
    smtk::io::LoadJSON::ofLog(array, log2) == 0,
    "Importing an empty log should return 0");
  cJSON_Delete(array);

  test(log2.numberOfRecords() == log.numberOfRecords(),
    "Log did not survive JSON round trip intact.");
}

void testLoggerSerialization2()
{
  smtk::io::Logger log;
  smtk::io::Logger log2;

  smtkErrorMacro(log, "this is an error message");
  smtkWarningMacro(log, "this is a warning message");
  smtkDebugMacro(log, "this is a debug message");

  std::cout << "Log1 is\n" << log.convertToString() << "\n";
  cJSON* array = cJSON_CreateArray();
  test(
    smtk::io::SaveJSON::forLog(array, log, 0, log.numberOfRecords() + 100) ==
    static_cast<int>(log.numberOfRecords()),
    "Exporting too many records should export those we have.");
  test(
    smtk::io::LoadJSON::ofLog(array, log2) ==
    static_cast<int>(log.numberOfRecords()),
    "Importing what we exported produced the wrong number of records.");
  std::cout << "Log2 is\n" << log2.convertToString() << "\n";
  cJSON_Delete(array);

  test(log2.numberOfRecords() == log.numberOfRecords(),
    "Log did not survive JSON round trip intact.");
}

void testExportEntityRef(
  const EntityRefs& entities, IteratorStyle relations, int correctCount)
{
  cJSON* json = cJSON_CreateObject();
  SaveJSON::forEntities(json, entities, relations, JSON_ENTITIES);
  int numRecords = 0;
  for (cJSON* child = json->child; child; child = child->next)
    ++numRecords;
  if (numRecords != correctCount)
    { // For debugging, print out (names of) what we exported:
    if (!entities.empty())
      entities.begin()->manager()->assignDefaultNames();
    std::cout
      << SaveJSON::forEntities(entities, relations, JSON_PROPERTIES)
      << "\n\n"
      << "Exported " << numRecords << ","
      << " expecting " << correctCount
      << " for related record enum " << relations
      << "\n";
    }
  cJSON_Delete(json);
  test(numRecords == correctCount, "Exported wrong number of records.");
}

void testModelExport()
{
  ManagerPtr sm = Manager::create();
  UUIDArray uids = smtk::model::testing::createTet(sm);
  UUIDArray::size_type modelStart = uids.size();
  uids.push_back(sm->addModel().entity());
  sm->findEntity(uids[21])->relations().push_back(uids[modelStart]);
  sm->findEntity(uids[modelStart])->relations().push_back(uids[21]);
  EntityRefs entities;
  entities.insert(EntityRef(sm, uids[8])); // An edge

  testExportEntityRef(entities, smtk::model::ITERATE_BARE, 1);
  testExportEntityRef(entities, smtk::model::ITERATE_CHILDREN, 9);
  testExportEntityRef(entities, smtk::model::ITERATE_MODELS, 79);

  std::string json = SaveJSON::forEntities(entities, smtk::model::ITERATE_BARE, JSON_DEFAULT);
  std::cout << "json for vertex is \n" << json << "\n";
}

int main(int argc, char* argv[])
{
  testLoggerSerialization1();
  testLoggerSerialization2();
  testModelExport();

  int debug = argc > 2 ? 1 : 0;
  std::ifstream file(argc > 1 ? argv[1] : "testOut");
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));
  cJSON* json = cJSON_CreateObject();

  ManagerPtr sm = Manager::create();

  int status = 0;
  status |= LoadJSON::intoModelManager(data.c_str(), sm);
  status |= SaveJSON::fromModelManager(json, sm,
    // Do not export sessions; they will have different UUIDs
    static_cast<JSONFlags>(JSON_ENTITIES | JSON_TESSELLATIONS | JSON_PROPERTIES));

  char* exported = cJSON_Print(json);
  cJSON_Delete(json);
  json = cJSON_CreateObject();
  ManagerPtr sm2 = Manager::create();

  status |= LoadJSON::intoModelManager(exported, sm2);
  status |= SaveJSON::fromModelManager(json, sm2,
    // Do not export sessions; they will have different UUIDs
    static_cast<JSONFlags>(JSON_ENTITIES | JSON_TESSELLATIONS | JSON_PROPERTIES));
  char* exported2 = cJSON_Print(json);

  if (debug || strcmp(exported, exported2))
    {
    std::cout << "====== snip =======\n";
    std::cout << exported << "\n";
    std::cout << "====== snip =======\n";
    std::cout << exported2 << "\n";
    std::cout << "====== snip =======\n";
    test(strcmp(exported, exported2) == 0, "double import/export pass not exact");
    }
  cJSON_Delete(json);
  free(exported);
  free(exported2);

  return status == 0;
}
