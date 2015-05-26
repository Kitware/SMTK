#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# Utility to build a kit name from the current directory.
function(smtk_get_kit_name kitvar)
  string(REPLACE "${SMTK_SOURCE_DIR}/" "" dir_prefix ${CMAKE_CURRENT_SOURCE_DIR})
  string(REPLACE "/" "_" kit "${dir_prefix}")
  set(${kitvar} "${kit}" PARENT_SCOPE)
  # Optional second argument to get dir_prefix.
  if (${ARGC} GREATER 1)
    set(${ARGV1} "${dir_prefix}" PARENT_SCOPE)
  endif (${ARGC} GREATER 1)
endfunction(smtk_get_kit_name)


# Builds a source file and an executable that does nothing other than
# compile the given header files.
function(smtk_add_header_test name dir_prefix)
  set(hfiles ${ARGN})
  set(suffix ".cxx")
  set(cxxfiles)
  foreach (header ${ARGN})
    string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}" "" header "${header}")
    get_filename_component(headername ${header} NAME_WE)
    set(src ${CMAKE_CURRENT_BINARY_DIR}/TestBuild_${name}_${headername}${suffix})
    configure_file(${SMTK_SOURCE_DIR}/CMake/TestBuild.cxx.in ${src} @ONLY)
    set(cxxfiles ${cxxfiles} ${src})
  endforeach (header)

  # message(STATUS "sysTools_BINARY_DIR='${sysTools_BINARY_DIR}'")
  # include_directories(${sysTools_BINARY_DIR})

  #include the build directory for the export header
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_library(TestBuild_${name} ${cxxfiles} ${hfiles})
  # target_link_libraries(TestBuild_${name} sysTools)
  set_source_files_properties(${hfiles}
    PROPERTIES HEADER_FILE_ONLY TRUE
    )
endfunction(smtk_add_header_test)

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE. Also makes sure we install the headers
# into the include folder
function(smtk_public_headers)
  smtk_get_kit_name(name dir_prefix)
  foreach (header IN LISTS ARGN)
    if (IS_ABSOLUTE "${header}")
      file(RELATIVE_PATH header_sub "${CMAKE_CURRENT_BINARY_DIR}" "${header}")
      if (NOT header STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/${header_sub}")
        message(FATAL_ERROR "Could not determine subdirectoy path for '${header}' relative to '${CMAKE_CURRENT_BINARY_DIR}'")
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
    install (FILES ${header} DESTINATION include/smtk/${SMTK_VERSION}/${dir_prefix}${suffix})
  endforeach ()
  if (SMTK_ENABLE_TESTING)
    smtk_add_header_test("${name}" "${dir_prefix}" ${ARGN})
  endif()
endfunction(smtk_public_headers)

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE.
function(smtk_private_headers)
  smtk_get_kit_name(name dir_prefix)
  if (SMTK_ENABLE_TESTING)
    smtk_add_header_test("${name}" "${dir_prefix}" ${ARGN})
  endif()
endfunction(smtk_private_headers)

# Declare a library as needed to be installed
# supports the signature
#  smtk_install_library(target [DEPENDS <targets>])
# which allows you to export a target that has dependencies
function(smtk_install_library target)
  install(TARGETS ${target}
    EXPORT SMTK
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  export(TARGETS ${target} APPEND FILE ${SMTK_BINARY_DIR}/SMTKConfig.cmake)
endfunction(smtk_install_library)

#generate an export header and create an install target for it
function(smtk_export_header target file)
  smtk_get_kit_name(name dir_prefix)
  generate_export_header(${target} EXPORT_FILE_NAME ${file})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}  DESTINATION include/smtk/${SMTK_VERSION}/${dir_prefix})
endfunction(smtk_export_header)

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
  endforeach (name)
  set(${result} ${newNames} PARENT_SCOPE)
endfunction(smtk_prepend_string)

include(SMTKOperatorXML)
include(SMTKSessionJSON)

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

endfunction(smtk_source_group)

