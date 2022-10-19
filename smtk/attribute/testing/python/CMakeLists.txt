# Tests that don't use command line args
set(plainTests
  associationTest
  attributeAutoNamingTest
  attributeFindItemTest
  basicAttributeDefinitionTest
  basicAttributeDerivationTest
  definitionTagsTest
  unitAttributeBasics
  categoryTest
  datetimeitemTest
  expressionTest
  expressionTest2
)

foreach(test ${plainTests})
  smtk_add_test_python(${test}Py ${test}.py)
  set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Attribute")
endforeach()


# Tests that use command line args
smtk_add_test_python(basicAttributeXMLWriterTestPy basicAttributeXMLWriterTest.py dummypy.sbi)
set_tests_properties(basicAttributeXMLWriterTestPy PROPERTIES SKIP_RETURN_CODE 125 LABELS "Attribute")
smtk_add_test_python(definitionDefaultValueTestPy definitionDefaultValueTest.py dummy.sbt)

set(vtk_tests)
if (SMTK_ENABLE_VTK_SESSION)
  list(APPEND vtk_tests buildAttributeTest)
endif()

# Tests that require SMTK_DATA_DIR
set(smtkAttributePythonDataTests
  copyDefinitionTest
  copyAttributeTest
)
set(smtkAttributePythonNewDataTests
  attributeItemByPath
  attributeQuery
  extensibleGroupTest
  groupItemRotateTest
  itemUpdater
  referenceItemChildrenTest
  referenceItemIOTest
  valueItemRotateTest
  vectorExpressionTest
  ${vtk_tests}
)

if (SMTK_DATA_DIR)
  foreach (test ${smtkAttributePythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      ${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Attribute")
  endforeach()
  # New-style tests that use smtk.testing.TestCase
  foreach (test ${smtkAttributePythonNewDataTests})
    smtk_add_test_python(${test}Py ${test}.py
    --data-dir ${SMTK_DATA_DIR}  --temp-dir ${CMAKE_BINARY_DIR}/Testing/Temporary)
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Attribute")
  endforeach()
endif()
