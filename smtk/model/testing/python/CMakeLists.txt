set(smtkModelPythonTests
  modelBodyCreate
  modelBodyEntityRefs
  entityRefTutorial
)

# Additional tests that require SMTK_DATA_DIR
set(smtkModelPythonDataTests
  modelAttributes
  modelSetPropertyOp
)

# Additional tests that require specific bridges
if (SMTK_ENABLE_EXODUS_SESSION AND SMTK_ENABLE_DISCRETE_SESSION AND SMTK_ENABLE_CGM_SESSION)
  set(smtkModelPythonDataTests
    ${smtkModelPythonDataTests}
    integrationMultiSessionDescriptivePhrase
  )
endif()

foreach (test ${smtkModelPythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  foreach (test ${smtkModelPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
endif()
