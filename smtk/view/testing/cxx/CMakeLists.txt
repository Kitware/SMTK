set(unit_tests
  unitDescriptivePhrase.cxx
  unitJSONView.cxx
)

if (SMTK_ENABLE_POLYGON_SESSION)
  smtk_unit_tests(
    Label "View"
    SOURCES ${unit_tests}
    LIBRARIES
      smtkCore
      smtkCoreResourceTesting
      smtkPolygonSession
      ${Boost_LIBRARIES}
  )
endif()
