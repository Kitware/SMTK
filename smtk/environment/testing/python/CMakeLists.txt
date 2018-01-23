set(smtkEnvironmentPythonTests
testEnvironment
)

foreach (test ${smtkEnvironmentPythonTests})
  smtk_add_test_python(${test}Py ${test}.py)
  set_tests_properties( ${test}Py PROPERTIES LABELS "Environment" )
endforeach()
