project(ex_add_a_session)
cmake_minimum_required(VERSION 3.5)

# ++ 1 ++
include(SMTKSessionJSON) # defines smtk_session_json()

# The smtk_session_json() function writes a file to the current
# binary directory sharing the same name as the input file
# but with "_json.h" replacing the file extension. For this
# example, that filename is "Session_json.h".
# smtk_session_json() appends the exact filename to the
# "sessionJSON" variable.
smtk_session_json(
  "${CMAKE_CURRENT_SOURCE_DIR}/Session.json"
  sessionJSON
)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# -- 1 --

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
