set(smtkRemoteSessionPythonDataTests
  integrationRemoteCGMWorker
)

if (SMTK_DATA_DIR AND EXISTS "${SMTK_DATA_DIR}/ReadMe.mkd")
  foreach(test ${smtkRemoteSessionPythonDataTests})
    add_test(${test}Py
      ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${SMTK_BINARY_DIR}/Testing
      --worker-dir=${SMTK_BINARY_DIR}/Testing
    )
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
endif()
