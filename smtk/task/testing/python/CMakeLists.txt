set(smtkTaskPythonTests
  testTaskManager
)

# tests that require SMTK_DATA_DIR
set(smtkTaskPythonDataTests
)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  list(APPEND smtkTaskPythonDataTests testTaskAgents)
endif()

foreach (test ${smtkTaskPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
endforeach()

#only run these tests if we have a valid data directory
#built with hdf5
if (SMTK_DATA_DIR)
  foreach (test ${smtkTaskPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR} )
    set_tests_properties( ${test}Py PROPERTIES LABELS "Task" )
  endforeach()
endif()
