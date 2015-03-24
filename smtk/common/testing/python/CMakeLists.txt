set(smtkCommonPythonTests
  version
  uuidGenerator
)

foreach (test ${smtkCommonPythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py --src-dir=${SMTK_SOURCE_DIR})
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endforeach()
