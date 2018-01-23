set(smtkModelPythonTests
  modelBodyCreate
  modelBodyEntityRefs
  entityRefTutorial
)

# Additional tests that require SMTK_DATA_DIR
set(smtkModelPythonDataTests
  modelAttributes
  modelSetPropertyOp
  modelCloseModelOp
  modelPhrase
)

if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkModelPythonDataTests
    ${smtkModelPythonDataTests}
    modelInstance
  )
endif()

if (SMTK_ENABLE_DISCRETE_SESSION AND SMTK_ENABLE_CGM_SESSION)
  set(smtkModelPythonDataTests
    ${smtkModelPythonDataTests}
    multiSessionDescriptivePhrase
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
  endforeach()
endif()
