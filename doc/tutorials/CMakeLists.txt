# Now add tutorial directories

# To build an smtk-dependent cxx project, the following variables are used to
# find smtk and its dependent packages
set(build_options
  -Dsmtk_DIR:PATH=${PROJECT_BINARY_DIR}
  -Dpybind11_DIR:PATH=${pybind11_DIR}
  -DMOAB_DIR:PATH=${MOAB_DIR}
)

if (SMTK_ENABLE_REMUS_SUPPORT)
  list(APPEND build_options
    -DRemus_DIR:PATH=${Remus_DIR}
    -DZeroMQ_ROOT_DIR:PATH=${ZeroMQ_ROOT_DIR}
  )
endif()

add_test(NAME cxx_first_steps
         COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
         ${CMAKE_CURRENT_SOURCE_DIR}/cxx_first_steps
         ${CMAKE_CURRENT_BINARY_DIR}/cxx_first_steps
         --build-generator ${CMAKE_GENERATOR}
         --build-options ${build_options}
         --test-command print_version)

add_subdirectory(add_a_session)
add_subdirectory(implement_an_operator)
add_subdirectory(python_first_steps)
add_subdirectory(debugging_python_tests)
