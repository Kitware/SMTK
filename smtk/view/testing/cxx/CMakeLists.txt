set(unit_tests
  unitDescriptivePhrase.cxx
)

smtk_unit_tests(
  Label "View"
  SOURCES ${unit_tests}
  LIBRARIES
    smtkCore
    smtkCoreResourceTesting
    smtkPolygonSession
    ${Boost_LIBRARIES}
)
