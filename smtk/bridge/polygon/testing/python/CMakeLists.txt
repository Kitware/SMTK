set(smtkPolygonSessionPythonTests
)

# Additional tests that require SMTK_DATA_DIR
set(smtkPolygonSessionPythonDataTests
  polygonCreate
  polygonCreateFaces
  polygonForceCreateFace
  polygonTweakEdge
  #polygonImport
  #polygonReadFile
)

foreach (test ${smtkPolygonSessionPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkPolygonSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      -D "${SMTK_DATA_DIR}")
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()
