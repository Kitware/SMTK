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

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkPolygonSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      -D "${SMTK_DATA_DIR}")
  endforeach()
endif()
