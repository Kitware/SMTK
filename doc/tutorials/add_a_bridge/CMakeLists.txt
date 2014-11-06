project(ex_add_a_bridge)
cmake_minimum_required(VERSION 2.8.11)

# ++ 1 ++
include(SMTKBridgeJSON) # defines smtk_bridge_json()

# The smtk_bridge_json() function writes a file to the current
# binary directory sharing the same name as the input file
# but with "_json.h" replacing the file extension. For this
# example, that filename is "Bridge_json.h".
# smtk_bridge_json() appends the exact filename to the
# "bridgeJSON" variable.
smtk_bridge_json(
  "${CMAKE_CURRENT_SOURCE_DIR}/Bridge.json"
  bridgeJSON
)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# -- 1 --

# Testing must be enabled to build this tutorial
# because it depends on SMTKCoreModelTesting.
#if (SMTK_ENABLE_TESTING)
#  add_executable(add_a_bridge add_a_bridge.cxx)
#  target_link_libraries(add_a_bridge SMTKCore SMTKCoreModelTesting)
#  if (SMTK_BUILD_CGM)
#    target_link_libraries(add_a_bridge cgmSMTK)
#  endif (SMTK_BUILD_CGM)
#  add_test(tut-add_a_bridge ${EXECUTABLE_OUTPUT_PATH}/add_a_bridge)
#endif()
