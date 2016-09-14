set(smtkIOPythonTests
  unitOperatorLog
  ResourceSetTest
)

# Additional tests that require SMTK_DATA_DIR
set(smtkIOPythonDataTests
)

foreach (test ${smtkIOPythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkIOPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()

  set(reader_test attributeReaderTest)
  add_test(${reader_test}Py
    ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/${reader_test}.py
    ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/ShallowWater2D.sbi
    29
    19
  )
  set_tests_properties(${reader_test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )

  set(reader_test ResourceSetReaderTest)
  add_test(${reader_test}Py
    ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/${reader_test}.py
    ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/resources.xml
    2
  )
  set_tests_properties(${reader_test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )

endif()
