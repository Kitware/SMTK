################################################################################
# Tests
################################################################################
set(unit_tests
  TestArcs.cxx
  TestNodalResource.cxx
  TestNodalResourceFilter.cxx
  TestScalability.cxx
)

smtk_unit_tests(
  LABEL "Graph"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)

# Now add tests which attempt to cause build failures
# to ensure that intentional compile-time failures are
# preserved.
smtk_build_failure_tests(
  LABEL "Graph"
  TESTS TestArcs.cxx 3
  LIBRARIES smtkCore
)
