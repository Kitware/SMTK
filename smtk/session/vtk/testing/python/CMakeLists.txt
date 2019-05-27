set(smtkVTKSessionPythonTests
  # Empty for now.
)

# Additional tests that require SMTK_DATA_DIR
set(smtkVTKSessionPythonDataTests
  exoReadFile
  slacReadFile
  smtkReadFile
)

foreach (test ${smtkVTKSessionPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
  set_tests_properties( ${test}Py PROPERTIES LABELS "VTKSession" )
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkVTKSessionPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR})
    set_tests_properties( ${test}Py PROPERTIES LABELS "VTKSession" SKIP_RETURN_CODE 125)
  endforeach()
endif()
