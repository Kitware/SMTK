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
  importExport
)

# Additional tests that require specific bridges
if (SMTK_ENABLE_EXODUS_SESSION AND SMTK_ENABLE_DISCRETE_SESSION AND SMTK_ENABLE_CGM_SESSION)
  set(smtkModelPythonDataTests
    ${smtkModelPythonDataTests}
    multiSessionDescriptivePhrase
  )
endif()

foreach (test ${smtkModelPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkModelPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
