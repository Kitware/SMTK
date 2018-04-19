set(unit_tests
)

set(unit_tests_which_require_data
#  unitOperationFilterSort.cxx
)

if (SMTK_ENABLE_POLYGON_SESSION)
  smtk_unit_tests(
    Label "workflow"
    SOURCES ${unit_tests}
    SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
    LIBRARIES
      smtkCore
      smtkCoreResourceTesting
      smtkPolygonSession
  )
endif()
