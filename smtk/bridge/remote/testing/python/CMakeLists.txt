set(smtkRemoteSessionPythonDataTests
  integrationRemoteCGMWorker
)

if (SMTK_DATA_DIR AND EXISTS "${SMTK_DATA_DIR}/cmb-testing-data.marker")
  foreach(test ${smtkRemoteSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${SMTK_BINARY_DIR}/Testing
      --worker-dir=${SMTK_BINARY_DIR}/Testing)
  endforeach()
endif()
