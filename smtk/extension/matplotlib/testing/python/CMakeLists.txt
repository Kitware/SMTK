set(smtkMatplotlibExtPythonDataTests
  renderMesh
)

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkMatplotlibExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES LABELS "ExtMatplotlib")
  endforeach()
endif()
