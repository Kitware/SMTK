add_executable(attributeReaderWriterTest attributeReaderWriterTest.cxx)
target_link_libraries(attributeReaderWriterTest smtkCore)

set(attributeTests
  basicAttributeTest
  basicAttributeDefinitionTest
  basicAttributeDerivationTest
  basicAttributeXMLWriterTest
  childrenItemsTest
  expressionTest
  expressionTest2
  attributeAssociationTest
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
  target_link_libraries(${tst} smtkCore)
  add_test(NAME ${tst} COMMAND ${tst} ${${tst}_ARGS})
endforeach()

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  set(attributeDataTests
    attributeReaderTest
    extensibleAttributeTest
  )
  set(attributeReaderTest_ARGS
    "${SMTK_DATA_DIR}/attribute/attribute_system/HydraTemplateV1.sbt"
  )
  set(extensibleAttributeTest_ARGS
    "${SMTK_DATA_DIR}/attribute/attribute_system/ExtensibleTemplateTest.xml"
    "${CMAKE_BINARY_DIR}/Testing/Temporary/ExtensibleTemplateTestResult.xml"
  )
  foreach(tst ${attributeDataTests})
    add_executable(${tst} ${tst}.cxx)
    target_link_libraries(${tst} smtkCore)
    add_test(NAME ${tst} COMMAND ${tst} ${${tst}_ARGS})
  endforeach()
endif()
