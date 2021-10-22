set(unit_tests
  TestManager.cxx
  TestToken.cxx
)

set(unit_tests_which_require_data
)

smtk_unit_tests(
  LABEL "String"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore
)
