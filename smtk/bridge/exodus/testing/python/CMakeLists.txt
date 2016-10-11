set(smtkExodusSessionPythonTests
  # Empty for now.
)

# Additional tests that require SMTK_DATA_DIR
set(smtkExodusSessionPythonDataTests
  exoReadFile
  slacReadFile
)

foreach (test ${smtkExodusSessionPythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "${smtk_python_test_environment}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkExodusSessionPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "${smtk_python_test_environment}"
    )
  endforeach()
endif()
