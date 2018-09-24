project(ex_add_a_session)
cmake_minimum_required(VERSION 3.5)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Testing must be enabled to build this tutorial
# because it depends on smtkCoreModelTesting.
#if (SMTK_ENABLE_TESTING)
#  add_executable(add_a_session add_a_session.cxx)
#  target_link_libraries(add_a_session smtkCore smtkCoreModelTesting)
#  if (SMTK_ENABLE_CGM_SESSION)
#    target_link_libraries(add_a_session smtkCGMSession)
#  endif (SMTK_ENABLE_CGM_SESSION)
#  add_test(NAME tut-add_a_session COMMAND add_a_session)
#endif()
