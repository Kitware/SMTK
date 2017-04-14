set(smtkDelaunayExtPythonDataTests)

# This testfails during superbuild testing when built with shiboken, so we only
# test it when pybind11 bindings are enabled
if (SMTK_ENABLE_POLYGON_SESSION AND SMTK_USE_PYBIND11)
  set(smtkDelaunayExtPythonDataTests
    ${smtkDelaunayExtPythonDataTests}
    unitTestTessellateFace
    unitTestTriangulateFace
  )
endif()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkDelaunayExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
