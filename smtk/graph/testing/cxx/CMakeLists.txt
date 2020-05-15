################################################################################
# Tests
################################################################################
set(unit_tests
  TestPlanarResource.cxx
  TestNodalResource.cxx
)

smtk_unit_tests(
  Label "GraphResource"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
