project(ex_implement_an_operator)
cmake_minimum_required(VERSION 2.8.11)

# ++ 1 ++
include(SMTKOperatorXML) # defines smtk_operator_xml()

# The smtk_operator_xml() function writes a file to the current
# binary directory sharing the same name as the input file
# but with "_xml.h" replacing the file extension. For this
# example, that filename is "implement_an_operator_xml.h".
# smtk_operator_xml() appends the exact filename to the
# "operatorXML" variable.
smtk_operator_xml(
  "${CMAKE_CURRENT_SOURCE_DIR}/implement_an_operator.xml"
  operatorXML
)

# Make sure we can include the resulting file:
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# -- 1 --

# Testing must be enabled to build this tutorial
# because it depends on SMTKCoreModelTesting.
if (SMTK_ENABLE_TESTING)
  add_executable(implement_an_operator implement_an_operator.cxx)
  target_link_libraries(implement_an_operator SMTKCore SMTKCoreModelTesting)
  if (SMTK_BUILD_CGM)
    target_link_libraries(implement_an_operator cgmSMTK)
  endif (SMTK_BUILD_CGM)
  add_test(tut-implement_an_operator ${EXECUTABLE_OUTPUT_PATH}/implement_an_operator)
endif()

if (RST2HTML_EXECUTABLE)
  smtk_add_doc(implement_an_operator
    DESTINATION share/doc/SMTK/tutorials/implement_an_operator)
endif()
