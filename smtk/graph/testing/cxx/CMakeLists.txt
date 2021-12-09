################################################################################
# Tests
################################################################################
set(unit_tests
  TestPlanarResource.cxx
  TestNodalResource.cxx
  TestNodalResourceFilter.cxx
  TestScalability.cxx
  TestVisitArcs.cxx
)

smtk_unit_tests(
  LABEL "Graph"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
