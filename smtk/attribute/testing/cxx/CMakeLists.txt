add_executable(attributeReaderWriterTest attributeReaderWriterTest.cxx)
target_link_libraries(attributeReaderWriterTest smtkCore)

set(attributeTests
  basicAttributeDefinitionTest
  basicAttributeDerivationTest
  basicAttributeXMLWriterTest
  childrenItemsTest
  definitionTagsTest
  discreteStringsWithDefaultTest
  expressionTest
  expressionTest2
  extensibleAttributeCopyTest
  fileValidationTest
  attributeAutoNamingTest
  attributeReferencingTest
  categoryTest
)
set(basicAttributeXMLWriterTest_ARGS
  "${CMAKE_BINARY_DIR}/Testing/Temporary/basicAttributeXMLWriterTest.xml"
  "${CMAKE_BINARY_DIR}/Testing/Temporary/basicAttributeXMLWriterTest1.xml")
set(childrenItemsTest_ARGS "dummy.sbi")
foreach(tst ${attributeTests})
  add_executable(${tst} ${tst}.cxx)
  target_link_libraries(${tst}
    smtkCore
    ${Boost_LIBRARIES}
  )
  add_test(NAME ${tst} COMMAND ${tst} ${${tst}_ARGS})
  set_tests_properties(${tst} PROPERTIES SKIP_RETURN_CODE 125)
endforeach()

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR)
  set(attributeDataTests
    attributeJsonWriterTest
    attributeReaderTest
    extensibleAttributeTest
    TestReadWrite
  )
  set(attributeReaderTest_ARGS
    "${SMTK_DATA_DIR}/attribute/attribute_collection/HydraTemplateV1.sbt"
  )
  set(extensibleAttributeTest_ARGS
    "${SMTK_DATA_DIR}/attribute/attribute_collection/ExtensibleTemplateTest.xml"
    "${CMAKE_BINARY_DIR}/Testing/Temporary/ExtensibleTemplateTestResult.xml"
  )
  set(attributeJsonWriterTest_ARGS
    "${SMTK_DATA_DIR}/attribute/attribute_collection/attributeJsonWriterTest.json"
    )
  set(TestReadWrite_ARGS)
  file(GLOB_RECURSE sbi_files ${SMTK_DATA_DIR}/*.sbi)
  foreach(sbi_file ${sbi_files})
    list(APPEND TestReadWrite_ARGS ${sbi_file})
  endforeach()

  foreach(tst ${attributeDataTests})
    add_executable(${tst} ${tst}.cxx)
    target_link_libraries(${tst} smtkCore ${Boost_LIBRARIES})
    target_compile_definitions(${tst} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
    add_test(NAME ${tst} COMMAND ${tst} ${${tst}_ARGS})
    set_tests_properties(${tst} PROPERTIES LABELS "Attribute")
    set_tests_properties(${tst} PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()

################################################################################
# New tests using smtk test harness
################################################################################

set(unit_tests
  unitAttributeBasics
  unitAttributeAssociation
  unitAttributeAssociationConstraints
  unitAttributeAnalysis
  unitComponentItem.cxx
  unitDateTimeItem.cxx
  unitJsonItemDefinitions.cxx
  unitPathGrammar.cxx
)

set(unit_tests_which_require_data)

set(extra_libs)

if(SMTK_ENABLE_POLYGON_SESSION)
  list(APPEND unit_tests_which_require_data
    unitJsonItems.cxx
    )
  list(APPEND extra_libs
    smtkPolygonSession
    )
endif()


smtk_unit_tests(
  Label "Attribute"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES
    smtkCore
    ${extra_libs}
)
