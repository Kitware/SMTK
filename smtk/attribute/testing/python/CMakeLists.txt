# Tests that don't use command line args
set(plainTests
  associationTest
  attributeAutoNamingTest
  attributeFindItemTest
  attributeReferencingTest
  basicAttributeDefinitionTest
  basicAttributeDerivationTest
  basicAttributeTest
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
smtk_add_test_python(definitionDefaultValueTestPy definitionDefaultValueTest.py dummy.sbt)


# Tests that require SMTK_DATA_DIR
set(smtkAttributePythonDataTests
  copyDefinitionTest
  copyAttributeTest
)
set(smtkAttributePythonNewDataTests
  attributeItemByPath
)

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
