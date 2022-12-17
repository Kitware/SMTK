# Now add tutorial directories

# If building with coverage or sanitizers, the tutorials will fail to build
# because they don't know to use the appropriate flags.
if (SMTK_ENABLE_COVERAGE OR SMTK_ENABLE_SANITIZER)
  return ()
endif ()

# To build an smtk-dependent cxx project, the following variables are used to
# find smtk and its dependent packages
set(build_options
  -Dsmtk_DIR:PATH=${CMAKE_BINARY_DIR}/${SMTK_INSTALL_CONFIG_DIR}
  -DEigen3_DIR:PATH=${Eigen3_DIR}
)

if(NOT WIN32)
  add_test(NAME cxx_first_steps
    COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
    ${CMAKE_CURRENT_SOURCE_DIR}/cxx_first_steps
    ${CMAKE_CURRENT_BINARY_DIR}/cxx_first_steps
    --build-generator ${CMAKE_GENERATOR}
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND})
  set_tests_properties(cxx_first_steps
    PROPERTIES
    ENVIRONMENT "${smtk_python_test_environment}")

add_subdirectory(add_a_session)

add_test(NAME implement_an_operator
    COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
    ${CMAKE_CURRENT_SOURCE_DIR}/implement_an_operator
    ${CMAKE_CURRENT_BINARY_DIR}/implement_an_operator
    --build-generator ${CMAKE_GENERATOR}
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND})
  set_tests_properties(implement_an_operator
    PROPERTIES
    ENVIRONMENT "${smtk_python_test_environment}")

add_test(NAME create_a_project
  COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
    ${CMAKE_CURRENT_SOURCE_DIR}/create_a_project
    ${CMAKE_CURRENT_BINARY_DIR}/create_a_project
    --build-generator ${CMAKE_GENERATOR}
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND})

endif()

add_subdirectory(python_first_steps)
