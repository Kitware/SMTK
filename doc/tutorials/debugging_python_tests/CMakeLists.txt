project(tut_python_debug)
cmake_minimum_required(VERSION 3.5)

if (SMTK_ENABLE_TESTING AND SMTK_ENABLE_PYTHON_WRAPPING AND SMTK_DATA_DIR AND SMTK_ENABLE_DISCRETE_SESSION)

  smtk_add_test_python(tut-debug_python_test debug.py
      --src-dir=${SMTK_SOURCE_DIR}
      --temp-dir=${CMAKE_CURRENT_BINARY_DIR}
      --data-dir=${SMTK_DATA_DIR}
  )
endif()

