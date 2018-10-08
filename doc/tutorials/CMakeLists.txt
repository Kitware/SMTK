# Now add tutorial directories

add_test(NAME cxx_first_steps
         COMMAND ctest --build-and-test
         ${CMAKE_CURRENT_SOURCE_DIR}/cxx_first_steps
         ${CMAKE_CURRENT_BINARY_DIR}/cxx_first_steps
         --build-generator ${CMAKE_GENERATOR}
         --build-options
           -Dsmtk_DIR:PATH=${PROJECT_BINARY_DIR}
           -Dpybind11_DIR:PATH=${pybind11_DIR}
           -DMOAB_DIR:PATH=${MOAB_DIR}
           -DRemus_DIR:PATH=${Remus_DIR}
           -DZeroMQ_DIR:PATH=${ZeroMQ_DIR}
         --test-command print_version)

add_subdirectory(add_a_session)
add_subdirectory(implement_an_operator)
add_subdirectory(python_first_steps)
add_subdirectory(debugging_python_tests)
