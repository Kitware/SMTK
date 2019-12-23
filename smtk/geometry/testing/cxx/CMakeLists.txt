################################################################################
# Tests
################################################################################
set(unit_tests
  TestGeometry.cxx
)

smtk_unit_tests(
  Label "Geoemtry"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
