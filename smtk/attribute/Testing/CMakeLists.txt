add_executable(basicAttributeTest basicAttributeTest.cxx)
add_executable(basicAttributeDefinitionTest basicAttributeDefinitionTest.cxx)
add_executable(basicAttributeDerivationTest basicAttributeDerivationTest.cxx)
add_executable(basicAttributeXMLWriterTest basicAttributeXMLWriterTest.cxx)
add_executable(childrenItemsTest childrenItemsTest.cxx)
add_executable(expressionTest expressionTest.cxx)
add_executable(expressionTest2 expressionTest2.cxx)
add_executable(attributeAssociationTest attributeAssociationTest.cxx)
add_executable(attributeAutoNamingTest attributeAutoNamingTest.cxx)
add_executable(attributeReferencingTest attributeReferencingTest.cxx)
add_executable(categoryTest categoryTest.cxx)
add_executable(extensibleAttributeTest extensibleAttributeTest.cxx)

target_link_libraries(basicAttributeTest SMTKCore)
target_link_libraries(basicAttributeDefinitionTest SMTKCore)
target_link_libraries(basicAttributeDerivationTest SMTKCore)
target_link_libraries(basicAttributeXMLWriterTest SMTKCore)
target_link_libraries(childrenItemsTest SMTKCore)
target_link_libraries(expressionTest SMTKCore)
target_link_libraries(expressionTest2 SMTKCore)
target_link_libraries(attributeAssociationTest SMTKCore)
target_link_libraries(attributeAutoNamingTest SMTKCore)
target_link_libraries(attributeReferencingTest SMTKCore)
target_link_libraries(categoryTest SMTKCore)
target_link_libraries(extensibleAttributeTest SMTKCore)


add_test(basicAttributeTest ${EXECUTABLE_OUTPUT_PATH}/basicAttributeTest)
add_test(basicAttributeDefinitionTest ${EXECUTABLE_OUTPUT_PATH}/basicAttributeDefinitionTest)
add_test(basicAttributeDerivationTest ${EXECUTABLE_OUTPUT_PATH}/basicAttributeDerivationTest)
add_test(basicAttributeXMLWriterTest
${EXECUTABLE_OUTPUT_PATH}/basicAttributeXMLWriterTest
  "${CMAKE_BINARY_DIR}/Testing/Temporary/basicAttributeXMLWriterTest.xml"
  "${CMAKE_BINARY_DIR}/Testing/Temporary/basicAttributeXMLWriterTest1.xml")
add_test(childrenItemsTest ${EXECUTABLE_OUTPUT_PATH}/childrenItemsTest dummy.sbi)
add_test(expressintTest ${EXECUTABLE_OUTPUT_PATH}/expressionTest)
add_test(expressionTest2 ${EXECUTABLE_OUTPUT_PATH}/expressionTest2)
add_test(attributeAssociationTest ${EXECUTABLE_OUTPUT_PATH}/attributeAssociationTest)
add_test(attributeAutoNamingTest ${EXECUTABLE_OUTPUT_PATH}/attributeAutoNamingTest)
add_test(attributeReferencingTest ${EXECUTABLE_OUTPUT_PATH}/attributeReferencingTest)
add_test(categoryTest ${EXECUTABLE_OUTPUT_PATH}/categoryTest)

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
add_test(extensibleAttributeTest
${EXECUTABLE_OUTPUT_PATH}/extensibleAttributeTest
   "${SMTK_DATA_DIR}/smtk/attribute/ExtensibleTemplateTest.xml"
   "${CMAKE_BINARY_DIR}/Testing/Temporary/ExtensibleTemplateTestResult.xml")
endif()
