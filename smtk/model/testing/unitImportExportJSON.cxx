#include "smtk/model/ExportJSON.h"
#include "smtk/model/ImportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/util/Testing/helpers.h"

#include "cJSON.h"

#include <fstream>
#include <iostream>
#include <string>

#include <string.h>

using namespace smtk::model;
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
  status |= ExportJSON::fromModel(json, sm);

  char* exported = cJSON_Print(json);
  cJSON_Delete(json);
  json = cJSON_CreateObject();
  ManagerPtr sm2 = Manager::create();

  status |= ImportJSON::intoModel(exported, sm2);
  status |= ExportJSON::fromModel(json, sm2);
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
