set(unit_tests
  SOURCES
    unitActiveFragmentTree.cxx
)

set(unit_tests_which_require_data
  SOURCES_REQUIRE_DATA
)

smtk_unit_tests(
  LABEL "Polygon"
  ${unit_tests}
  #${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkCoreModelTesting smtkPolygonSession
)
