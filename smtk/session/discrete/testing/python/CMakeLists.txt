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

if (SMTK_DATA_DIR)
  foreach (test ${smtkDiscreteSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
    set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()
