set(smtkModelPythonTests
  modelBodyCreate
)

find_package( PythonInterp 2.7 )

if( PYTHONINTERP_FOUND )
  set(LIB_ENV_VAR "")
  if(APPLE AND UNIX)
    set(LIB_ENV_VAR "DYLD_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}")
  elseif(UNIX)
    set(LIB_ENV_VAR "LD_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}")
  endif()
  set(SHIBOKEN_SMTK_PYTHON "")
  if(UNIX)
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR}:${SHIBOKEN_LIBRARY_PATH}")
  endif()

  foreach (test ${smtkModelPythonTests})
    add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
else()
  message("could not find python interp")
endif()
