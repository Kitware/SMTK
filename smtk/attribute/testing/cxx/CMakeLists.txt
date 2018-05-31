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
endforeach()

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR)
  set(attributeDataTests
    attributeJsonWriterTest
    attributeReaderTest
    extensibleAttributeTest
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
  foreach(tst ${attributeDataTests})
    add_executable(${tst} ${tst}.cxx)
    target_link_libraries(${tst} smtkCore ${Boost_LIBRARIES})
    add_test(NAME ${tst} COMMAND ${tst} ${${tst}_ARGS})
    set_tests_properties(${tst} PROPERTIES LABELS "Attribute")
  endforeach()
endif()

################################################################################
# New tests using smtk test harness
################################################################################

set(unit_tests
  unitAttributeBasics
  unitAttributeAssociation
  unitComponentItem.cxx
  unitDateTimeItem.cxx
  unitJsonItemDefinitions.cxx
)

set(extra_libs)

if(SMTK_ENABLE_POLYGON_SESSION)
  list(APPEND unit_tests
    unitJsonItems.cxx
    )
  list(APPEND extra_libs
    smtkPolygonSession
    )
endif()


smtk_unit_tests(
  Label "Attribute"
  SOURCES ${unit_tests}
  LIBRARIES
    smtkCore
    ${extra_libs}
)
