//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/job/db/BindingInt.h"
#include "smtk/job/db/BindingText.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <string>
#include <vector>

int jobDatabaseBindings(int, char*[])
{
  using namespace smtk::job::db;

  sqlite3* database = nullptr;
  test(sqlite3_open(":memory:", &database) == SQLITE_OK, "Open an in-memory database.");
  {
    std::string text;
    int number = 0;
    sqlQuery query(database, "SELECT 'first', 42 UNION ALL SELECT 'second', 43;");

    // Each bind call constructs a temporary scalar wrapper. Execute in a later
    // statement so ASAN detects a binding that retains a pointer to the wrapper.
    query.bind<sqlBindingText<std::string>>(0, text);
    query.bind<sqlBindingInt<int>>(1, number);
    test(query.execute(), "Execute scalar bindings after their temporaries expire.");
    test(text == "first" && number == 42, "Scalar bindings retain only the first row.");
  }
  {
    // Owning scalar wrappers must not cause ordinary output containers to be
    // copied: all result rows should still reach the caller's containers.
    std::vector<std::string> text{ "existing" };
    std::vector<int> numbers{ 41 };
    sqlQuery query(database, "SELECT 'first', 42 UNION ALL SELECT 'second', 43;");
    query.bindText(0, text);
    query.bindInt(1, numbers);
    test(query.execute(), "Execute container bindings.");
    test(
      text == std::vector<std::string>({ "existing", "first", "second" }),
      "Text rows are appended to the original container.");
    test(
      numbers == std::vector<int>({ 41, 42, 43 }),
      "Integer rows are appended to the original container.");
  }
  test(sqlite3_close(database) == SQLITE_OK, "Close the database after queries are finalized.");
  return 0;
}
