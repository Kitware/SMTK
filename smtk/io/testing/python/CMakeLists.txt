set(smtkIOPythonTests
  unitOperatorLog
  ResourceSetTest
)

# Additional tests that require SMTK_DATA_DIR
set(smtkIOPythonDataTests
)

foreach (test ${smtkIOPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkIOPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()

  set(reader_test attributeReaderTest)
  smtk_add_test_python(${reader_test}Py ${reader_test}.py
    ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/ShallowWater2D.sbi
    29
    19
  )

  set(reader_test ResourceSetReaderTest)
  smtk_add_test_python(${reader_test}Py ${reader_test}.py
    ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/resources.xml
    2
  )

endif()
