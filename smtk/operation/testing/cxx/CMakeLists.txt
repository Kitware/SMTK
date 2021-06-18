set(unit_tests
  TestAsyncOperation.cxx
  TestAvailableOperations.cxx
  TestMutexedOperation.cxx
  unitOperation.cxx
  unitNamingGroup.cxx
  TestOperationGroup.cxx
  TestOperationLauncher.cxx
  TestRemoveResource.cxx
  TestThreadSafeLazyEvaluation.cxx
)

find_package(Threads REQUIRED)

smtk_unit_tests(
  LABEL "Operation"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
