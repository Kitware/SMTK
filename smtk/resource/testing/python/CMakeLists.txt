set(smtkResourcePythonTests
)

# Additional tests that require SMTK_DATA_DIR
set(smtkResourcePythonDataTests
  unitSelectionManager
)

foreach (test ${smtkResourcePythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${SMTK_SOURCE_DIR})
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkResourcePythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
