set(smtkRemusExtPythonDataTests)

# THe mesher test relies on the discrete session to load a file.
if (SMTK_ENABLE_DISCRETE_SESSION)
  set(smtkRemusExtPythonDataTests
    ${smtkRemusExtPythonDataTests}
    mesher
  )
endif()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkRemusExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
