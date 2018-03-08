set(smtkIOPythonTests
  unitOperationLog
  logMessage
)

# do not add ResourceSetTest if we are running on windows
if (NOT WIN32)
  list(APPEND smtkIOPythonTests ResourceSetTest)
endif()

# Additional tests that require SMTK_DATA_DIR
set(smtkIOPythonDataTests
)

foreach (test ${smtkIOPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
  set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125)
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkIOPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()

  set(reader_test attributeReaderTest)
  smtk_add_test_python(${reader_test}Py ${reader_test}.py
    ${SMTK_DATA_DIR}/attribute/attribute_collection/resourceTest/ShallowWater2D.sbi
    29
    19
    )

  # do not add ResourceSetReaderTest if we are running on windows
  if (NOT WIN32)
    set(reader_test ResourceSetReaderTest)
    smtk_add_test_python(${reader_test}Py ${reader_test}.py
      ${SMTK_DATA_DIR}/attribute/attribute_collection/resourceTest/resources.xml
      2
      )
  endif()

endif()
