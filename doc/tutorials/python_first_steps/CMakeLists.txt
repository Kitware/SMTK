project(tut_python_first_steps)
cmake_minimum_required(VERSION 2.8.11)

find_package(PythonInterp 2.7 REQUIRED)


if (SMTK_ENABLE_TESTING AND SMTK_BUILD_PYTHON_WRAPPINGS AND SMTK_DATA_DIR)
  add_test(
    NAME tut-python_first_steps
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/first_steps.py
      ${SMTK_DATA_DIR}/smtk/test2D.json
  )

  # Now figure out the environment the test should run in.
  get_filename_component(SHIBOKEN_LIBRARY_PATH ${SHIBOKEN_LIBRARY} PATH)
  set(LIB_ENV_VAR "${SMTK_BINARY_DIR}/lib:${SHIBOKEN_LIBRARY_PATH}")
  if (APPLE AND UNIX)
    set(LIB_ENV_VAR "DYLD_LIBRARY_PATH=${LIB_ENV_VAR}")
  elseif (UNIX)
    set(LIB_ENV_VAR "LD_LIBRARY_PATH=${LIB_ENV_VAR}")
  endif()
  set(SHIBOKEN_SMTK_PYTHON "")
  if (UNIX)
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR}:${SHIBOKEN_LIBRARY_PATH}")
  else() #windows
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR};${SHIBOKEN_LIBRARY_PATH}")
  endif()

  # Assign the environment to the test(s)
  set_tests_properties(tut-python_first_steps
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endif()
