################################################################################
# Tests
################################################################################
set(unit_tests
  TestPlanarResource.cxx
  TestNodalResource.cxx
  TestNodalResourceFilter.cxx
  TestVisitArcs.cxx
)

smtk_unit_tests(
  Label "GraphResource"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
