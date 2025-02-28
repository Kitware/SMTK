project(ex_implement_an_operator)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

find_package(smtk)

# The smtk_encode_file() function writes a file to the current
# binary directory sharing the same name as the input file
# but with "_xml.h" replacing the file extension. For this
# example, that filename is "implement_an_operator_xml.h".
# smtk_encode_file() provides a target name that we can
# add as a dependency so this file gets generated during the build.
smtk_encode_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/implement_an_operator.xml"
  HEADER_OUTPUT headerName
)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# -- 1 --

include(CTest)
enable_testing()

add_executable(implement_an_operator implement_an_operator.cxx ${headerName})
target_link_libraries(implement_an_operator smtkCore smtkCoreModelTesting)
add_test(NAME tut-implement_an_operator COMMAND implement_an_operator)
set_tests_properties(tut-implement_an_operator PROPERTIES SKIP_RETURN_CODE 125)
