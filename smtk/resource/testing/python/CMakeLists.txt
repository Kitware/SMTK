set(smtkResourcePythonTests
)

# Additional tests that require SMTK_DATA_DIR
if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkResourcePythonDataTests
    unitSelectionManager
  )
endif()

foreach (test ${smtkResourcePythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${SMTK_SOURCE_DIR})
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkResourcePythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
