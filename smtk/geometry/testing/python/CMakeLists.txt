set(smtkGeometryPythonTests
)

# Additional tests that require SMTK_DATA_DIR
set(smtkGeometryPythonDataTests
)

foreach (test ${smtkGeometryPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${smtk_SOURCE_DIR})
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkGeometryPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
