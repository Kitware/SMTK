# installed whether testing is on or not.
# smtk_public_headers(smtkCore helpers.h)

set(commonTests
  unitExtension
  unitPaths
  unitRangeDetector
  unitUnionFind
  unitUUID
)

foreach (test ${commonTests})
  add_executable(${test} MACOSX_BUNDLE  ${test}.cxx)
  target_link_libraries(${test} smtkCore)
  add_test(
    NAME ${test}
    COMMAND $<TARGET_FILE:${test}>)
endforeach()
target_link_libraries(unitPaths ${Boost_LIBRARIES})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_executable(QuerySMTKPythonForModule QuerySMTKPythonForModule.cxx)
  target_link_libraries(QuerySMTKPythonForModule smtkCore)
  install(TARGETS QuerySMTKPythonForModule
          DESTINATION bin)
  add_test(
    NAME QuerySMTKPython
    COMMAND $<TARGET_FILE:QuerySMTKPythonForModule>)
endif()

################################################################################
# New tests using smtk test harness
################################################################################

set(unit_tests
  TestArchive.cxx
  UnitTestDerivedThreadPool.cxx
  UnitTestDateTime.cxx
  UnitTestDateTimeZonePair.cxx
  UnitTestFactory.cxx
  UnitTestInfixExpressionGrammar.cxx
  UnitTestInfixExpressionGrammarImpl.cxx
  UnitTestLinks.cxx
  UnitTestObservers.cxx
  UnitTestRuntimeTypeContainer.cxx
  UnitTestThreadPool.cxx
  UnitTestTypeContainer.cxx
  UnitTestTypeHierarchy.cxx
  UnitTestTypeMap.cxx
  UnitTestTypeName.cxx
  UnitTestUpdateFactory.cxx
  UnitTestVersionNumber.cxx
  UnitTestVisit.cxx
)

set(unit_tests_which_require_data
)

set(_extra_libraries
  ${Boost_LIBRARIES}
  Threads::Threads
)

smtk_unit_tests(
  LABEL "Common"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore ${_extra_libraries}
)
