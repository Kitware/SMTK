set(smtk_markup_tests_without_data
  TestDelete.cxx
  TestIds.cxx
  TestResource.cxx
)

smtk_unit_tests(
  LABEL "Markup"
  SOURCES ${smtk_markup_tests_without_data}
  SOURCES_REQUIRE_DATA ${smtk_tests_which_require_data}
  LIBRARIES smtkCore smtkMarkup
)
