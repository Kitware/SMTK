#
set (sim_tests
)

if (SMTK_PYTHON_VERSION STREQUAL "2")
  # The ACE3P workflow doesn't yet support Python3.
  # https://gitlab.kitware.com/cmb/simulation-workflows/issues/2
  list(APPEND sim_tests
    TestSimExportOmega3P_01
  )
endif ()

# All simulation tests require simulation-workflows in SMTK_DATA_DIR
if (SMTK_DATA_DIR)
  foreach (test ${sim_tests})
    smtk_add_test_python(${test}Py ${test}.py
    --data-dir ${SMTK_DATA_DIR}  --temp-dir ${CMAKE_BINARY_DIR}/Testing/Temporary)
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()
