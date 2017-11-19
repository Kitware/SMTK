set(unit_tests
  unitOperator
)

smtk_unit_tests(
  Label "operation"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
