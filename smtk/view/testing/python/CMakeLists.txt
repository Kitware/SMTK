set(smtkViewPythonTests
)

# Additional tests that require SMTK_DATA_DIR
set(smtkViewPythonDataTests
)

foreach (test ${smtkViewPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkViewPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
  endforeach()
endif()
