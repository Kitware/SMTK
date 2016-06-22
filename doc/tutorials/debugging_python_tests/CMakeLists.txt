project(tut_python_debug)
cmake_minimum_required(VERSION 3.5)

if (SMTK_ENABLE_TESTING AND SMTK_ENABLE_PYTHON_WRAPPING AND SMTK_DATA_DIR AND SMTK_ENABLE_DISCRETE_SESSION)

  add_test(
    NAME tut-debug_python_test
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/debug.py
      --src-dir=${SMTK_SOURCE_DIR}
      --temp-dir=${CMAKE_CURRENT_BINARY_DIR}
      --data-dir=${SMTK_DATA_DIR}
  )

  # Assign environment variables to the test(s)
  set_tests_properties(tut-debug_python_test
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endif()

