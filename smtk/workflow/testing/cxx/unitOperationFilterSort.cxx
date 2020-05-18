//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/workflow/OperationFilterSort.h"

#include "smtk/workflow/json/jsonOperationFilterSort.h"

#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/operation/Manager.h"

#include <fstream>

using json = nlohmann::json;

int unitOperationFilterSort(int, char*[])
{
  bool ok = true;
  smtk::io::Logger::instance().setFlushToStdout(true);

  auto manager = smtk::operation::Manager::create();

  std::string readFilePath(SMTK_DATA_DIR);
  std::string jsonFile("/workflow/filterList.json");

  std::ifstream file((readFilePath + jsonFile).c_str());
  std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  json j = json::parse(data);

  smtk::workflow::OperationFilterSort::Ptr filter;
  smtk::workflow::from_json(j, filter, manager);

  auto& filters = filter->filterList();
  smtkTest(filters.size() == 15, "Expected 15 filters, have " << filters.size());

  json jj;
  smtk::workflow::to_json(jj, filter, manager);

  std::cout << jj.dump(2) << "\n";

  return ok ? 0 : 1;
}
