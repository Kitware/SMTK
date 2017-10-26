set(unit_tests
  TestResourceManager.cxx
)

smtk_unit_tests(
  Label "Resource"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
