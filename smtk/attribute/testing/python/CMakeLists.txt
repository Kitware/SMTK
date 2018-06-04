# Tests that don't use command line args
set(plainTests
  associationTest
  attributeAutoNamingTest
  attributeFindItemTest
  attributeReferencingTest
  basicAttributeDefinitionTest
  basicAttributeDerivationTest
  unitAttributeBasics
  categoryTest
  datetimeitemTest
  expressionTest
  expressionTest2
)

foreach(test ${plainTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()


# Tests that use command line args
smtk_add_test_python(basicAttributeXMLWriterTestPy basicAttributeXMLWriterTest.py dummypy.sbi)
set_tests_properties(basicAttributeXMLWriterTestPy PROPERTIES SKIP_RETURN_CODE 125)
smtk_add_test_python(definitionDefaultValueTestPy definitionDefaultValueTest.py dummy.sbt)


# Tests that require SMTK_DATA_DIR
set(smtkAttributePythonDataTests
  copyDefinitionTest
  copyAttributeTest
)
set(smtkAttributePythonNewDataTests
  attributeItemByPath
)

if (SMTK_DATA_DIR)
  foreach (test ${smtkAttributePythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      ${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
  # New-style tests that use smtk.testing.TestCase
  foreach (test ${smtkAttributePythonNewDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      -D ${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()
