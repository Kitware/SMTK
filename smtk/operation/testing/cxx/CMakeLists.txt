set(unit_tests
  TestAsyncOperation.cxx
  TestAvailableOperations.cxx
  TestHints.cxx
  TestMutexedOperation.cxx
  unitOperation.cxx
  unitNamingGroup.cxx
  TestOperationGroup.cxx
  TestOperationLauncher.cxx
  TestRemoveResource.cxx
  TestSafeBlockingInvocation.cxx
  TestThreadSafeLazyEvaluation.cxx
)
set(unit_tests_which_require_data
  TestRemoveResourceAssociations.cxx
  TestRemoveResourceProject.cxx
)

# TestHints requires include directives to be processed by smtk_encode_file:
smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/TestHints.sbt" HEADER_OUTPUT testHeaders)
add_custom_target(testHeadersTarget DEPENDS ${testHeaders})

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
add_dependencies(UnitTests_smtk_operation_testing_cxx
  testHeadersTarget
)
