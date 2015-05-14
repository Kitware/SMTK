set(smtkRemusExtPythonDataTests)

# THe mesher test relies on the discrete session to load a file.
if (SMTK_ENABLE_DISCRETE_SESSION)
  set(smtkRemusExtPythonDataTests
    ${smtkRemusExtPythonDataTests}
    mesher
  )
endif()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  foreach (test ${smtkRemusExtPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
endif()
