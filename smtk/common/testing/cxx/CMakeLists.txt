smtk_public_headers(helpers.h)

set(commonTests
  unitUUID
  unitPaths
  unitUnionFind
  unitRangeDetector
)

foreach (test ${commonTests})
  add_executable(${test} MACOSX_BUNDLE  ${test}.cxx)
  target_link_libraries(${test} smtkCore)
  add_test(
    NAME ${test}
    COMMAND $<TARGET_FILE:${test}>)
endforeach()


################################################################################
# New tests using smtk test harness
################################################################################

set(unit_tests
  UnitTestDateTime.cxx
  UnitTestDateTimeZonePair.cxx
)

smtk_unit_tests(
  Label "Attribute"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore ${Boost_LIBRARIES}
)
