set(unit_tests
  TestTask
)

find_package(Threads REQUIRED)

smtk_unit_tests(
  Label "task"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
