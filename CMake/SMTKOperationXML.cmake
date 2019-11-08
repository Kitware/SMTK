include("${CMAKE_CURRENT_LIST_DIR}/EncodeStringFunctions.cmake")

# Given a list of filenames (opSpecs) containing XML descriptions of
# operations, configure C++ source that encodes the XML as a string.
# Includes in the xml that resolve to accessible files are replaced by
# the injection of the included file's contents. Query paths for included
# files are passed to this macro with the flag INCLUDE_DIRS.
# The resulting files are placed in the current binary directory and
# appended to genFiles.
# smtk_operation_xml(
#   <xml> <output_variable>
#   INCLUDE_DIRS <path_to_include_directories>
#   )
function(smtk_operation_xml opSpecs genFiles)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs INCLUDE_DIRS)
  cmake_parse_arguments(SMTK_op "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  list(APPEND SMTK_op_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
  if (smtk_PREFIX_PATH)
    list(APPEND SMTK_op_INCLUDE_DIRS ${smtk_PREFIX_PATH}/${SMTK_OPERATION_INCLUDE_DIR})
  endif ()
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}_xml.h")
    configureFileAsCVariable("${opSpec}" "${genFile}" "${genFileBase}_xml" "${SMTK_op_INCLUDE_DIRS}")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()

# Given a list of filenames (opSpecs) containing XML descriptions of
# operations, configure Python source that encodes the XML as a string.
# Includes in the xml that resolve to accessible files are replaced by
# the injection of the included file's contents. Query paths for included
# files are passed to this macro with the flag INCLUDE_DIRS.
# The resulting files are placed in the current binary directory and
# appended to genFiles.
# smtk_pyoperation_xml(
#   <xml> <output_variable>
#   INCLUDE_DIRS <path_to_include_directories>
#   )
function(smtk_pyoperation_xml opSpecs genFiles)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs INCLUDE_DIRS)
  cmake_parse_arguments(SMTK_op "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  list(APPEND SMTK_op_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
  if (smtk_PREFIX_PATH)
    list(APPEND SMTK_op_INCLUDE_DIRS ${smtk_PREFIX_PATH}/${SMTK_OPERATION_INCLUDE_DIR})
  endif ()
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}_xml.py")
    configureFileAsPyVariable("${opSpec}" "${genFile}" "${genFileBase}_xml" "${SMTK_op_INCLUDE_DIRS}")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()
