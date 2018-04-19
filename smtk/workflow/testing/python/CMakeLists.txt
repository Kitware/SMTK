set(smtkWorkflowPythonTests
)

# Additional tests that require SMTK_DATA_DIR
if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkWorkflowPythonDataTests
#    unitOperatorFilterSort
  )
endif()

foreach (test ${smtkWorkflowPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkWorkflowPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
  endforeach()
endif()
