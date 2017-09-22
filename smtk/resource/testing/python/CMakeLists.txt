set(smtkResourcePythonTests
)

foreach (test ${smtkResourcePythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${SMTK_SOURCE_DIR})
endforeach()
