# tests that require SMTK_DATA_DIR
set(pythonDataTests
)

if (SMTK_DATA_DIR)
  foreach (test ${pythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
    set_tests_properties( ${test}Py PROPERTIES LABELS "Project" )
  endforeach()
endif()
