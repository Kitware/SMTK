# Using smtk test harness

set(unit_tests
  TestDefineOp.cxx
  TestProject.cxx
  TestProjectAssociation.cxx
  TestProjectLifeCycle.cxx
  TestProjectResources.cxx
)
set(unit_tests_which_require_data
  TestProjectPortability.cxx
  TestProjectReadWrite2.cxx
  TestProjectReadWriteEmpty.cxx
)
if (SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND unit_tests_which_require_data
    TestCreateTwoOperationProject.cxx
  )
endif()

set(extra_libs)
if (SMTK_ENABLE_VTK_SUPPORT)
  add_compile_definitions(VTK_SUPPORT)
  list(APPEND extra_libs smtkVTKSession)
  list(APPEND unit_tests_which_require_data
    TestProjectReadWrite.cxx
  )
endif()

smtk_unit_tests(
  LABEL "Project"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES
    smtkCore
    ${Boost_LIBRARIES}
    ${extra_libs}
)
