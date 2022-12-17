#[=======================================================================[.rst:
smtk_encode_file
----------------

This function adds a custom command that generates a C++ header or Python
source-file which encodes the contents of a file (``<xml>``) from your
source directory. This is used frequently for operation sim-builder
template (SBT) files, icon (SVG) files, and other data you wish to
include in packages. By encoding the file, there is no need to rely on
code to search for it on the filesystem; it is either compiled into a
library (C++) or included as part of a python module (which uses Python's
utilities for importing the data rather than requiring you to find it).

Because this function is frequently used to encode SMTK operation SBT
files, it provides a utility to replace XML ``<include href="…"/>``
directives with the contents of the referenced file.
Query paths for included files are passed to this macro with the
flag ``INCLUDE_DIRS``.

By default, the output header or python files are placed in the binary
directory that corresponds to their source path, but with the input
content type (e.g., ``_xml``, ``_json``, ``_svg``) appended and the
appropriate extension (``.h`` or ``.py``).
If you wish to override this, pass an output filename to
the ``DESTINATION`` option.

.. code-block:: cmake

   smtk_encode_file(
     <xml>
     INCLUDE_DIRS <path_to_include_directories>
     EXT "py" (optional: defaults to "h" and C++; use "py" for python)
     NAME "var" (optional: the name of the generated string literal or function)
     TYPE "_json" (optional: defaults to "_xml", appended to file and var name)
     HEADER_OUTPUT var (optional: filled in with full path of generated header)
     DESTINATION var (optional: specify output relative to current build dir)
   )

Note that if you use ``smtk_encode_file()`` in a subdirectory relative to
the target which compiles its output, you must add the ``DESTINATION`` (if
specified) or the value returned in ``HEADER_OUTPUT`` to a custom_target
(see `this FAQ`_ for details on why).

.. _this FAQ: https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#how-can-i-add-a-dependency-to-a-source-file-which-is-generated-in-a-subdirectory

``TARGET_OUTPUT`` can be used to generate a custom_target directly, but
it is not recommended because it causes long paths and extra targets
for make and VS projects on windows.

Note that if you are using ``smtk_encode_file`` external to SMTK, you do
not need to include ``${smtk_PREFIX_PATH}/${SMTK_OPERATION_INCLUDE_DIR}``
in ``INCLUDE_DIRS`` as it is automatically added; this allows your
external projects to reference the operation, result, and hint attribute
definitions that SMTK provides.

Currently, the following ``TYPE`` options are recognized:
``_py``, ``_json``, ``_svg``, ``_xml``, and ``_cpp``.
The ``cpp`` option does not indicate the input-file's content format
but rather that the output file should be an inline, anonymous-namespaced
C++ function. This option exists for large files (more than 16kiB), for which
MSVC compilers cannot produce a string-literal.
#]=======================================================================]
function(smtk_encode_file inFile)
  set(options "PYTHON;DEBUG")
  set(oneValueArgs "TYPE;HEADER_OUTPUT;TARGET_OUTPUT;NAME;DESTINATION")
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
    list(APPEND _SMTK_op_INCLUDE_DIRS "${smtk_PREFIX_PATH}/${SMTK_OPERATION_INCLUDE_DIR}")
  endif ()

  get_filename_component(_genFileBase "${inFile}" NAME_WE)
  cmake_path(RELATIVE_PATH inFile OUTPUT_VARIABLE relInFile) # Relative to current source dir.
  if ("${relInFile}" STREQUAL "")
    set(relInFile "${inFile}")
  endif()
  get_filename_component(_genFileDir "${relInFile}" DIRECTORY)
  if ("${_genFileDir}" STREQUAL "")
    set(_genFile "${CMAKE_CURRENT_BINARY_DIR}/${_genFileBase}${inType}.${inExt}")
  else()
    set(_genFile "${CMAKE_CURRENT_BINARY_DIR}/${_genFileDir}/${_genFileBase}${inType}.${inExt}")
  endif()

  if (DEFINED _SMTK_op_DESTINATION)
    cmake_path(
      ABSOLUTE_PATH _SMTK_op_DESTINATION
      BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    set(_genFile "${_SMTK_op_DESTINATION}")
  endif()

  # Strip any leading underscore from the file "type" as smtk_encode_file
  # will add it back.
  if (${inType} MATCHES "_.*")
    string(LENGTH "${inType}" _inLen)
    string(SUBSTRING "${inType}" 1 ${_inLen} inTypeWithoutUnderscore)
  else()
    set(inTypeWithoutUnderscore "${inType}")
  endif()

  # If asked to debug, pass that to the executable
  if (_SMTK_op_DEBUG)
    set(inDebug "DEBUG")
  endif()

  if (_SMTK_op_PYTHON)
    set(inTypeWithoutUnderscore "py")
  endif()

  # If a literal/function name was provided, pass it along
  if (_SMTK_op_NAME)
    set(_nameArgs "NAME;${_SMTK_op_NAME}")
  endif()

  add_custom_command(
    OUTPUT  "${_genFile}"
    MAIN_DEPENDENCY "${inFile}"
    DEPENDS
      smtk_encode_file
      "${inFile}"
    COMMAND smtk_encode_file
    ARGS
      HEADER_OUTPUT "${_genFile}"
      INCLUDE_DIRS "\"${_SMTK_op_INCLUDE_DIRS}\""
      TYPE "${inTypeWithoutUnderscore}"
      EXT "${inExt}"
      ${_nameArgs}
      ${inDebug}
      "${inFile}"
  )
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
