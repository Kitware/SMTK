set(smtkStringPythonTests
  testStringToken
)

foreach (test ${smtkStringPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${smtk_SOURCE_DIR})
endforeach()
