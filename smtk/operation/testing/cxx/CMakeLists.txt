set(unit_tests
  TestAsyncOperation
  TestAvailableOperations
  TestMutexedOperation
  unitOperation
  TestOperationGroup
  TestRemoveResource
)

find_package(Threads REQUIRED)

smtk_unit_tests(
  Label "operation"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore Threads::Threads
)
