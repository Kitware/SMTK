# Additional tests that require SMTK_DATA_DIR
set(smtkAttributePythonDataTests
  copyDefinitionTest
  copyAttributeTest
)
set(smtkAttributePythonNewDataTests
  attributeItemByPath
)

smtk_add_test_python(associationTestPy associationTest.py)
smtk_add_test_python(basicAttributeTestPy basicAttributeTest.py)
smtk_add_test_python(basicAttributeDefinitionTestPy basicAttributeDefinitionTest.py)
smtk_add_test_python(basicAttributeDerivationTestPy basicAttributeDerivationTest.py)
smtk_add_test_python(basicAttributeXMLWriterTestPy basicAttributeXMLWriterTest.py dummypy.sbi)
smtk_add_test_python(expressintTestPy expressionTest.py)
smtk_add_test_python(expressionTest2Py expressionTest2.py)
smtk_add_test_python(attributeAutoNamingTestPy attributeAutoNamingTest.py)
smtk_add_test_python(attributeReferencingTestPy attributeReferencingTest.py)
smtk_add_test_python(categoryTestPy categoryTest.py)
smtk_add_test_python(attributeFindItemTestPy attributeFindItemTest.py)
smtk_add_test_python(definitionDefaultValueTestPy definitionDefaultValueTest.py dummy.sbt)

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkAttributePythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      ${SMTK_DATA_DIR})
  endforeach()
  # New-style tests that use smtk.testing.TestCase
  foreach (test ${smtkAttributePythonNewDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      -D ${SMTK_DATA_DIR})
  endforeach()
endif()
