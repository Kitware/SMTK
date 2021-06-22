set(unit_tests
  TestActive.cxx
  TestTask.cxx
)

find_package(Threads REQUIRED)

smtk_unit_tests(
  LABEL "Task"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
