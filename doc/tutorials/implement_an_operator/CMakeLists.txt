project(ex_implement_an_operator)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

find_package(smtk)

# this is probably not the right way to do this, but we need the path to SMTK's
# include directory so EncodeStringFunctions.cmake finds things.
get_target_property(SMTK_INCLUDE_DIR smtkCore INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(SMTK_LIB_DIR smtkCore LOCATION)
get_filename_component(SMTK_LIB_DIR ${SMTK_LIB_DIR} DIRECTORY)

# The smtk_operation_xml() function writes a file to the current
# binary directory sharing the same name as the input file
# but with "_xml.h" replacing the file extension. For this
# example, that filename is "implement_an_operator_xml.h".
# smtk_operation_xml() appends the exact filename to the
# "operatorXML" variable.
smtk_operation_xml(
  "${CMAKE_CURRENT_SOURCE_DIR}/implement_an_operator.xml"
  operatorXML
)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# -- 1 --

include(CTest)
enable_testing()

add_executable(implement_an_operator implement_an_operator.cxx)
target_link_libraries(implement_an_operator smtkCore smtkCoreModelTesting)
add_test(NAME tut-implement_an_operator COMMAND implement_an_operator)
set_tests_properties(tut-implement_an_operator PROPERTIES SKIP_RETURN_CODE 125)
