set(smtkMarkupPythonNoDataTests
  markupResource
)

# NB: New-style tests should use smtk.testing.TestCase
set(smtkMarkupPythonDataTests)

foreach (test ${smtkMarkupPythonNoDataTests})
  smtk_add_test_python(${test}Py ${test}.py
  --temp-dir ${CMAKE_BINARY_DIR}/Testing/Temporary)
set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Markup")
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkMarkupPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
    --data-dir ${SMTK_DATA_DIR}  --temp-dir ${CMAKE_BINARY_DIR}/Testing/Temporary)
  set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Markup")
  endforeach()
endif()
