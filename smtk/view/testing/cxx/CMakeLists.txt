set(unit_tests
)

set(unit_tests_which_require_data
  unitBadge.cxx
  unitDescriptivePhrase.cxx
  unitComponentPhraseModel.cxx
  unitJSONView.cxx
)

set(helper_srcs
  utility.h
  utility.cxx
)

if (SMTK_ENABLE_POLYGON_SESSION)
  smtk_unit_tests(
    Label "View"
    SOURCES ${unit_tests}
    SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
    EXTRA_SOURCES ${helper_srcs}
    LIBRARIES
      smtkCore
      smtkCoreResourceTesting
      smtkPolygonSession
      ${Boost_LIBRARIES}
  )
endif()
