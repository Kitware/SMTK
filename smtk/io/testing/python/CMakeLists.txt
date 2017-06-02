set(smtkIOPythonTests
  unitOperatorLog
)

# do not add ResourceSetTest if we are running on windows with shiboken
if (NOT (MSVC AND NOT SMTK_USE_PYBIND11))
  list(APPEND smtkIOPythonTests ResourceSetTest)
endif()

# Additional tests that require SMTK_DATA_DIR
set(smtkIOPythonDataTests
)

# Additional tests that require specific bridges
if (SMTK_ENABLE_EXODUS_SESSION)
  set(smtkIOPythonDataTests
    ${smtkIOPythonDataTests}
  readSMTKFile
  )
endif()

foreach (test ${smtkIOPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach (test ${smtkIOPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
  endforeach()

  set(reader_test attributeReaderTest)
  smtk_add_test_python(${reader_test}Py ${reader_test}.py
    ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/ShallowWater2D.sbi
    29
    19
    )

  # do not add ResourceSetReaderTest if we are running on windows with shiboken
  if (NOT (MSVC AND NOT SMTK_USE_PYBIND11))
    set(reader_test ResourceSetReaderTest)
    smtk_add_test_python(${reader_test}Py ${reader_test}.py
      ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/resources.xml
      2
      )
  endif()

endif()
