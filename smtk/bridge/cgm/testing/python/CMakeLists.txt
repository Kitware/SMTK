set(smtkCGMBridgePythonTests
  cgmSolidModeling
  cgmBuildUp
  cgmTransforms
)

# Additional tests that require SMTK_DATA_DIR
set(smtkCGMBridgePythonDataTests
  cgmLoadFile
)

foreach (test ${smtkCGMBridgePythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  foreach (test ${smtkCGMBridgePythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             ${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
endif()
