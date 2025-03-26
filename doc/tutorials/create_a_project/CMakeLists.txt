project(create_a_project)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

find_package(smtk)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# -- 1 --

include(CTest)
enable_testing()

add_executable(create_a_project create_a_project.cxx)
target_link_libraries(create_a_project smtkCore)
add_test(
  NAME tut-create_a_project
  COMMAND create_a_project)
  # WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Testing/Temporary/tutorial/create_a_project")
