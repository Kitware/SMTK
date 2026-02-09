# Tests that don't require any command-line arguments
set(plainTests
  queueJob
)

foreach(test ${plainTests})
  smtk_add_test_python(${test}Py ${test}.py)
  set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Job")
endforeach()
