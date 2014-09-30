//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/ExportJSON.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "cJSON.h"

#include <fstream>
#include <iostream>
#include <string>

#include <string.h>

using namespace smtk::model;
using namespace smtk::io;
using smtk::shared_ptr;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? 1 : 0;
  std::ifstream file(argc > 1 ? argv[1] : "testOut");
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));
  cJSON* json = cJSON_CreateObject();

  ManagerPtr sm = Manager::create();

  int status = 0;
  status |= ImportJSON::intoModel(data.c_str(), sm);
  status |= ExportJSON::fromModel(json, sm,
    // Do not export bridge sessions; they will have different UUIDs
    static_cast<JSONFlags>(JSON_ENTITIES | JSON_TESSELLATIONS | JSON_PROPERTIES));

  char* exported = cJSON_Print(json);
  cJSON_Delete(json);
  json = cJSON_CreateObject();
  ManagerPtr sm2 = Manager::create();

  status |= ImportJSON::intoModel(exported, sm2);
  status |= ExportJSON::fromModel(json, sm2,
    // Do not export bridge sessions; they will have different UUIDs
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
