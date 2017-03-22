set(smtkRemusExtPythonDataTests)

# The mesher test relies on the discrete session to load a file.
# This test continually fails during superbuild testing when built with
# shiboken, so we only test it when pybind11 bindings are enabled
if (SMTK_ENABLE_DISCRETE_SESSION AND SMTK_USE_PYBIND11)
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
