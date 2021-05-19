# Using smtk test harness

set(unit_tests
  TestDefineOp
  TestProject
  TestProjectAssociation
  TestProjectLifeCycle
  TestProjectLifeCycle_Deprecated
  TestProjectResources
)
set(unit_tests_which_require_data
  TestProjectReadWrite
  TestProjectReadWrite_Deprecated
  TestProjectReadWrite2
  TestProjectReadWrite2_Deprecated
  TestProjectReadWriteEmpty
)
set(extra_libs)
if (SMTK_ENABLE_VTK_SUPPORT)
  add_compile_definitions(VTK_SUPPORT)
  list(APPEND extra_libs smtkVTKSession)
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
