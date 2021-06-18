################################################################################
# Tests
################################################################################
set(unit_tests
  TestGeometry.cxx
  TestSelectionFootprint.cxx
)

smtk_unit_tests(
  LABEL "Geometry"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
