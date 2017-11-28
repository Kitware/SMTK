set(unit_tests
  TestAsyncOperator
  TestAvailableOperators
  TestMutexedOperator
  unitNewOp
)

find_package(Threads REQUIRED)

smtk_unit_tests(
  Label "operation"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore Threads::Threads
)
