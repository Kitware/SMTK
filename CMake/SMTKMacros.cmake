#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

include(GenerateExportHeader)

# Utility to build a kit name from the current directory.
function(smtk_get_kit_name kitvar)
  string(REPLACE "${${PROJECT_NAME}_SOURCE_DIR}/" "" dir_prefix ${CMAKE_CURRENT_SOURCE_DIR})
  string(REPLACE "/" "_" kit "${dir_prefix}")
  set(${kitvar} "${kit}" PARENT_SCOPE)
  # Optional second argument to get dir_prefix.
  if (${ARGC} GREATER 1)
    set(${ARGV1} "${dir_prefix}" PARENT_SCOPE)
  endif ()
endfunction()

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE. Also makes sure we install the headers
# into the include folder
function(smtk_public_headers lib)
  smtk_get_kit_name(name dir_prefix)
  foreach (header IN LISTS ARGN)
    if (IS_ABSOLUTE "${header}")
      file(RELATIVE_PATH header_sub "${CMAKE_CURRENT_BINARY_DIR}" "${header}")
      if (NOT header STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/${header_sub}")
        message(FATAL_ERROR "Could not determine subdirectory path for '${header}' relative to '${CMAKE_CURRENT_BINARY_DIR}'")
      endif ()
    else ()
      set(header_sub "${header}")
    endif ()
    get_filename_component(subdir "${header_sub}" DIRECTORY)
    if (subdir)
      set(suffix "/${subdir}")
    else ()
      set(suffix "")
    endif ()
    install (FILES ${header} DESTINATION include/${PROJECT_NAME}/${SMTK_VERSION}/${dir_prefix}${suffix})
  endforeach ()
endfunction()

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE.
function(smtk_private_headers)
  smtk_get_kit_name(name dir_prefix)
endfunction()

# Optionally precompile headers to improve compilation speed.
if(NOT SMTK_BUILD_PRECOMPILED_HEADERS OR ${CMAKE_VERSION} VERSION_LESS "3.16.0")
  function(smtk_precompile_headers)
  endfunction()
else()
  function(smtk_precompile_headers)
    target_precompile_headers(${ARGN})
  endfunction()
endif()

# Declare a library as needed to be installed
# supports the signature
#  smtk_install_library(target [DEPENDS <targets>])
# which allows you to export a target that has dependencies
function(smtk_install_library target)
  set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET hidden)
  if (NOT SMTK_EXPORT_SET)
    set(SMTK_EXPORT_SET "${PROJECT_NAME}")
  endif ()
  install(TARGETS ${target}
    EXPORT ${SMTK_EXPORT_SET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endfunction()

# Declare a target (library or executable) as needing installation.
# Usage:
#   smtk_install_target(target)
function(smtk_install_target target)
  smtk_install_library(${target})
endfunction()

#generate an export header and create an install target for it
function(smtk_export_header target file)
  smtk_get_kit_name(name dir_prefix)
  generate_export_header(${target} EXPORT_FILE_NAME ${file})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}  DESTINATION include/${PROJECT_NAME}/${SMTK_VERSION}/${dir_prefix})
endfunction()

# Builds a source file and an executable that does nothing other than
# compile the given header files.
function(smtk_prepend_string prefix result)
  set(names ${ARGN})
  set(newNames "")
  foreach (name ${ARGN})
    if (IS_ABSOLUTE "${name}")
      set(newName "${name}")
    else ()
      set(newName "${prefix}/${name}")
    endif ()
    set(newNames ${newNames} ${newName})
  endforeach ()
  set(${result} ${newNames} PARENT_SCOPE)
endfunction()

include("${CMAKE_CURRENT_LIST_DIR}/SMTKOperationXML.cmake")

# Builds source groups for the smtk files so that they show up nicely in
# Visual Studio.
# this function will set also set two variable in the parent scope.
# they will be ${source_dir}Srcs and ${source_dir}Headers. So for
# example if you call smtk_source_group(model) we will set the vars:
#   modelSrcs and modelHeaders
function(smtk_source_group source_dir)

  set(src_prop_name ${source_dir}Srcs)
  set(header_prop_name ${source_dir}Headers)

  get_directory_property(sources DIRECTORY ${source_dir} DEFINITION ${src_prop_name})
  get_directory_property(headers DIRECTORY ${source_dir} DEFINITION ${header_prop_name})

  smtk_prepend_string("${source_dir}" sources ${sources})
  smtk_prepend_string("${source_dir}" headers ${headers})

  source_group("${source_dir}_Source" FILES ${sources})
  source_group("${source_dir}_Header" FILES ${headers})

  set(${source_dir}Srcs ${sources} PARENT_SCOPE)
  set(${source_dir}Headers ${headers} PARENT_SCOPE)

endfunction()

set(_SMTKMacros_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")
