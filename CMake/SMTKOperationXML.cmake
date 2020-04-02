set(_smtk_operation_xml_script_file "${CMAKE_CURRENT_LIST_FILE}")


include("${CMAKE_CURRENT_LIST_DIR}/EncodeStringFunctions.cmake")

# Given a filename (opSpecs) containing an XML description of an
# operation, configure C++ or Python source that encodes the XML as a string.
# Includes in the xml that resolve to accessible files are replaced by
# the injection of the included file's contents. Query paths for included
# files are passed to this macro with the flag INCLUDE_DIRS.
# The resulting files are placed in the current binary directory.
# smtk_operation_xml(
#   <xml>
#   INCLUDE_DIRS <path_to_include_directories>
#   EXT "py" (optional, defaults to "h" and C++, use "py" for python)
#   TYPE "_json" (optional, defaults to "_xml", appended to file and var name)
#   HEADER_OUTPUT (optional, filled in with full path of generated header)
#   )
# The caller must add the filename in HEADER_OUTPUT to a custom_target, if
# the generated file is in a subdirectory (see https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#how-can-i-add-a-dependency-to-a-source-file-which-is-generated-in-a-subdirectory).
#
# TARGET_OUTPUT can be used to generate a custom_target directly, but
# it is not recommended because it causes long paths and extra targets
# for make and VS projects on windows.
#
# use of add_custom_command() borrowed from vtkEncodeString.cmake
function(smtk_encode_file inOpSpecs)
  set(options "PYTHON")
  set(oneValueArgs "TYPE;HEADER_OUTPUT;TARGET_OUTPUT")
  set(multiValueArgs INCLUDE_DIRS)
  cmake_parse_arguments(_SMTK_op "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if (_SMTK_op_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unrecognized arguments to smtk_encode_file: "
      "${_SMTK_op_UNPARSED_ARGUMENTS}")
  endif ()

  set(inExt "h")
  if (_SMTK_op_PYTHON)
    set(inExt "py")
  endif()
  set(inType "_xml")
  if (DEFINED _SMTK_op_TYPE)
    set(inType ${_SMTK_op_TYPE})
  endif()
  list(APPEND _SMTK_op_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
  if (smtk_PREFIX_PATH)
    list(APPEND _SMTK_op_INCLUDE_DIRS ${smtk_PREFIX_PATH}/${SMTK_OPERATION_INCLUDE_DIR})
  endif ()

  get_filename_component(_genFileBase "${inOpSpecs}" NAME_WE)
  set(_genFile "${CMAKE_CURRENT_BINARY_DIR}/${_genFileBase}${inType}.${inExt}")

  add_custom_command(
    OUTPUT  ${_genFile}
    DEPENDS "${_smtk_operation_xml_script_file}"
            "${inOpSpecs}"
    COMMAND "${CMAKE_COMMAND}"
            "-DgenFileBase=${_genFileBase}"
            "-DgenFile=${_genFile}"
            "-Dinclude_dirs=\"${_SMTK_op_INCLUDE_DIRS}\""
            "-DopSpec=${inOpSpecs}"
            "-Dtype=${inType}"
            "-Dext=${inExt}"
            "-D_smtk_operation_xml_run=ON"
            -P "${_smtk_operation_xml_script_file}")
  set_source_files_properties(${_genFile} PROPERTIES GENERATED ON)

  if (DEFINED _SMTK_op_HEADER_OUTPUT)
    set("${_SMTK_op_HEADER_OUTPUT}"
      "${_genFile}"
      PARENT_SCOPE)
  endif ()
  if (DEFINED _SMTK_op_TARGET_OUTPUT)
    # create a target name like: "generate_smtk_session_vtk_Read_xml.h"
    file(RELATIVE_PATH _targetName ${CMAKE_BINARY_DIR} ${_genFile})
    string(REPLACE "\\" "_" _targetName "generate_${_targetName}")
    string(REPLACE "/" "_" _targetName "${_targetName}")
    string(REPLACE ":" "" _targetName "${_targetName}")

    add_custom_target(${_targetName} DEPENDS ${_genFile})

    set("${_SMTK_op_TARGET_OUTPUT}"
      "${_targetName}"
      PARENT_SCOPE)
  endif ()

endfunction()

# the old implementation, warn users to update to new.
function(smtk_operation_xml opSpecs genFiles)
  message(DEPRECATION "smtk_operation_xml() is deprecated, please update to smtk_encode_file()")
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

# the old implementation, should be replaced when the new one works.
# users always call configure_file on our result - needs to be folded in.
function(smtk_pyoperation_xml opSpecs genFiles)
  set(options "")
  set(oneValueArgs "TYPE")
  set(multiValueArgs INCLUDE_DIRS)
  cmake_parse_arguments(_SMTK_op "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  set(ext "py")
  set(type "_xml")
  if (DEFINED _SMTK_op_TYPE)
    set(type ${_SMTK_op_TYPE})
  endif()
  list(APPEND _SMTK_op_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
  if (smtk_PREFIX_PATH)
    list(APPEND _SMTK_op_INCLUDE_DIRS ${smtk_PREFIX_PATH}/${SMTK_OPERATION_INCLUDE_DIR})
  endif ()
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}${type}.${ext}")
    configureFileAsPyVariable("${opSpec}" "${genFile}" "${genFileBase}${type}" "${_SMTK_op_INCLUDE_DIRS}")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()



if (_smtk_operation_xml_run AND CMAKE_SCRIPT_MODE_FILE)
  if (ext STREQUAL "py")
    configureFileAsPyVariable("${opSpec}" "${genFile}" "${genFileBase}${type}" "${include_dirs}")
  else ()
    configureFileAsCVariable("${opSpec}" "${genFile}" "${genFileBase}${type}" "${include_dirs}")
  endif()
endif()
