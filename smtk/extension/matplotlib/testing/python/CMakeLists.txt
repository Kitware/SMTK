set(smtkMatplotlibExtPythonDataTests
  renderMesh
)

if (SMTK_DATA_DIR)
  foreach (test ${smtkMatplotlibExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES LABELS "ExtMatplotlib")
  endforeach()
endif()
