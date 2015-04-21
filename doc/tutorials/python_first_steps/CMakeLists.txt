project(tut_python_first_steps)
cmake_minimum_required(VERSION 2.8.11)

if (SMTK_ENABLE_TESTING AND SMTK_ENABLE_PYTHON_WRAPPING AND SMTK_DATA_DIR)

  add_test(
    NAME tut-python_first_steps
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/first_steps.py
      ${SMTK_DATA_DIR}/smtk/test2D.json
  )

  # Assign environment variables to the test(s)
  set_tests_properties(tut-python_first_steps
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endif()
