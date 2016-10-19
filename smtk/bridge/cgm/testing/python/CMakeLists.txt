set(smtkCGMSessionPythonTests
  cgmBooleans
  cgmSolidModeling
  cgmBuildUp
  cgmTransforms
)

# Additional tests that require SMTK_DATA_DIR
set(smtkCGMSessionPythonDataTests
  cgmReadFile
)

foreach (test ${smtkCGMSessionPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkCGMSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      ${SMTK_DATA_DIR})
  endforeach()
endif()
