set(smtkDelaunayExtPythonDataTests)

if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkDelaunayExtPythonDataTests
    ${smtkDelaunayExtPythonDataTests}
    unitTestTessellateFaces
    unitTestTriangulateFaces
  )
endif()

if (SMTK_DATA_DIR)
  foreach (test ${smtkDelaunayExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES LABELS "ExtDelaunay")
  endforeach()
endif()
