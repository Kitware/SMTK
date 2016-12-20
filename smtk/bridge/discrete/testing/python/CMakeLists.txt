set(smtkDiscreteSessionPythonTests
#  discreteModeling
)

# Additional tests that require SMTK_DATA_DIR
set(smtkDiscreteSessionPythonDataTests
  discreteCreateEdges
  discreteCreateAndSaveEdges
  discreteReadFile
  discreteSplitEdge
)

foreach (test ${smtkDiscreteSessionPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkDiscreteSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
  endforeach()
endif()
