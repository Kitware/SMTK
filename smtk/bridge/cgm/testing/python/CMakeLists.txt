set(smtkCGMBridgePythonTests
  cgmSolidModeling
  cgmBuildUp
  cgmTransforms
)

# Additional tests that require SMTK_DATA_DIR
set(smtkCGMBridgePythonDataTests
  cgmLoadFile
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
  else() #windows
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR};${SHIBOKEN_LIBRARY_PATH}")
  endif()

  foreach (test ${smtkCGMBridgePythonTests})
    add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()

  if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
    foreach (test ${smtkCGMBridgePythonDataTests})
      add_test(${test}Py
               ${PYTHON_EXECUTABLE}
               ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
               ${SMTK_DATA_DIR})
      set_tests_properties(${test}Py
        PROPERTIES
          ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
      )
    endforeach()
  endif()


else()
  message("could not find python interp")
endif()
