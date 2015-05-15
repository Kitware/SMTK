add_test(basicViewWriterTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicViewWriterTest.py)
set_tests_properties(basicViewWriterTestPy
  PROPERTIES
    ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
)
