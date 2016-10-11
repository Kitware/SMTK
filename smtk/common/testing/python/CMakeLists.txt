set(smtkCommonPythonTests
  version
  uuidGenerator
)

foreach (test ${smtkCommonPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${SMTK_SOURCE_DIR})
endforeach()
