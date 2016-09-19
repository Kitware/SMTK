set(smtkCGMSessionPythonTests
  cgmBooleans
  cgmSolidModeling
  cgmBuildUp
  cgmTransforms
)

# Additional tests that require SMTK_DATA_DIR
set(smtkCGMSessionPythonDataTests
  cgmReadFile
)

foreach (test ${smtkCGMSessionPythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "${SMTK_TEST_LIB_ENV};${SMTK_PYTHONPATH}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkCGMSessionPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             ${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "${SMTK_TEST_LIB_ENV};${SMTK_PYTHONPATH}"
    )
  endforeach()
endif()
