set(smtkVTKExtPythonTests
)

# Additional tests that require SMTK_DATA_DIR
set(smtkVTKExtPythonDataTests
  displayAnalysisMesh
)

foreach (test ${smtkVTKExtPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkVTKExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
