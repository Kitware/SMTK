# Additional tests that require SMTK_DATA_DIR
set(smtkModelPythonDataTests
  copyDefinitionTest
  copyAttributeTest
)

add_test(associationTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/associationTest.py)
add_test(basicAttributeTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeTest.py)
add_test(basicAttributeDefinitionTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeDefinitionTest.py)
add_test(basicAttributeDerivationTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeDerivationTest.py)
add_test(basicAttributeXMLWriterTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeXMLWriterTest.py dummypy.sbi)
add_test(expressintTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/expressionTest.py)
add_test(expressionTest2Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/expressionTest2.py)
add_test(attributeAutoNamingTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/attributeAutoNamingTest.py)
add_test(attributeReferencingTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/attributeReferencingTest.py)
add_test(categoryTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/categoryTest.py)
add_test(attributeFindItemTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/attributeFindItemTest.py)
add_test(definitionDefaultValueTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/definitionDefaultValueTest.py dummy.sbt)
set_tests_properties(
  associationTestPy basicAttributeTestPy basicAttributeDefinitionTestPy
  basicAttributeDerivationTestPy basicAttributeXMLWriterTestPy
  expressintTestPy expressionTest2Py definitionDefaultValueTestPy
  attributeAutoNamingTestPy attributeAutoNamingTestPy
  attributeReferencingTestPy categoryTestPy attributeFindItemTestPy
  PROPERTIES
    ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
)

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  foreach (test ${smtkModelPythonDataTests})
    add_test(
      ${test}Py
      ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
      ${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
endif()
