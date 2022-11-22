set(smtk_markup_tests_without_data
  TestDelete.cxx
  TestIds.cxx
  TestResource.cxx
)
set(smtk_markup_tests_which_require_data
  TestTag.cxx
)
set(smtk_markup_extra_source
  helpers.h
  helpers.cxx
)

smtk_unit_tests(
  LABEL "Markup"
  SOURCES ${smtk_markup_tests_without_data}
  SOURCES_REQUIRE_DATA ${smtk_markup_tests_which_require_data}
  EXTRA_SOURCES ${smtk_markup_extra_source}
  LIBRARIES smtkCore smtkMarkup
)
