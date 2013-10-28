#include "smtk/model/ExportJSON.h"
#include "smtk/model/ImportJSON.h"
#include "smtk/model/ModelBody.h"

#include "cJSON.h"

#include <fstream>
#include <string>
#include <iostream>

#include <assert.h>
#include <string.h>

using namespace smtk::model;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? 1 : 0;
  std::ifstream file(argc > 1 ? argv[1] : "testOut");
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));
  cJSON* json = cJSON_CreateObject();

  UUIDsToLinks smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  ModelBody sm(&smTopology, &smArrangements, &smTessellation);

  int status = 0;
  status |= ImportJSON::IntoModel(data.c_str(), &sm);
  status |= ExportJSON::FromModel(json, &sm);

  char* exported = cJSON_Print(json);
  cJSON_Delete(json);
  json = cJSON_CreateObject();
  UUIDsToLinks smTopology2;
  UUIDsToArrangements smArrangements2;
  UUIDsToTessellations smTessellation2;
  ModelBody sm2(&smTopology2, &smArrangements2, &smTessellation2);

  status |= ImportJSON::IntoModel(exported, &sm2);
  status |= ExportJSON::FromModel(json, &sm2);
  char* exported2 = cJSON_Print(json);

  if (debug || strcmp(exported, exported2))
    {
    std::cout << "====== snip =======\n";
    std::cout << exported << "\n";
    std::cout << "====== snip =======\n";
    std::cout << exported2 << "\n";
    std::cout << "====== snip =======\n";
    assert(strcmp(exported, exported2) == 0 && "double import/export pass not exact");
    }
  cJSON_Delete(json);
  free(exported);
  free(exported2);

  return status == 0;
}
