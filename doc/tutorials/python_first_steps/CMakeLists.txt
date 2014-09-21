project(tut_python_first_steps)
cmake_minimum_required(VERSION 2.8.11)

find_package(PythonInterp 2.7 REQUIRED)

if (SMTK_ENABLE_TESTING AND SMTK_BUILD_PYTHON_WRAPPINGS AND SMTK_DATA_DIR)
  add_test(
    NAME tut-python_first_steps
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/first_steps.py ${SMTK_DATA_DIR}/smtk/test2D.json
  )
endif()
