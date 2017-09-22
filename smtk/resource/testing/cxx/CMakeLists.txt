set(unit_tests
)

smtk_unit_tests(
  Label "Resource"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
