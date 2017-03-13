set(smtkDelaunayExtPythonDataTests)

if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkDelaunayExtPythonDataTests
    ${smtkDelaunayExtPythonDataTests}
    unitTestTriangulateFace
  )
endif()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkDelaunayExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()
