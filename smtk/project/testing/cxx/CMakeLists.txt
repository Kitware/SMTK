# Using smtk test harness

set(unit_tests)
set(unit_tests_which_require_data)
set(extra_libs)

if(SMTK_ENABLE_VTK_SESSION)
  list(APPEND unit_tests_which_require_data
    TestProjectCopy.cxx
    TestResourceCopy.cxx
    )
  list(APPEND extra_libs
    smtkVTKSession
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
