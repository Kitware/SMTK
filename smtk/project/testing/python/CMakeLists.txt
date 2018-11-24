# Tests that don't use data
set(smtkProjectDataTests
  projectManagerTest
)

if (SMTK_DATA_DIR)
  foreach (test ${smtkProjectDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
  endforeach()
endif()
