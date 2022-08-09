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
set(unit_tests_which_require_data
  TestRemoveResourceAssociations.cxx
  TestRemoveResourceProject.cxx
)

find_package(Threads REQUIRED)
set(extra_libs)
if (SMTK_ENABLE_VTK_SUPPORT)
  add_compile_definitions(VTK_SUPPORT)
  list(APPEND extra_libs smtkVTKSession)
endif()

smtk_unit_tests(
  LABEL "Operation"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore
    ${Boost_LIBRARIES}
    ${extra_libs}
)
