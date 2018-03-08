set(smtkDelaunayExtPythonDataTests)

if (SMTK_ENABLE_POLYGON_SESSION)
  set(smtkDelaunayExtPythonDataTests
    ${smtkDelaunayExtPythonDataTests}
    testDelaunayMeshWorker
  )

# TODO: figure out why active test only works on OS X
if (APPLE)
  set(smtkDelaunayExtPythonDataTests
    ${smtkDelaunayExtPythonDataTests}
    testActiveDelaunayMeshWorker
  )
endif()

endif()

if (SMTK_DATA_DIR)
  foreach (test ${smtkDelaunayExtPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py PROPERTIES LABELS "ExtDelaunay" SKIP_RETURN_CODE 125)
  endforeach()
endif()
