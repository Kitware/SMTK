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
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "${smtk_python_test_environment}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkPolygonSessionPythonDataTests})
    add_test(${test}Py
      ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
      -D "${SMTK_DATA_DIR}"
    )
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "${smtk_python_test_environment}"
    )
  endforeach()
endif()
