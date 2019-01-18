set(smtkModelPythonTests
  modelBodyCreate
  modelBodyEntityRefs
  modelRegressions
  entityRefTutorial
)

# Additional tests that require SMTK_DATA_DIR
set(smtkModelPythonDataTests
  modelAttributes
  modelSetPropertyOp
)

if (SMTK_ENABLE_DISCRETE_SESSION)
  set(smtkModelPythonDataTests
    ${smtkModelPythonDataTests}
    modelCloseModelOp
  )
endif()

if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkModelPythonDataTests
    ${smtkModelPythonDataTests}
    modelInstance
  )
endif()

foreach (test ${smtkModelPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkModelPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()
